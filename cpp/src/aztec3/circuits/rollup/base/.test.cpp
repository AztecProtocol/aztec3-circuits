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

#include "aztec3/circuits/rollup/base/utils.hpp"
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
#include <cstddef>
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
using aztec3::circuits::rollup::base::utils::dummy_base_rollup_inputs_with_vk_proof;
// using aztec3::circuits::mock::mock_kernel_inputs;

using aztec3::circuits::abis::AppendOnlyTreeSnapshot;

using aztec3::circuits::abis::MembershipWitness;
using aztec3::circuits::abis::NullifierLeafPreimage;
using aztec3::circuits::abis::PreviousRollupData;
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
        size_t public_inputs_size = base_rollup__sim(base_rollup_inputs_vec.data(), &public_inputs_buf);
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
        free((void*)public_inputs_buf);
        info("finished retesting via cbinds...");
    }
};

template <size_t N>
std::array<fr, N> get_sibling_path(stdlib::merkle_tree::MemoryTree tree, size_t leafIndex, size_t subtree_depth_to_skip)
{
    std::array<fr, N> siblingPath;
    auto path = tree.get_hash_path(leafIndex);
    // slice out the skip
    leafIndex = leafIndex >> (subtree_depth_to_skip);

    for (size_t i = 0; i < N; i++) {
        if (leafIndex & (1 << i)) {
            siblingPath[i] = path[subtree_depth_to_skip + i].first;
        } else {
            siblingPath[i] = path[subtree_depth_to_skip + i].second;
        }
    }
    return siblingPath;
}

TEST_F(base_rollup_tests, no_new_contract_leafs)
{
    // When there are no contract deployments. The contract tree should be inserting 0 leafs, (not empty leafs);
    // Initially, the start_contract_tree_snapshot is empty (leaf is 0. hash it up).
    // Get sibling path of index 0 leaf (for circuit to check membership via sibling path)
    // No contract leaves -> will insert empty tree -> i.e. end_contract_tree_root = start_contract_tree_root

    BaseRollupInputs emptyInputs = dummy_base_rollup_inputs_with_vk_proof();
    auto empty_contract_tree = native_base_rollup::MerkleTree(CONTRACT_TREE_HEIGHT);
    auto sibling_path_of_0 =
        get_sibling_path<CONTRACT_SUBTREE_INCLUSION_CHECK_DEPTH>(empty_contract_tree, 0, CONTRACT_SUBTREE_DEPTH);
    // Set the new_contracts_subtree_sibling_path
    emptyInputs.new_contracts_subtree_sibling_path = sibling_path_of_0;

    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(emptyInputs);

    AppendOnlyTreeSnapshot<NT> expectedEndContractTreeSnapshot = {
        .root = empty_contract_tree.root(),
        .next_available_leaf_index = 2,
    };
    ASSERT_EQ(outputs.start_contract_tree_snapshot, emptyInputs.start_contract_tree_snapshot);
    ASSERT_EQ(outputs.end_contract_tree_snapshot, expectedEndContractTreeSnapshot);
    run_cbind(emptyInputs, outputs);
}

TEST_F(base_rollup_tests, contract_leaf_inserted)
{
    // When there is a contract deployment, the contract tree should be inserting 1 leaf.
    // The remaining leafs should be 0 leafs, (not empty leafs);
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();

    // Create a "mock" contract deployment
    NewContractData<NT> new_contract = {
        .contract_address = fr(1),
        .portal_contract_address = fr(3),
        .function_tree_root = fr(2),
    };
    inputs.kernel_data[0].public_inputs.end.new_contracts[0] = new_contract;

    auto empty_contract_tree = native_base_rollup::MerkleTree(CONTRACT_TREE_HEIGHT);
    auto sibling_path_of_0 =
        get_sibling_path<CONTRACT_SUBTREE_INCLUSION_CHECK_DEPTH>(empty_contract_tree, 0, CONTRACT_SUBTREE_DEPTH);
    // Set the new_contracts_subtree_sibling_path
    inputs.new_contracts_subtree_sibling_path = sibling_path_of_0;

    // create expected end contract tree snapshot
    auto expected_contract_leaf = crypto::pedersen_hash::hash_multiple(
        { new_contract.contract_address, new_contract.portal_contract_address, new_contract.function_tree_root });
    auto expeted_end_contracts_snapshot_tree = stdlib::merkle_tree::MemoryTree(CONTRACT_TREE_HEIGHT);
    expeted_end_contracts_snapshot_tree.update_element(0, expected_contract_leaf);

    AppendOnlyTreeSnapshot<NT> expected_end_contracts_snapshot = {
        .root = expeted_end_contracts_snapshot_tree.root(),
        .next_available_leaf_index = 2,
    };
    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);

    ASSERT_EQ(outputs.start_contract_tree_snapshot, inputs.start_contract_tree_snapshot);
    ASSERT_EQ(outputs.end_contract_tree_snapshot, expected_end_contracts_snapshot);
    run_cbind(inputs, outputs);
}

