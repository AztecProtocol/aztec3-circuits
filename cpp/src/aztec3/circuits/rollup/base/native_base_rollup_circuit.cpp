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
#include <aztec3/circuits/abis/base_rollup/base_rollup_inputs.hpp>
#include <aztec3/circuits/abis/base_rollup/base_rollup_public_inputs.hpp>
#include <aztec3/circuits/abis/base_rollup/constant_base_rollup_data.hpp>
#include <aztec3/circuits/abis/base_rollup/nullifier_leaf_preimage.hpp>
#include <cstdint>
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

            contract_leaves.push_back(contract_leaf);
        }
    }

    return contract_leaves;
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

    return sha256::sha256_to_field(calldata_hash_inputs_bytes_vec);
}

void check_membership(NT::fr root,
                      NT::fr leaf,
                      abis::MembershipWitness<NT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT> witness)
{
    // Extract values
    NT::uint32 leaf_index = witness.leaf_index;
    auto sibling_path = witness.sibling_path;

    // Perform merkle membership check with the provided sibling path up to the root
    for (size_t i = 0; i < PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT; i++) {
        if (leaf_index & (1 << i)) {
            leaf = crypto::pedersen_hash::hash_multiple({ leaf, sibling_path[i] });
        } else {
            leaf = crypto::pedersen_hash::hash_multiple({ sibling_path[i], leaf });
        }
    }
    if (leaf != root) {
        throw std::runtime_error("Merkle membership check failed");
    }
}
/**
 * @brief Check all of the provided commitments against the historical tree roots
 *
 * @param constantBaseRollupData
 * @param baseRollupInputs
 */
void perform_historical_private_data_tree_membership_checks(ConstantBaseRollupData constantBaseRollupData,
                                                            BaseRollupInputs baseRollupInputs)
{
    // For each of the historic_private_data_tree_membership_checks, we need to do an inclusion proof
    // against the historical root provided in the rollup constants
    auto historic_root = constantBaseRollupData.start_tree_of_historic_private_data_tree_roots_snapshot.root;

    // TODO: why are there two witnesses per historic data root witness?

    for (size_t i = 0; i < 2; i++) {
        NT::fr leaf = baseRollupInputs.kernel_data[i].public_inputs.constants.old_tree_roots.private_data_tree_root;
        abis::MembershipWitness<NT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT> historic_root_witness =
            baseRollupInputs.historic_private_data_tree_root_membership_witnesses[i];

        check_membership(historic_root, leaf, historic_root_witness);
    }
}

void perform_historical_contract_data_tree_membership_checks(ConstantBaseRollupData constantBaseRollupData,
                                                             BaseRollupInputs baseRollupInputs)
{
    auto historic_root = constantBaseRollupData.start_tree_of_historic_contract_tree_roots_snapshot.root;

    // TODO: why are there two witnesses per historic data root witness?

    for (size_t i = 0; i < 2; i++) {
        NT::fr leaf = baseRollupInputs.kernel_data[i].public_inputs.constants.old_tree_roots.contract_tree_root;
        abis::MembershipWitness<NT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT> historic_root_witness =
            baseRollupInputs.historic_contract_tree_root_membership_witnesses[i];

        check_membership(historic_root, leaf, historic_root_witness);
    }
}
// Important types:
//   - BaseRollupPublicInputs - where we want to put our return values
//
// TODO: replace auto
BaseRollupPublicInputs base_rollup_circuit(ConstantBaseRollupData constantBaseRollupData,
                                           BaseRollupInputs baseRollupInputs)
{

    // First we compute the contract tree leaves

    // Verify the previous kernel proofs
    for (size_t i = 0; i < 2; i++) {
        NT::Proof proof = baseRollupInputs.kernel_data[i].proof;
        assert(verify_kernel_proof(proof));
    }

    std::vector<NT::fr> contract_leaves = calculate_contract_leaves(baseRollupInputs);

    std::array<NT::fr, 3> new_subtrees = calculate_new_subtrees(baseRollupInputs, contract_leaves);
    NT::fr contracts_tree_subroot = new_subtrees[0];
    NT::fr commitments_tree_subroot = new_subtrees[1];
    NT::fr nullifiers_tree_subroot = new_subtrees[2];

    // Calculate the overall calldata hash
    NT::fr calldata_hash = calculate_calldata_hash(baseRollupInputs, contract_leaves);

    // TODO: do a membership check that the notes provided exist within the historic trees data
    perform_historical_private_data_tree_membership_checks(constantBaseRollupData, baseRollupInputs);
    perform_historical_contract_data_tree_membership_checks(constantBaseRollupData, baseRollupInputs);

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

    NT::fr prover_contribution_hash = get_prover_contribution_hash(); // TODO: implement

    BaseRollupPublicInputs public_inputs = {
        .end_aggregation_object = aggregation_object,
        .constants = constantBaseRollupData,
        .start_nullifier_tree_snapshot = mockNullifierStartSnapshot, // TODO: implement:
        .end_nullifier_tree_snapshots = mockNullifierEndSnapshot,    // TODO: implement
        .new_commitments_subtree_root = commitments_tree_subroot,
        .new_nullifiers_subtree_root = nullifiers_tree_subroot,
        .new_contract_leaves_subtree_root = contracts_tree_subroot,
        .calldata_hash = calldata_hash,
        .prover_contributions_hash = prover_contribution_hash,
    };
    return public_inputs;
}

} // namespace aztec3::circuits::rollup::native_base_rollup