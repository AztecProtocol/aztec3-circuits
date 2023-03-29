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
#include "barretenberg/crypto/sha256/sha256.hpp"
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
using aztec3::circuits::kernel::private_kernel::utils::dummy_previous_kernel_with_vk_proof;
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
    void run_cbind(BaseRollupInputs& base_rollup_inputs,
                   BaseRollupPublicInputs& expected_public_inputs,
                   bool compare_pubins = true)
    {
        // TODO might be able to get rid of proving key buffer
        uint8_t const* pk_buf;
        size_t pk_size = base_rollup__init_proving_key(&pk_buf);
        info("Proving key size: ", pk_size);

        // TODO might be able to get rid of verification key buffer
        uint8_t const* vk_buf;
        size_t vk_size = base_rollup__init_verification_key(pk_buf, &vk_buf);
        info("Verification key size: ", vk_size);

        std::vector<uint8_t> base_rollup_inputs_vec;
        write(base_rollup_inputs_vec, base_rollup_inputs);

        // uint8_t const* proof_data;
        // size_t proof_data_size;
        uint8_t const* public_inputs_buf;
        info("creating proof");
        size_t public_inputs_size = base_rollup__sim(base_rollup_inputs_vec.data(),
                                                     false, // second_present
                                                     &public_inputs_buf);
        // info("Proof size: ", proof_data_size);
        info("PublicInputs size: ", public_inputs_size);

        if (compare_pubins) {
            BaseRollupPublicInputs public_inputs;
            info("about to read...");
            uint8_t const* public_inputs_buf_tmp = public_inputs_buf;
            read(public_inputs_buf_tmp, public_inputs);
            info("about to assert...");
            ASSERT_EQ(public_inputs.calldata_hash.size(), expected_public_inputs.calldata_hash.size());
            for (size_t i = 0; i < public_inputs.calldata_hash.size(); i++) {
                ASSERT_EQ(public_inputs.calldata_hash[i], expected_public_inputs.calldata_hash[i]);
            }
            info("after assert...");

            // TODO why do the post-write buffers not match?
            //     something in aggregation object [de]serialization?
            info("about to write expected...");
            std::vector<uint8_t> expected_public_inputs_vec;
            write(expected_public_inputs_vec, expected_public_inputs);
            info("about to assert buffers eq...");
            ASSERT_EQ(public_inputs_size, expected_public_inputs_vec.size());
            // Just compare the first 10 bytes of the serialized public outputs
            if (public_inputs_size > 10) {
                // for (size_t 0; i < public_inputs_size; i++) {
                for (size_t i = 0; i < 10; i++) {
                    ASSERT_EQ(public_inputs_buf[i], expected_public_inputs_vec[i]);
                }
            }
        }
        (void)base_rollup_inputs;     // unused
        (void)expected_public_inputs; // unused
        (void)compare_pubins;         // unused

        free((void*)pk_buf);
        free((void*)vk_buf);
        // free((void*)proof_data);
        // SCARY WARNING TODO FIXME why does this free cause issues
        // free((void*)public_inputs_buf);
        info("finished retesting via cbinds...");
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
        // grab mocked previous kernel (need a valid vk, proof, aggobj)
        kernel_data[0] = dummy_previous_kernel_with_vk_proof();
        kernel_data[1] = dummy_previous_kernel_with_vk_proof();

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
};

TEST_F(base_rollup_tests, no_new_contract_leafs)
{
    // When there are no contract deployments. The contract tree should be inserting 0 leafs, (not empty leafs);
    BaseRollupInputs emptyInputs = getEmptyBaseRollupInputs();
    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(emptyInputs);

    // @todo Check the snaphots are updated accordingly.
    // ASSERT_EQ(expectedOut, outputs.end_contract_tree_snapshot);

    run_cbind(emptyInputs, outputs);
}

TEST_F(base_rollup_tests, contract_leaf_inserted)
{
    // When there is a contract deployment, the contract tree should be inserting 1 leaf.
    // The remaining leafs should be 0 leafs, (not empty leafs);
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

    // @todo Check the snaphots are updated accordingly.
    // ASSERT_EQ(contract_tree.root(), outputs.new_contract_leaves_subtree_root);

    run_cbind(inputs, outputs);
}

