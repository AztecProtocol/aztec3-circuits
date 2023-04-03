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
#include <cstdint>
#include <tuple>
#include <vector>

namespace aztec3::circuits::rollup::native_merge_rollup {

/**
 * @brief Create an aggregation object for the proofs that are provided
 *          - We add points P0 for each of our proofs
 *          - We add points P1 for each of our proofs
 *          - We concat our public inputs
 *
 * @param mergeRollupInputs
 * @return AggregationObject
 */
AggregationObject aggregate_proofs(MergeRollupInputs mergeRollupInputs)
{
    // TODO: NOTE: for now we simply return the aggregation object from the first proof
    return mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs.end_aggregation_object;
}

void assert_both_input_proofs_of_same_rollup_type(MergeRollupInputs mergeRollupInputs)
{
    assert(mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs.rollup_type ==
           mergeRollupInputs.previous_rollup_data[1].merge_rollup_public_inputs.rollup_type);
}

void assert_equal_constants(ConstantRollupData left, ConstantRollupData right)
{
    assert(left.base_rollup_vk_hash == right.base_rollup_vk_hash);
    assert(left.merge_rollup_vk_hash == right.merge_rollup_vk_hash);
    assert(left.private_kernel_vk_tree_root == right.private_kernel_vk_tree_root);
    assert(left.public_kernel_vk_tree_root == right.public_kernel_vk_tree_root);
    assert(left.start_tree_of_historic_private_data_tree_roots_snapshot ==
           right.start_tree_of_historic_private_data_tree_roots_snapshot);
    assert(left.start_tree_of_historic_contract_tree_roots_snapshot ==
           right.start_tree_of_historic_contract_tree_roots_snapshot);
    assert(left.tree_of_historic_l1_to_l2_msg_tree_roots_snapshot ==
           right.tree_of_historic_l1_to_l2_msg_tree_roots_snapshot);
}

// function that does sha256 hash of the calldata from each previous rollup data
std::array<fr, 2> compute_calldata_hash(MergeRollupInputs mergeRollupInputs)
{
    // Compute the calldata hash
    std::array<uint8_t, 2 * 32> calldata_hash_input_bytes;
    for (uint8_t i = 0; i < 2; i++) {
        std::array<fr, 2> calldata_hash_fr =
            mergeRollupInputs.previous_rollup_data[i].merge_rollup_public_inputs.calldata_hash;

        auto high_buffer = calldata_hash_fr[0].to_buffer();
        auto low_buffer = calldata_hash_fr[1].to_buffer();

        for (uint8_t j = 0; j < 16; ++j) {
            calldata_hash_input_bytes[i * 32 + j] = high_buffer[16 + j];
            calldata_hash_input_bytes[i * 32 + 16 + j] = low_buffer[16 + j];
        }
    }

    std::vector<uint8_t> calldata_hash_input_bytes_vec(calldata_hash_input_bytes.begin(),
                                                       calldata_hash_input_bytes.end());

    auto h = sha256::sha256(calldata_hash_input_bytes_vec);

    // Split the hash into two fields, a high and a low
    std::array<uint8_t, 32> buf_1, buf_2;
    for (uint8_t i = 0; i < 16; i++) {
        buf_1[i] = 0;
        buf_1[16 + i] = h[i];
        buf_2[i] = 0;
        buf_2[16 + i] = h[i + 16];
    }
    auto high = fr::serialize_from_buffer(buf_1.data());
    auto low = fr::serialize_from_buffer(buf_2.data());

    return { high, low };
}

// asserts that the end snapshot of previous_rollup 0 equals the start snapshot of previous_rollup 1 (i.e. ensure they
// follow on from one-another).
void ensure_prev_rollups_follow_on_from_each_other(MergeRollupInputs mergeRollupInputs)
{
    auto privateDataEndSnapshot0 =
        mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs.end_private_data_tree_snapshot;
    auto privateDataStartSnapshot1 =
        mergeRollupInputs.previous_rollup_data[1].merge_rollup_public_inputs.start_private_data_tree_snapshot;

    auto nullifierEndSnapshot0 =
        mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs.end_nullifier_tree_snapshot;
    auto nullifierStartSnapshot1 =
        mergeRollupInputs.previous_rollup_data[1].merge_rollup_public_inputs.start_nullifier_tree_snapshot;

    auto contractEndSnapshot0 =
        mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs.end_contract_tree_snapshot;
    auto contractStartSnapshot1 =
        mergeRollupInputs.previous_rollup_data[1].merge_rollup_public_inputs.start_contract_tree_snapshot;

    assert(privateDataEndSnapshot0 == privateDataStartSnapshot1 && nullifierEndSnapshot0 == nullifierStartSnapshot1 &&
           contractEndSnapshot0 == contractStartSnapshot1);
}

MergeRollupPublicInputs merge_rollup_circuit(MergeRollupInputs mergeRollupInputs)
{
    // Verify the previous rollup proofs

    // check that both input proofs are either both "BASE" or "MERGE" and not a mix!
    // this prevents having wonky commitment, nullifier and contract subtrees.
    assert_both_input_proofs_of_same_rollup_type(mergeRollupInputs);

    // TODO: Check both previous rollup vks (in previous_rollup_data) against the permitted set of kernel vks.
    // we don't have a set of permitted kernel vks yet.

    // compute calldata hash:
    auto new_calldata_hash = compute_calldata_hash(mergeRollupInputs);

    // Ensure the end snapshot of previous_rollup 0 equals the start snapshot of previous_rollup 1 (i.e. ensure they
    // follow on from one-another). This ensures the low_leaves which were updated in rollup 0 are being used as the
    // 'starting' pointers in rollup 1.
    ensure_prev_rollups_follow_on_from_each_other(mergeRollupInputs);

    AggregationObject aggregation_object = aggregate_proofs(mergeRollupInputs);

    // Check that the constants are the same in both proofs
    auto left = mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs;
    auto right = mergeRollupInputs.previous_rollup_data[1].merge_rollup_public_inputs;
    assert_equal_constants(left.constants, right.constants);

    MergeRollupPublicInputs public_inputs = {
        .end_aggregation_object = aggregation_object,
        .constants = left.constants,
        .start_private_data_tree_snapshot =
            mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs.start_private_data_tree_snapshot,
        .end_private_data_tree_snapshot =
            mergeRollupInputs.previous_rollup_data[1].merge_rollup_public_inputs.end_private_data_tree_snapshot,
        .start_nullifier_tree_snapshot =
            mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs.start_nullifier_tree_snapshot,
        .end_nullifier_tree_snapshot =
            mergeRollupInputs.previous_rollup_data[1].merge_rollup_public_inputs.end_nullifier_tree_snapshot,
        .start_contract_tree_snapshot =
            mergeRollupInputs.previous_rollup_data[0].merge_rollup_public_inputs.start_contract_tree_snapshot,
        .end_contract_tree_snapshot =
            mergeRollupInputs.previous_rollup_data[1].merge_rollup_public_inputs.end_contract_tree_snapshot,
        .calldata_hash = new_calldata_hash,
    };

    return public_inputs;
}

} // namespace aztec3::circuits::rollup::native_merge_rollup