TEST_F(base_rollup_tests, contract_leaf_inserted_in_non_empty_snapshot_tree)
{
    // Same as before except our start_contract_snapshot_tree is not empty
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();

    // Create a "mock" contract deployment
    NewContractData<NT> new_contract = {
        .contract_address = fr(1),
        .portal_contract_address = fr(3),
        .function_tree_root = fr(2),
    };
    inputs.kernel_data[0].public_inputs.end.new_contracts[0] = new_contract;

    auto start_contract_tree_snapshot = native_base_rollup::MerkleTree(CONTRACT_TREE_HEIGHT);
    // insert 12 leaves to the tree (next available leaf index is 12)
    for (size_t i = 0; i < 12; ++i) {
        start_contract_tree_snapshot.update_element(i, fr(i));
    }
    // set the start_contract_tree_snapshot
    inputs.start_contract_tree_snapshot = {
        .root = start_contract_tree_snapshot.root(),
        .next_available_leaf_index = 12,
    };

    // Set the new_contracts_subtree_sibling_path
    auto sibling_path = get_sibling_path<CONTRACT_SUBTREE_INCLUSION_CHECK_DEPTH>(
        start_contract_tree_snapshot, 12, CONTRACT_SUBTREE_DEPTH);
    inputs.new_contracts_subtree_sibling_path = sibling_path;

    // create expected end contract tree snapshot
    auto expected_contract_leaf = crypto::pedersen_hash::hash_multiple(
        { new_contract.contract_address, new_contract.portal_contract_address, new_contract.function_tree_root });
    auto expeted_end_contracts_snapshot_tree = start_contract_tree_snapshot;
    expeted_end_contracts_snapshot_tree.update_element(12, expected_contract_leaf);

    AppendOnlyTreeSnapshot<NT> expected_end_contracts_snapshot = {
        .root = expeted_end_contracts_snapshot_tree.root(),
        .next_available_leaf_index = 14,
    };
    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);

    ASSERT_EQ(outputs.start_contract_tree_snapshot, inputs.start_contract_tree_snapshot);
    ASSERT_EQ(outputs.end_contract_tree_snapshot, expected_end_contracts_snapshot);
    run_cbind(inputs, outputs);
}