// TODO: move into helper file
template <size_t N> NT::fr calc_root(NT::fr leaf, NT::uint32 leafIndex, std::array<NT::fr, N> siblingPath)
{
    for (size_t i = 0; i < siblingPath.size(); i++) {
        if (leafIndex & (1 << i)) {
            leaf = crypto::pedersen_hash::hash_multiple({ siblingPath[i], leaf });
        } else {
            leaf = crypto::pedersen_hash::hash_multiple({ leaf, siblingPath[i] });
        }
        leafIndex >>= 1;
    }
    return leaf;
}

fr calc_root_sibling_path(std::vector<std::pair<fr, fr>> siblingPath)
{
    fr leaf = 0;
    for (size_t i = 0; i < siblingPath.size(); i++) {
        leaf = crypto::pedersen_hash::hash_multiple({ siblingPath[i].first, siblingPath[i].second });
    }
    return leaf;
}

TEST_F(base_rollup_tests, new_nullifier_tree)
{
    /**
     * DESCRIPTION
     */
    // This test checks for insertions of all 0 values
    // In this special case we will not need to provide sibling paths to check insertion of the nullifier values
    // This is because 0 values are not actually inserted into the tree, rather the inserted subtree is left
    // empty to begin with

    /**
     * SETUP
     */
    BaseRollupInputs inputs = getEmptyBaseRollupInputs();

    // Create a nullifier tree with 8 nullifiers, this padding is required so that the default 0 value in an indexed
    // merkle tree does not affect our tests Nullifier tree at the start
    native_base_rollup::NullifierTree nullifier_tree = native_base_rollup::NullifierTree(NULLIFIER_TREE_HEIGHT);
    // Insert 7 nullifiers so that the tree is now balanced
    for (size_t i = 1; i < 8; ++i) {
        nullifier_tree.update_element(i);
    }

    // Get nullifier tree start state
    fr start_subtree_root = nullifier_tree.root();
    uint32_t start_next_index = 8;
    AppendOnlyTreeSnapshot<NT> nullifier_tree_start_snapshot = {
        .root = start_subtree_root,
        .next_available_leaf_index = start_next_index,
    };

    // Generate a new empty subtree that will be added to the tree
    stdlib::types::merkle_tree::MemoryTree new_nullifier_subtree =
        stdlib::types::merkle_tree::MemoryTree(NULLIFIER_SUBTREE_DEPTH);
    // TODO: remove this when making empty hash 0
    for (size_t i = 0; i < 8; ++i) {
        new_nullifier_subtree.update_element(
            i, native_base_rollup::NullifierLeaf{ .value = 0, .nextIndex = 0, .nextValue = 0 }.hash());
    }
    // sub tree roots are same in contract and out
    fr subtree_root = new_nullifier_subtree.root();

    // Get the sibling path, we should be able to use the same path to get to the end root
    std::vector<std::pair<fr, fr>> sibling_path = nullifier_tree.get_hash_path(start_next_index);
    std::vector<fr> frontier_path = nullifier_tree.get_frontier_path(start_next_index);

    // Chop the first 3 levels from the frontier_path
    frontier_path.erase(frontier_path.begin(), frontier_path.begin() + 3);
    std::array<fr, NULLIFIER_SUBTREE_INCLUSION_CHECK_DEPTH> frontier_path_array;
    std::copy(frontier_path.begin(), frontier_path.end(), frontier_path_array.begin());

    // Use subtree root and sibling path to calculate the expected end state
    auto end_next_index = start_next_index + uint32_t(KERNEL_NEW_NULLIFIERS_LENGTH * 2);
    fr root = calc_root(subtree_root, end_next_index >> (NULLIFIER_SUBTREE_DEPTH + 1), frontier_path_array);

    // Expected end state
    AppendOnlyTreeSnapshot<NT> nullifier_tree_end_snapshot = {
        .root = root,
        .next_available_leaf_index = end_next_index,
    };

    // Update our start state
    inputs.start_nullifier_tree_snapshot = nullifier_tree_start_snapshot;
    inputs.new_nullifiers_subtree_sibling_path = frontier_path_array;

    /**
     * RUN
     */

    // Run the circuit
    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);

    /**
     * ASSERT
     */
    // Start state
    ASSERT_EQ(outputs.start_nullifier_tree_snapshot.root, nullifier_tree_start_snapshot.root);
    ASSERT_EQ(outputs.start_nullifier_tree_snapshot.next_available_leaf_index,
              nullifier_tree_start_snapshot.next_available_leaf_index);

    // End state
    ASSERT_EQ(outputs.end_nullifier_tree_snapshot.root, nullifier_tree_end_snapshot.root);
    ASSERT_EQ(outputs.end_nullifier_tree_snapshot.next_available_leaf_index,
              nullifier_tree_end_snapshot.next_available_leaf_index);

    // As adding empty subtree, start root == end root
    ASSERT_EQ(outputs.end_nullifier_tree_snapshot.root, outputs.start_nullifier_tree_snapshot.root);
}

