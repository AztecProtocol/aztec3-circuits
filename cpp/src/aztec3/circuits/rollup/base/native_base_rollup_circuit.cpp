#include "aztec3/constants.hpp"
#include "barretenberg/crypto/pedersen_hash/pedersen.hpp"
#include "barretenberg/crypto/sha256/sha256.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/stdlib/hash/pedersen/pedersen.hpp"
#include "barretenberg/stdlib/merkle_tree/membership.hpp"
#include "barretenberg/stdlib/merkle_tree/memory_tree.hpp"
#include "barretenberg/stdlib/merkle_tree/merkle_tree.hpp"
#include "init.hpp"

#include <algorithm>
#include <array>
#include <aztec3/circuits/abis/rollup/base/base_rollup_inputs.hpp>
#include <aztec3/circuits/abis/rollup/base/base_rollup_public_inputs.hpp>
#include <aztec3/circuits/abis/rollup/nullifier_leaf_preimage.hpp>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <vector>

namespace aztec3::circuits::rollup::native_base_rollup {

// TODO: can we aggregate proofs if we do not have a working circuit impl

// TODO: change the public inputs array - we wont be using this?

// Access Native types through NT namespace

bool verify_kernel_proof(NT::Proof kernel_proof)
{
    std::cout << kernel_proof << std::endl; // REMOVE_ME
    return true;
}

/**
 * @brief Create an aggregation object for the proofs that are provided
 *          - We add points P0 for each of our proofs
 *          - We add points P1 for each of our proofs
 *          - We concat our public inputs
 *
 * @param baseRollupInputs
 * @return AggregationObject
 */
AggregationObject aggregate_proofs(BaseRollupInputs baseRollupInputs)
{

    // TODO: NOTE: for now we simply return the aggregation object from the first proof
    return baseRollupInputs.kernel_data[0].public_inputs.end.aggregation_object;
}

/** TODO: implement
 * @brief Get the prover contribution hash object
 *
 * @return NT::fr
 */
NT::fr get_prover_contribution_hash()
{
    return NT::fr(0);
}

std::vector<NT::fr> calculate_contract_leaves(BaseRollupInputs baseRollupInputs)
{

    std::vector<NT::fr> contract_leaves;

    for (size_t i = 0; i < 2; i++) {

        auto new_contacts = baseRollupInputs.kernel_data[i].public_inputs.end.new_contracts;

        // loop over the new contracts
        // TODO: NOTE: we are currently assuming that there is only going to be one
        for (size_t j = 0; j < new_contacts.size(); j++) {

            NT::address contract_address = new_contacts[j].contract_address;
            // TODO: UPDATE protal_contract_address is listed as a 20 byte address in the ABI
            NT::fr portal_contract_address = new_contacts[j].portal_contract_address;
            NT::fr function_tree_root = new_contacts[j].function_tree_root;

            // Pedersen hash of the 3 fields
            auto contract_leaf =
                crypto::pedersen_hash::hash_multiple({ contract_address, portal_contract_address, function_tree_root });

            // @todo What to do about no contract deployments? Insert a zero, we talked with Mike.
            // For the nullifier member-ship, ignore when the leaf is zero.
            contract_leaves.push_back(contract_leaf);
        }
    }

    return contract_leaves;
}

template <size_t N>
NT::fr iterate_through_tree_via_sibling_path(NT::fr leaf, NT::uint32 leafIndex, std::array<NT::fr, N> siblingPath)
{
    for (size_t i = 0; i < siblingPath.size(); i++) {
        if (leafIndex & (1 << i)) {
            leaf = crypto::pedersen_hash::hash_multiple({ leaf, siblingPath[i] });
        } else {
            leaf = crypto::pedersen_hash::hash_multiple({ siblingPath[i], leaf });
        }
    }
    return leaf;
}

template <size_t N>
void check_merkle_membership(NT::fr leaf, NT::uint32 leafIndex, std::array<NT::fr, N> siblingPath, NT::fr root)
{
    auto calculatedRoot = iterate_through_tree_via_sibling_path(leaf, leafIndex, siblingPath);
    if (calculatedRoot != root) {
        // throw std::runtime_error("Merkle membership check failed");
    }
}

template <size_t N>
void check_membership_of_subtree_in_snapshot(NT::uint32 depth,
                                             NT::uint32 nextAvailableLeafIndex,
                                             std::array<NT::fr, N> siblingPath,
                                             NT::fr snapshotRoot)
{
    stdlib::merkle_tree::MemoryTree empty_subtree = stdlib::merkle_tree::MemoryTree(depth);
    auto leafToCheck = empty_subtree.root();
    // next_available_leaf_index is at the leaf level. We need at the subtree level (say height 3). So divide by 8.
    // (if leaf is at index x, its parent is at index floor(x/2))
    auto leafIndex = nextAvailableLeafIndex / (NT::uint32(1) << depth);

    check_merkle_membership(leafToCheck, leafIndex, siblingPath, snapshotRoot);
}

AppendOnlySnapshot insert_subtree_to_private_data_tree(BaseRollupInputs baseRollupInputs,
                                                       NT::fr new_private_data_subtree_root)
{
    // 8 commitments added -> subtree of height 3.
    // So to calculate the new root, we need to start hashing root of subtree with sibling path[2] onwards.
    auto siblingPath = baseRollupInputs.new_commitments_subtree_sibling_path;
    auto leafIndexToInsertAt = baseRollupInputs.start_private_data_tree_snapshot.next_available_leaf_index;
    auto newTreeSnapshot = baseRollupInputs.start_private_data_tree_snapshot;

    // TODO: Sanity check len of siblingPath > height of subtree
    // TODO: Ensure height of subtree is 3 (or 8 commitments)

    // if leaf is at index 8, parent is at index floor(x/2)
    auto leafIndexAtDepth3 = leafIndexToInsertAt / 8;
    // now iterate normally:
    newTreeSnapshot.root =
        iterate_through_tree_via_sibling_path(new_private_data_subtree_root, leafIndexAtDepth3, siblingPath);

    newTreeSnapshot.next_available_leaf_index = leafIndexToInsertAt + 8;

    return newTreeSnapshot;
}

AppendOnlySnapshot insert_subtree_to_contracts_tree(BaseRollupInputs baseRollupInputs,
                                                    NT::fr new_contracts_subtree_root)
{
    // 4 contracts added -> subtree of height 2.
    // So to calculate the new root, we need to start hashing root of subtree with sibling path[1] onwards.
    auto siblingPath = baseRollupInputs.new_contracts_subtree_sibling_path;
    auto leafIndexToInsertAt = baseRollupInputs.start_contract_tree_snapshot.next_available_leaf_index;
    auto newTreeSnapshot = baseRollupInputs.start_contract_tree_snapshot;

    // TODO: Sanity check len of siblingPath > height of subtree
    // TODO: Ensure height of subtree is 2 (or 4 leaves)

    auto leafIndexAtDepth2 = leafIndexToInsertAt / 4;
    // now iterate normally:
    newTreeSnapshot.root =
        iterate_through_tree_via_sibling_path(new_contracts_subtree_root, leafIndexAtDepth2, siblingPath);

    newTreeSnapshot.next_available_leaf_index = leafIndexToInsertAt + 8;

    return newTreeSnapshot;
}

std::array<NT::fr, 3> calculate_new_subtrees(BaseRollupInputs baseRollupInputs, std::vector<NT::fr> contract_leaves)
{
    // Leaves that will be added to the new trees
    std::array<NT::fr, 8> commitment_leaves; // TODO: use constant
    std::array<NT::fr, 8> nullifier_leaves;  // TODO: use constant

    // TODO: we have at size two for now, but we will need

    stdlib::merkle_tree::MemoryTree contracts_tree = stdlib::merkle_tree::MemoryTree(2);
    stdlib::merkle_tree::MemoryTree commitments_tree = stdlib::merkle_tree::MemoryTree(3);
    // TODO: nullifier tree will be a different tree impl - indexed merkle tree
    // stdlib::merkle_tree::MemoryTree nullifier_tree = stdlib::merkle_tree::MemoryTree(2);

    for (size_t i = 0; i < 2; i++) {

        auto new_commitments = baseRollupInputs.kernel_data[i].public_inputs.end.new_commitments;

        // Our commitments size MUST be 4 to calculate our subtrees correctly
        assert(new_commitments.size() == 4);

        for (size_t j = 0; j < new_commitments.size(); j++) {
            // todo: batch insert
            commitments_tree.update_element(i * 4 + j, new_commitments[j]);
        }

        // Nullifiers
        // TODO: not taking care of nullifiers right now
        // auto new_nullifiers = baseRollupInputs.kernel_data[i].public_inputs.end.new_nullifiers;
    }

    // Compute the merkle root of a contract subtree
    // TODO: consolidate what the tree depth should be
    // TODO: cleanup lmao
    // Contracts subtree
    for (size_t i = 0; i < contract_leaves.size(); i++) {
        contracts_tree.update_element(i, contract_leaves[i]);
    }
    NT::fr contracts_tree_subroot = contracts_tree.root();

    // Commitments subtree
    NT::fr commitments_tree_subroot = commitments_tree.root();

    // Nullifiers tree // TODO: implement
    NT::fr nullifiers_tree_subroot = NT::fr(0);

    return std::array<NT::fr, 3>{ contracts_tree_subroot, commitments_tree_subroot, nullifiers_tree_subroot };
}

NT::fr calculate_calldata_hash(BaseRollupInputs baseRollupInputs, std::vector<NT::fr> contract_leaves)
{
    // Compute calldata hashes
    // 22 = (4 + 4 + 1 + 2) * 2 (2 kernels, 4 nullifiers per kernel, 4 commitments per kernel, 1 contract
    // deployments, 2 contracts data fields (size 2 for each) )
    std::array<NT::fr, 22> calldata_hash_inputs;

    for (size_t i = 0; i < 2; i++) {
        // Nullifiers
        auto new_nullifiers = baseRollupInputs.kernel_data[i].public_inputs.end.new_nullifiers;
        auto new_commitments = baseRollupInputs.kernel_data[i].public_inputs.end.new_commitments;
        for (size_t j = 0; j < 4; j++) { // TODO: const
            calldata_hash_inputs[i * 4 + j] = new_nullifiers[j];
            calldata_hash_inputs[8 + i * 4 + j] = new_commitments[j];
        }

        // yuck - TODO: is contract_leaves fixed size?
        calldata_hash_inputs[16 + i] = contract_leaves[i];

        auto new_contracts = baseRollupInputs.kernel_data[i].public_inputs.end.new_contracts;

        // TODO: this assumes that there is only one contract deployment
        calldata_hash_inputs[18 + i] = new_contracts[0].portal_contract_address;
        calldata_hash_inputs[20 + i] = new_contracts[0].function_tree_root;
    }

    // FIXME
    // Calculate sha256 hash of calldata; TODO: work out typing here
    // 22 * 32 = 22 fields, each 32 bytes
    std::array<uint8_t, 22 * 32> calldata_hash_inputs_bytes;
    // Convert all into a buffer, then copy into the array, then hash
    for (size_t i = 0; i < calldata_hash_inputs.size(); i++) {
        auto as_bytes = calldata_hash_inputs[i].to_buffer();

        auto offset = i * 32;
        std::copy(as_bytes.begin(), as_bytes.end(), calldata_hash_inputs_bytes.begin() + offset);
    }
    // TODO: double check this gpt code
    std::vector<uint8_t> calldata_hash_inputs_bytes_vec(calldata_hash_inputs_bytes.begin(),
                                                        calldata_hash_inputs_bytes.end());

    // @todo Make this thing split into two field elements. Hi and low.
    /*std::cout << "calldata_hash_inputs_bytes_vec: " << calldata_hash_inputs_bytes_vec << std::endl;
    auto h = sha256::sha256(calldata_hash_inputs_bytes_vec);
    std::cout << "h: " << h << std::endl;*/

    return sha256::sha256_to_field(calldata_hash_inputs_bytes_vec);
}

/**
 * @brief Check all of the provided commitments against the historical tree roots
 *
 * @param constantBaseRollupData
 * @param baseRollupInputs
 */
void perform_historical_private_data_tree_membership_checks(BaseRollupInputs baseRollupInputs)
{
    // For each of the historic_private_data_tree_membership_checks, we need to do an inclusion proof
    // against the historical root provided in the rollup constants
    auto historic_root = baseRollupInputs.constants.start_tree_of_historic_private_data_tree_roots_snapshot.root;

    for (size_t i = 0; i < 2; i++) {
        NT::fr leaf = baseRollupInputs.kernel_data[i].public_inputs.constants.old_tree_roots.private_data_tree_root;
        abis::MembershipWitness<NT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT> historic_root_witness =
            baseRollupInputs.historic_private_data_tree_root_membership_witnesses[i];

        check_merkle_membership(
            leaf, historic_root_witness.leaf_index, historic_root_witness.sibling_path, historic_root);
    }
}

void perform_historical_contract_data_tree_membership_checks(BaseRollupInputs baseRollupInputs)
{
    auto historic_root = baseRollupInputs.constants.start_tree_of_historic_contract_tree_roots_snapshot.root;

    for (size_t i = 0; i < 2; i++) {
        NT::fr leaf = baseRollupInputs.kernel_data[i].public_inputs.constants.old_tree_roots.contract_tree_root;
        abis::MembershipWitness<NT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT> historic_root_witness =
            baseRollupInputs.historic_contract_tree_root_membership_witnesses[i];

        check_merkle_membership(
            leaf, historic_root_witness.leaf_index, historic_root_witness.sibling_path, historic_root);
    }
}
// Important types:
//   - BaseRollupPublicInputs - where we want to put our return values
//
// TODO: replace auto
BaseRollupPublicInputs base_rollup_circuit(BaseRollupInputs baseRollupInputs)
{

    // First we compute the contract tree leaves

    // Verify the previous kernel proofs
    for (size_t i = 0; i < 2; i++) {
        NT::Proof proof = baseRollupInputs.kernel_data[i].proof;
        assert(verify_kernel_proof(proof));
    }

    std::vector<NT::fr> contract_leaves = calculate_contract_leaves(baseRollupInputs);

    // Perform merkle membership check with the provided sibling path up to the root
    // check for commitments/private_data
    check_membership_of_subtree_in_snapshot(3,
                                            baseRollupInputs.start_private_data_tree_snapshot.next_available_leaf_index,
                                            baseRollupInputs.new_commitments_subtree_sibling_path,
                                            baseRollupInputs.start_private_data_tree_snapshot.root);
    // check for contracts
    check_membership_of_subtree_in_snapshot(2,
                                            baseRollupInputs.start_contract_tree_snapshot.next_available_leaf_index,
                                            baseRollupInputs.new_contracts_subtree_sibling_path,
                                            baseRollupInputs.start_contract_tree_snapshot.root);

    std::array<NT::fr, 3> new_subtrees = calculate_new_subtrees(baseRollupInputs, contract_leaves);
    NT::fr contracts_tree_subroot = new_subtrees[0];
    NT::fr commitments_tree_subroot = new_subtrees[1];
    // NT::fr nullifiers_tree_subroot = new_subtrees[2];

    // Insert subtrees to the tree:
    auto end_private_data_tree_snapshot =
        insert_subtree_to_private_data_tree(baseRollupInputs, commitments_tree_subroot);
    auto end_contract_tree_snapshot = insert_subtree_to_contracts_tree(baseRollupInputs, contracts_tree_subroot);

    // TODO: Nullifiers tree insertion

    // Calculate the overall calldata hash
    NT::fr calldata_hash = calculate_calldata_hash(baseRollupInputs, contract_leaves);

    // Perform membership checks that the notes provided exist within the historic trees data
    perform_historical_private_data_tree_membership_checks(baseRollupInputs);
    perform_historical_contract_data_tree_membership_checks(baseRollupInputs);

    AggregationObject aggregation_object = aggregate_proofs(baseRollupInputs);

    // TODO: update these mocks
    AppendOnlySnapshot mockNullifierStartSnapshot = {
        .root = NT::fr::one(),
        .next_available_leaf_index = 0,
    };
    AppendOnlySnapshot mockNullifierEndSnapshot = {
        .root = NT::fr::one(),
        .next_available_leaf_index = 0,
    };

    BaseRollupPublicInputs public_inputs = {
        .end_aggregation_object = aggregation_object,
        .constants = baseRollupInputs.constants,
        .start_private_data_tree_snapshot = baseRollupInputs.start_private_data_tree_snapshot,
        .end_private_data_tree_snapshot = end_private_data_tree_snapshot,
        .start_nullifier_tree_snapshot = mockNullifierStartSnapshot, // TODO: implement:
        .end_nullifier_tree_snapshot = mockNullifierEndSnapshot,     // TODO: implement:
        .start_contract_tree_snapshot = baseRollupInputs.start_contract_tree_snapshot,
        .end_contract_tree_snapshot = end_contract_tree_snapshot,
        .calldata_hash = calldata_hash,
    };
    return public_inputs;
}

} // namespace aztec3::circuits::rollup::native_base_rollup