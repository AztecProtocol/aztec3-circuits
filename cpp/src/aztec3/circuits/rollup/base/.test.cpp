// #include <barretenberg/common/serialize.hpp>
// #include <barretenberg/stdlib/types/types.hpp>
// #include <aztec3/oracle/oracle.hpp>
// #include <aztec3/circuits/apps/oracle_wrapper.hpp>
// #include <barretenberg/numeric/random/engine.hpp>
#include "aztec3/circuits/abis/append_only_tree_snapshot.hpp"
#include "aztec3/circuits/abis/private_kernel/new_contract_data.hpp"
#include "aztec3/circuits/abis/private_kernel/previous_kernel_data.hpp"
#include "aztec3/circuits/kernel/private/utils.hpp"
#include "aztec3/circuits/abis/rollup/nullifier_leaf_preimage.hpp"
#include "aztec3/constants.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/stdlib/merkle_tree/memory_tree.hpp"
#include "index.hpp"
#include "init.hpp"
#include "c_bind.h"

#include <aztec3/circuits/apps/test_apps/escrow/deposit.hpp>
#include <aztec3/circuits/apps/test_apps/basic_contract_deployment/basic_contract_deployment.hpp>

#include <aztec3/circuits/abis/call_context.hpp>
#include <aztec3/circuits/abis/call_stack_item.hpp>
#include <aztec3/circuits/abis/contract_deployment_data.hpp>
#include <aztec3/circuits/abis/function_data.hpp>
#include <aztec3/circuits/abis/signed_tx_request.hpp>
#include <aztec3/circuits/abis/tx_context.hpp>
#include <aztec3/circuits/abis/tx_request.hpp>
#include <aztec3/circuits/abis/private_circuit_public_inputs.hpp>
#include <aztec3/circuits/abis/private_kernel/private_inputs.hpp>
#include <aztec3/circuits/abis/private_kernel/public_inputs.hpp>
#include <aztec3/circuits/abis/private_kernel/accumulated_data.hpp>
#include <aztec3/circuits/abis/private_kernel/constant_data.hpp>
#include <aztec3/circuits/abis/private_kernel/old_tree_roots.hpp>
#include <aztec3/circuits/abis/private_kernel/globals.hpp>
// #include <aztec3/circuits/abis/private_kernel/private_inputs.hpp>
// #include <aztec3/circuits/abis/private_kernel/private_inputs.hpp>

#include <aztec3/circuits/apps/function_execution_context.hpp>

// #include <aztec3/circuits/mock/mock_circuit.hpp>
#include <aztec3/circuits/mock/mock_kernel_circuit.hpp>

#include <barretenberg/common/map.hpp>
#include <barretenberg/common/test.hpp>
#include <gtest/gtest.h>
#include <iostream>

// #include <aztec3/constants.hpp>
// #include <barretenberg/crypto/pedersen/pedersen.hpp>
// #include <barretenberg/stdlib/hash/pedersen/pedersen.hpp>

namespace {

using aztec3::circuits::abis::CallContext;
using aztec3::circuits::abis::CallStackItem;
using aztec3::circuits::abis::CallType;
using aztec3::circuits::abis::ContractDeploymentData;
using aztec3::circuits::abis::FunctionData;
using aztec3::circuits::abis::OptionalPrivateCircuitPublicInputs;
using aztec3::circuits::abis::PrivateCircuitPublicInputs;
using aztec3::circuits::abis::SignedTxRequest;
using aztec3::circuits::abis::TxContext;
using aztec3::circuits::abis::TxRequest;

using aztec3::circuits::abis::private_kernel::AccumulatedData;
using aztec3::circuits::abis::private_kernel::ConstantData;
using aztec3::circuits::abis::private_kernel::Globals;
using aztec3::circuits::abis::private_kernel::OldTreeRoots;
using aztec3::circuits::abis::private_kernel::PreviousKernelData;
using aztec3::circuits::abis::private_kernel::PrivateCallData;
using aztec3::circuits::abis::private_kernel::PrivateInputs;
using aztec3::circuits::abis::private_kernel::PublicInputs;

using aztec3::circuits::apps::test_apps::basic_contract_deployment::constructor;
using aztec3::circuits::apps::test_apps::escrow::deposit;

// using aztec3::circuits::mock::mock_circuit;
using aztec3::circuits::kernel::private_kernel::utils::default_previous_kernel;
using aztec3::circuits::mock::mock_kernel_circuit;
// using aztec3::circuits::mock::mock_kernel_inputs;

using aztec3::circuits::abis::AppendOnlyTreeSnapshot;

using aztec3::circuits::abis::MembershipWitness;
using aztec3::circuits::abis::NullifierLeafPreimage;
using aztec3::circuits::rollup::native_base_rollup::BaseRollupInputs;
using aztec3::circuits::rollup::native_base_rollup::BaseRollupPublicInputs;
using aztec3::circuits::rollup::native_base_rollup::ConstantRollupData;
using aztec3::circuits::rollup::native_base_rollup::NT;

using aztec3::circuits::abis::FunctionData;
using aztec3::circuits::abis::OptionallyRevealedData;
using aztec3::circuits::abis::private_kernel::NewContractData;

} // namespace

