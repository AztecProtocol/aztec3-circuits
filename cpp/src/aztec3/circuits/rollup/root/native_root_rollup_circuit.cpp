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
#include <aztec3/circuits/abis/rollup/root/root_rollup_inputs.hpp>
#include <aztec3/circuits/abis/rollup/root/root_rollup_public_inputs.hpp>
#include <aztec3/circuits/abis/rollup/nullifier_leaf_preimage.hpp>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <vector>

namespace aztec3::circuits::rollup::native_root_rollup {

// TODO: can we aggregate proofs if we do not have a working circuit impl
// TODO: change the public inputs array - we wont be using this?

// Access Native types through NT namespace

bool verify_merge_proof(NT::Proof merge_proof)
{
    (void)merge_proof;
    return true;
}

AggregationObject aggregate_proofs(RootRollupInputs rootRollupInputs)
{
    // TODO: NOTE: for now we simply return the aggregation object from the first proof
    return rootRollupInputs.previous_rollup_data[0].base_rollup_public_inputs.end_aggregation_object;
}

std::array<fr, 2> compute_calldata_hash(RootRollupInputs rootRollupInputs)
{

    // Compute the calldata hash
    std::array<uint8_t, 2 * 32> calldata_hash_input_bytes;
    for (uint8_t i = 0; i < 2; i++) {
        std::array<fr, 2> calldata_hash_fr =
            rootRollupInputs.previous_rollup_data[i].base_rollup_public_inputs.calldata_hash;

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

// Important types:
//   - BaseRollupPublicInputs - where we want to put our return values
//
// TODO: replace auto
RootRollupPublicInputs root_rollup_circuit(RootRollupInputs rootRollupInputs)
{
    // TODO: Check the historic trees as well
    // old -> leftmost
    // new -> rightmost

    AggregationObject aggregation_object = aggregate_proofs(rootRollupInputs);

    // Verify the previous merge proofs (for now these are actually base proofs)
    for (size_t i = 0; i < 2; i++) {
        NT::Proof proof = rootRollupInputs.previous_rollup_data[i].proof;
        assert(verify_merge_proof(proof));
    }

    auto left = rootRollupInputs.previous_rollup_data[0].base_rollup_public_inputs;
    auto right = rootRollupInputs.previous_rollup_data[1].base_rollup_public_inputs;

    // Compute the historic trees.

    RootRollupPublicInputs public_inputs = {
        .end_aggregation_object = aggregation_object,
        .start_private_data_tree_snapshot = left.start_private_data_tree_snapshot,
        .end_private_data_tree_snapshot = right.end_private_data_tree_snapshot,
        .start_nullifier_tree_snapshot = left.start_nullifier_tree_snapshot,
        .end_nullifier_tree_snapshot = right.end_nullifier_tree_snapshot,
        .start_contract_tree_snapshot = left.start_contract_tree_snapshot,
        .end_contract_tree_snapshot = right.end_contract_tree_snapshot,
        .start_tree_of_private_data_tree_roots_snapshot = AppendOnlySnapshot::empty(),
        .end_tree_of_private_data_tree_roots_snapshot = AppendOnlySnapshot::empty(),
        .start_tree_of_historic_contract_tree_roots_snapshot = AppendOnlySnapshot::empty(),
        .end_tree_of_historic_contract_tree_roots_snapshot = AppendOnlySnapshot::empty(),
        .calldata_hash = compute_calldata_hash(rootRollupInputs),
    };
    return public_inputs;
}

} // namespace aztec3::circuits::rollup::native_root_rollup