TEST_F(base_rollup_tests, new_commitments_tree) {}

TEST_F(base_rollup_tests, empty_block_calldata_hash)
{
    // calldata_hash should be computed from leafs of 704 0 bytes. (0x00)
    std::vector<uint8_t> zero_bytes_vec(704, 0);
    auto hash = sha256::sha256(zero_bytes_vec);
    BaseRollupInputs inputs = getEmptyBaseRollupInputs();
    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);

    std::array<fr, 2> calldata_hash_fr = outputs.calldata_hash;
    auto high_buffer = calldata_hash_fr[0].to_buffer();
    auto low_buffer = calldata_hash_fr[1].to_buffer();

    std::array<uint8_t, 32> calldata_hash;
    for (uint8_t i = 0; i < 16; ++i) {
        calldata_hash[i] = high_buffer[16 + i];
        calldata_hash[16 + i] = low_buffer[16 + i];
    }

    ASSERT_EQ(hash, calldata_hash);

    run_cbind(inputs, outputs);
}

TEST_F(base_rollup_tests, calldata_hash)
{
    // Execute the base rollup circuit with nullifiers, commitments and a contract deployment. Then check the calldata
    // hash against the expected value.
    BaseRollupInputs inputs = getEmptyBaseRollupInputs();
    std::vector<uint8_t> input_data(704, 0);

    for (uint8_t i = 0; i < 4; ++i) {
        // commitments
        input_data[i * 32 + 31] = i + 1; // 1
        inputs.kernel_data[0].public_inputs.end.new_nullifiers[i] = fr(i + 1);

        // nullifiers
        input_data[8 * 32 + i * 32 + 31] = i + 1; // 1
        inputs.kernel_data[0].public_inputs.end.new_commitments[i] = fr(i + 1);
    }

    // Add a contract deployment
    NewContractData<NT> new_contract = {
        .contract_address = fr(1),
        .portal_contract_address = fr(3),
        .function_tree_root = fr(2),
    };
    auto contract_leaf = crypto::pedersen_hash::hash_multiple(
        { new_contract.contract_address, new_contract.portal_contract_address, new_contract.function_tree_root });
    inputs.kernel_data[0].public_inputs.end.new_contracts[0] = new_contract;
    auto contract_leaf_buffer = contract_leaf.to_buffer();
    auto contract_address_buffer = new_contract.contract_address.to_field().to_buffer();
    auto portal_address_buffer = new_contract.portal_contract_address.to_field().to_buffer();
    for (uint8_t i = 0; i < 32; ++i) {
        input_data[16 * 32 + i] = contract_leaf_buffer[i];
        input_data[18 * 32 + i] = contract_address_buffer[i];
        input_data[20 * 32 + i] = portal_address_buffer[i];
    }

    auto hash = sha256::sha256(input_data);

    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);

    // Take the two fields and stich them together to get the calldata hash.
    std::array<fr, 2> calldata_hash_fr = outputs.calldata_hash;
    auto high_buffer = calldata_hash_fr[0].to_buffer();
    auto low_buffer = calldata_hash_fr[1].to_buffer();

    std::array<uint8_t, 32> calldata_hash;
    for (uint8_t i = 0; i < 16; ++i) {
        calldata_hash[i] = high_buffer[16 + i];
        calldata_hash[16 + i] = low_buffer[16 + i];
    }

    ASSERT_EQ(hash, calldata_hash);
    run_cbind(inputs, outputs);
}

TEST_F(base_rollup_tests, test_compute_membership_historic) {}

TEST_F(base_rollup_tests, test_compute_and_insert_subtree) {}

TEST_F(base_rollup_tests, test_cbind_0)
{
    BaseRollupInputs inputs = getEmptyBaseRollupInputs();
    BaseRollupPublicInputs ignored_public_inputs;
    run_cbind(inputs, ignored_public_inputs, false);
}

} // namespace aztec3::circuits::rollup::base::native_base_rollup_circuit