namespace aztec3::circuits::rollup::base::native_base_rollup_circuit {

class base_rollup_tests : public ::testing::Test {
  protected:
    PreviousKernelData<NT> getEmptyPreviousKernelData()
    {

        std::array<NewContractData<NT>, KERNEL_NEW_CONTRACTS_LENGTH> new_contracts;
        new_contracts.fill(NewContractData<NT>{
            .contract_address = fr::zero(), .portal_contract_address = fr::zero(), .function_tree_root = fr::zero() });

        std::array<OptionallyRevealedData<NT>, KERNEL_OPTIONALLY_REVEALED_DATA_LENGTH> optionally_revealed_data;

        optionally_revealed_data.fill(OptionallyRevealedData<NT>{ .call_stack_item_hash = fr::zero(),
                                                                  .function_data = FunctionData<NT>::empty(),
                                                                  .emitted_events = { 0 },
                                                                  .vk_hash = fr::zero(),
                                                                  .portal_contract_address = { 0 },
                                                                  .pay_fee_from_l1 = false,
                                                                  .pay_fee_from_public_l2 = false,
                                                                  .called_from_l1 = false,
                                                                  .called_from_public_l2 = false });

        native_base_rollup::AggregationObject agg_obj;
        // Aggregation Object
        AccumulatedData<NT> accumulated_data = {
            .aggregation_object = agg_obj,
            .private_call_count = fr::zero(),
            .new_commitments = { 0 },
            .new_nullifiers = { 0 },
            .private_call_stack = { 0 },
            .public_call_stack = { 0 },
            .l1_msg_stack = { 0 },
            .new_contracts = new_contracts,
            .optionally_revealed_data = optionally_revealed_data,
        };

        OldTreeRoots<NT> old_tree_roots = {
            .private_data_tree_root = fr::zero(),
            .nullifier_tree_root = fr::zero(),
            .contract_tree_root = fr::zero(),
            .private_kernel_vk_tree_root = fr::zero(),
        };

        TxContext<NT> tx_context = {
            .is_fee_payment_tx = false,
            .is_rebate_payment_tx = false,
            .is_contract_deployment_tx = false,
            .contract_deployment_data = {
                .constructor_vk_hash = fr::zero(),
                .function_tree_root = fr::zero(),
                .contract_address_salt = fr::zero(),
                .portal_contract_address = fr::zero(),
            },
        };

        PublicInputs<NT> kernel_public_inputs = {
            .end = accumulated_data,
            .constants = { .old_tree_roots = old_tree_roots, .tx_context = tx_context },
            .is_private = true,
        };

        PreviousKernelData<NT> kernel_data = {
            .public_inputs = kernel_public_inputs,
        };

        return kernel_data;
    }

