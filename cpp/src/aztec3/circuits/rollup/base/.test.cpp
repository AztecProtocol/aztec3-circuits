// #include <barretenberg/common/serialize.hpp>
// #include <barretenberg/stdlib/types/types.hpp>
// #include <aztec3/oracle/oracle.hpp>
// #include <aztec3/circuits/apps/oracle_wrapper.hpp>
// #include <barretenberg/numeric/random/engine.hpp>
#include "aztec3/circuits/abis/append_only_tree_snapshot.hpp"
#include "aztec3/circuits/abis/rollup/nullifier_leaf_preimage.hpp"
#include "aztec3/constants.hpp"
#include "index.hpp"
#include "init.hpp"

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
using aztec3::circuits::mock::mock_kernel_circuit;
// using aztec3::circuits::mock::mock_kernel_inputs;

using aztec3::circuits::abis::AppendOnlyTreeSnapshot;

using aztec3::circuits::abis::MembershipWitness;
using aztec3::circuits::abis::NullifierLeafPreimage;
using aztec3::circuits::rollup::native_base_rollup::BaseRollupInputs;
using aztec3::circuits::rollup::native_base_rollup::BaseRollupPublicInputs;
using aztec3::circuits::rollup::native_base_rollup::ConstantRollupData;
using aztec3::circuits::rollup::native_base_rollup::NT;

} // namespace

namespace aztec3::circuits::rollup::base::native_base_rollup_circuit {

class base_rollup_tests : public ::testing::Test {};

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
    // @note If using VK when empty, it will fail with segfault.

    BaseRollupInputs baseRollupInputs = {
        .kernel_data = kernel_data,
        .start_nullifier_tree_snapshot = AppendOnlyTreeSnapshot<NT>::empty(),
        .low_nullifier_leaf_preimages = low_nullifier_leaf_preimages,
        .low_nullifier_membership_witness = low_nullifier_membership_witness,
        .historic_private_data_tree_root_membership_witnesses = historic_private_data_tree_root_membership_witnesses,
        .historic_contract_tree_root_membership_witnesses = historic_contract_tree_root_membership_witnesses,
        .constants = constantRollupData,
    };
    return baseRollupInputs;
}

TEST(base_rollup_tests, test_compute_contract_leafs)
{
    BaseRollupInputs emptyInputs = getEmptyBaseRollupInputs();

    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(emptyInputs);

    // Print outputs
    // std::cout << "outputs: " << outputs << std::endl;
}

TEST(base_rollup_tests, test_compute_calldata_hash) {}

TEST(base_rollup_tests, test_compute_membership_historic) {}

TEST(base_rollup_tests, test_compute_and_insert_subtree) {}

} // namespace aztec3::circuits::rollup::base::native_base_rollup_circuit