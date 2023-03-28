// #include <barretenberg/common/serialize.hpp>
// #include <barretenberg/stdlib/types/types.hpp>
// #include <aztec3/oracle/oracle.hpp>
// #include <aztec3/circuits/apps/oracle_wrapper.hpp>
// #include <barretenberg/numeric/random/engine.hpp>
#include "aztec3/circuits/abis/append_only_tree_snapshot.hpp"
#include "aztec3/circuits/abis/private_kernel/new_contract_data.hpp"
#include "aztec3/circuits/abis/private_kernel/previous_kernel_data.hpp"
#include "aztec3/circuits/abis/rollup/nullifier_leaf_preimage.hpp"
#include "aztec3/circuits/rollup/base/init.hpp"
#include "aztec3/constants.hpp"
#include "barretenberg/crypto/sha256/sha256.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/stdlib/merkle_tree/memory_tree.hpp"
#include "index.hpp"
#include "init.hpp"
#include "aztec3/circuits/rollup/base/native_base_rollup_circuit.hpp"
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
#include <cstdint>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

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
using aztec3::circuits::mock::mock_kernel_circuit;
// using aztec3::circuits::mock::mock_kernel_inputs;

using aztec3::circuits::abis::AppendOnlyTreeSnapshot;

using aztec3::circuits::abis::MembershipWitness;
using aztec3::circuits::abis::NullifierLeafPreimage;
using aztec3::circuits::rollup::native_base_rollup::BaseRollupInputs;
using aztec3::circuits::rollup::native_base_rollup::BaseRollupPublicInputs;
using aztec3::circuits::rollup::native_base_rollup::ConstantRollupData;
using aztec3::circuits::rollup::native_base_rollup::NT;

using aztec3::circuits::rollup::native_root_rollup::RootRollupInputs;
using aztec3::circuits::rollup::native_root_rollup::RootRollupPublicInputs;

using aztec3::circuits::abis::FunctionData;
using aztec3::circuits::abis::OptionallyRevealedData;
using aztec3::circuits::abis::private_kernel::NewContractData;

} // namespace

namespace aztec3::circuits::rollup::root::native_root_rollup_circuit {

class root_rollup_tests : public ::testing::Test {
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
        kernel_data[0] = getEmptyPreviousKernelData();
        kernel_data[1] = getEmptyPreviousKernelData();
        // @note If using VK when empty, it will fail with segfault.

        BaseRollupInputs baseRollupInputs = { .kernel_data = kernel_data,
                                              .start_private_data_tree_snapshot = AppendOnlyTreeSnapshot<NT>::empty(),
                                              .start_nullifier_tree_snapshot = AppendOnlyTreeSnapshot<NT>::empty(),
                                              .start_contract_tree_snapshot = AppendOnlyTreeSnapshot<NT>::empty(),
                                              .low_nullifier_leaf_preimages = low_nullifier_leaf_preimages,
                                              .low_nullifier_membership_witness = low_nullifier_membership_witness,
                                              .new_commitments_subtree_sibling_path = { 0 },
                                              .new_nullifiers_subtree_sibling_path = { 0 },
                                              .new_contracts_subtree_sibling_path = { 0 },
                                              .historic_private_data_tree_root_membership_witnesses =
                                                  historic_private_data_tree_root_membership_witnesses,
                                              .historic_contract_tree_root_membership_witnesses =
                                                  historic_contract_tree_root_membership_witnesses,
                                              .constants = constantRollupData };
        return baseRollupInputs;
    }

  protected:
    RootRollupInputs getEmptyRootRollupInputs()
    {
        BaseRollupInputs emptyInputs = getEmptyBaseRollupInputs();
        BaseRollupInputs emptyInputs2 = getEmptyBaseRollupInputs();
        BaseRollupPublicInputs outputs1 =
            aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(emptyInputs);
        BaseRollupPublicInputs outputs2 =
            aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(emptyInputs2);

        RootRollupInputs rootRollupInputs = {};

        return rootRollupInputs;
    }
};

TEST_F(root_rollup_tests, blabber) {}

} // namespace aztec3::circuits::rollup::root::native_root_rollup_circuit