  protected:
    BaseRollupInputs getEmptyBaseRollupInputs()
    {
        ConstantRollupData constantRollupData = ConstantRollupData::empty();

        std::array<NullifierLeafPreimage<NT>, 2 * KERNEL_NEW_NULLIFIERS_LENGTH> low_nullifier_leaf_preimages;
        std::array<MembershipWitness<NT, NULLIFIER_TREE_HEIGHT>, 2 * KERNEL_NEW_NULLIFIERS_LENGTH>
            low_nullifier_membership_witness;

        for (size_t i = 0; i < 2 * KERNEL_NEW_NULLIFIERS_LENGTH; ++i) {
            low_nullifier_leaf_preimages[i] = NullifierLeafPreimage<NT>::empty();
            low_nullifier_membership_witness[i] = MembershipWitness<NT, NULLIFIER_TREE_HEIGHT>::empty();
        }

        std::array<MembershipWitness<NT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT>, 2>
            historic_private_data_tree_root_membership_witnesses = {
                MembershipWitness<NT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT>::empty(),
                MembershipWitness<NT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT>::empty()
            };

        std::array<MembershipWitness<NT, CONTRACT_TREE_ROOTS_TREE_HEIGHT>, 2>
            historic_contract_tree_root_membership_witnesses = {
                MembershipWitness<NT, CONTRACT_TREE_ROOTS_TREE_HEIGHT>::empty(),
                MembershipWitness<NT, CONTRACT_TREE_ROOTS_TREE_HEIGHT>::empty()
            };

        // Kernels
        std::array<abis::private_kernel::PreviousKernelData<NT>, 2> kernel_data;
        // kernel_data[0] = getEmptyPreviousKernelData();
        // kernel_data[1] = getEmptyPreviousKernelData();
        kernel_data[0] = default_previous_kernel();
        kernel_data[1] = default_previous_kernel();
        // @note If using VK when empty, it will fail with segfault.

        BaseRollupInputs baseRollupInputs = { .kernel_data = kernel_data,
                                              .start_nullifier_tree_snapshot = AppendOnlyTreeSnapshot<NT>::empty(),
                                              .low_nullifier_leaf_preimages = low_nullifier_leaf_preimages,
                                              .low_nullifier_membership_witness = low_nullifier_membership_witness,
                                              .historic_private_data_tree_root_membership_witnesses =
                                                  historic_private_data_tree_root_membership_witnesses,
                                              .historic_contract_tree_root_membership_witnesses =
                                                  historic_contract_tree_root_membership_witnesses,
                                              .constants = constantRollupData };
        return baseRollupInputs;
    }
};

TEST_F(base_rollup_tests, no_new_contract_leafs)
{
    // When there are no contract deployments. The contract tree should be inserting 0 leafs, (not empty leafs);
    // @todo This needs to be implemented in the circuit.
    BaseRollupInputs emptyInputs = getEmptyBaseRollupInputs();
    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(emptyInputs);

    stdlib::merkle_tree::MemoryTree contract_tree = stdlib::merkle_tree::MemoryTree(2);
    ASSERT_EQ(contract_tree.root(), outputs.new_contract_leaves_subtree_root);
}

TEST_F(base_rollup_tests, contract_leaf_inserted)
{
    // When there is a contract deployment, the contract tree should be inserting 1 leaf.
    // The remaining leafs should be 0 leafs, (not empty leafs);
    // @todo This needs to be implemented in the circuit. But same as above
    BaseRollupInputs inputs = getEmptyBaseRollupInputs();

    // Create a "mock" contract deployment
    NewContractData<NT> new_contract = {
        .contract_address = fr(1),
        .portal_contract_address = fr(3),
        .function_tree_root = fr(2),
    };
    inputs.kernel_data[0].public_inputs.end.new_contracts[0] = new_contract;

    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);

    stdlib::merkle_tree::MemoryTree contract_tree = stdlib::merkle_tree::MemoryTree(2);
    auto contract_leaf = crypto::pedersen_hash::hash_multiple(
        { new_contract.contract_address, new_contract.portal_contract_address, new_contract.function_tree_root });

    contract_tree.update_element(0, contract_leaf);

    ASSERT_EQ(contract_tree.root(), outputs.new_contract_leaves_subtree_root);
}

TEST_F(base_rollup_tests, new_nullifier_tree)
{
    BaseRollupInputs inputs = getEmptyBaseRollupInputs();

    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);
}

TEST_F(base_rollup_tests, empty_block_calldata_hash) {}

TEST_F(base_rollup_tests, calldata_hash) {}

TEST_F(base_rollup_tests, test_compute_membership_historic) {}

TEST_F(base_rollup_tests, test_compute_and_insert_subtree) {}

TEST_F(base_rollup_tests, test_cbind_0)
{
    uint8_t const* pk_buf;
    size_t pk_size = base_rollup__init_proving_key(&pk_buf);
    info("Proving key size: ", pk_size);

    uint8_t const* vk_buf;
    size_t vk_size = base_rollup__init_verification_key(pk_buf, &vk_buf);
    info("Verification key size: ", vk_size);

    BaseRollupInputs baseRollupInputs = getEmptyBaseRollupInputs();

    std::vector<uint8_t> base_rollup_inputs_vec;
    write(base_rollup_inputs_vec, baseRollupInputs);

    // uint8_t const* proof_data;
    // size_t proof_data_size;
    uint8_t const* public_inputs;
    info("creating proof");
    size_t public_inputs_size = base_rollup__sim(base_rollup_inputs_vec.data(),
                                                 false, // second_present
                                                 &public_inputs);
    // info("Proof size: ", proof_data_size);
    info("PublicInputs size: ", public_inputs_size);

    free((void*)pk_buf);
    free((void*)vk_buf);
    // free((void*)proof_data);
    free((void*)public_inputs);
}

} // namespace aztec3::circuits::rollup::base::native_base_rollup_circuit