TEST_F(base_rollup_tests, new_commitments_tree)
{
    // Create 4 new mock commitments. Add them to kernel data.
    // Then get sibling path so we can verify insert them into the tree.
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();

    std::array<NT::fr, KERNEL_NEW_COMMITMENTS_LENGTH> new_commitments_kernel_0 = { fr(0), fr(1), fr(2), fr(3) };
    std::array<NT::fr, KERNEL_NEW_COMMITMENTS_LENGTH> new_commitments_kernel_1 = { fr(4), fr(5), fr(6), fr(7) };

    inputs.kernel_data[0].public_inputs.end.new_commitments = new_commitments_kernel_0;
    inputs.kernel_data[1].public_inputs.end.new_commitments = new_commitments_kernel_1;

    // get sibling path
    auto start_tree = native_base_rollup::MerkleTree(PRIVATE_DATA_TREE_HEIGHT);
    auto sibling_path =
        get_sibling_path<PRIVATE_DATA_SUBTREE_INCLUSION_CHECK_DEPTH>(start_tree, 0, PRIVATE_DATA_SUBTREE_DEPTH);
    inputs.new_commitments_subtree_sibling_path = sibling_path;

    // create expected commitments snapshot tree
    auto expected_end_commitments_snapshot_tree = start_tree;
    for (size_t i = 0; i < new_commitments_kernel_0.size(); ++i) {
        expected_end_commitments_snapshot_tree.update_element(i, new_commitments_kernel_0[i]);
    }
    for (size_t i = 0; i < new_commitments_kernel_1.size(); ++i) {
        expected_end_commitments_snapshot_tree.update_element(KERNEL_NEW_COMMITMENTS_LENGTH + i,
                                                              new_commitments_kernel_1[i]);
    }
    AppendOnlyTreeSnapshot<NT> expected_end_commitments_snapshot = {
        .root = expected_end_commitments_snapshot_tree.root(),
        .next_available_leaf_index = 8,
    };

    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);
    ASSERT_EQ(outputs.start_private_data_tree_snapshot, inputs.start_private_data_tree_snapshot);
    ASSERT_EQ(outputs.end_private_data_tree_snapshot, expected_end_commitments_snapshot);
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

TEST_F(base_rollup_tests, empty_block_calldata_hash)
{
    // calldata_hash should be computed from leafs of 704 0 bytes. (0x00)
    std::vector<uint8_t> zero_bytes_vec(704, 0);
    auto hash = sha256::sha256(zero_bytes_vec);
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();
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
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();
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

TEST_F(base_rollup_tests, test_compute_membership_historic_private_data)
{
    // Test membership works for empty trees
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();

    auto tree = native_base_rollup::MerkleTree(PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT);
    inputs.constants.start_tree_of_historic_private_data_tree_roots_snapshot = {
        .root = tree.root(),
        .next_available_leaf_index = 0,
    };
    inputs.kernel_data[0].public_inputs.constants.old_tree_roots.private_data_tree_root = fr(0);

    // fetch sibling path from hash path (only get the second half of the hash path)
    auto hash_path = tree.get_hash_path(0);
    std::array<NT::fr, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT> sibling_path;
    for (size_t i = 0; i < PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT; ++i) {
        sibling_path[i] = hash_path[i].second;
    }
    inputs.historic_private_data_tree_root_membership_witnesses[0] = {
        .leaf_index = 0,
        .sibling_path = sibling_path,
    };

    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);
}

TEST_F(base_rollup_tests, test_constants_dont_change)
{
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();
    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);
    ASSERT_EQ(inputs.constants, outputs.constants);
    run_cbind(inputs, outputs);
}

TEST_F(base_rollup_tests, test_aggregate)
{
    // TODO: Fix this when aggregation works
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();
    BaseRollupPublicInputs outputs = aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(inputs);
    ASSERT_EQ(inputs.kernel_data[0].public_inputs.end.aggregation_object.public_inputs,
              outputs.end_aggregation_object.public_inputs);
}

TEST_F(base_rollup_tests, test_proof_verification) {}

TEST_F(base_rollup_tests, test_cbind_0)
{
    BaseRollupInputs inputs = dummy_base_rollup_inputs_with_vk_proof();
    BaseRollupPublicInputs ignored_public_inputs;
    run_cbind(inputs, ignored_public_inputs, false);
}

TEST(private_kernel_tests, test_dummy_previous_rollup_cbind)
{
    uint8_t const* cbind_previous_buf;
    size_t cbind_buf_size = base_rollup__dummy_previous_rollup(&cbind_previous_buf);

    PreviousRollupData<NT> previous = utils::dummy_previous_rollup_with_vk_proof();
    std::vector<uint8_t> expected_vec;
    write(expected_vec, previous);

    // Just compare the first 10 bytes of the serialized public outputs
    // TODO this is not a good test
    if (cbind_buf_size > 10) {
        // for (size_t 0; i < public_inputs_size; i++) {
        for (size_t i = 0; i < 10; i++) {
            ASSERT_EQ(cbind_previous_buf[i], expected_vec[i]);
        }
    }
}

} // namespace aztec3::circuits::rollup::base::native_base_rollup_circuit
