#include "aztec3/circuits/abis/append_only_tree_snapshot.hpp"
#include "aztec3/circuits/abis/rollup/base/base_rollup_public_inputs.hpp"
#include "aztec3/circuits/abis/rollup/merge/merge_rollup_inputs.hpp"
#include "aztec3/circuits/abis/rollup/merge/merge_rollup_public_inputs.hpp"
#include "aztec3/constants.hpp"
#include "barretenberg/proof_system/verification_key/verification_key.hpp"
#include "index.hpp"
#include "init.hpp"

#include <aztec3/circuits/kernel/private/utils.hpp>
#include <aztec3/circuits/mock/mock_kernel_circuit.hpp>
#include "aztec3/circuits/abis/private_kernel/new_contract_data.hpp"
#include "aztec3/circuits/rollup/base/utils.hpp"
// #include "aztec3/circuits/abis/rollup/base/previous_rollup_data.hpp"

namespace {
using NT = aztec3::utils::types::NativeTypes;
using AggregationObject = aztec3::utils::types::NativeTypes::AggregationObject;

using aztec3::circuits::abis::AppendOnlyTreeSnapshot;
using aztec3::circuits::abis::BaseRollupPublicInputs;
using aztec3::circuits::abis::ConstantRollupData;
using aztec3::circuits::abis::MembershipWitness;
using aztec3::circuits::abis::MergeRollupInputs;
using aztec3::circuits::abis::MergeRollupPublicInputs;
using aztec3::circuits::abis::PreviousRollupData;
using aztec3::circuits::abis::private_kernel::NewContractData;

using aztec3::circuits::kernel::private_kernel::utils::dummy_previous_kernel_with_vk_proof;

using plonk::TurboComposer;
} // namespace

namespace aztec3::circuits::rollup::merge::utils {

// Helper method since MergeRollupInputs.PreviousRollupData can only take the type of `MergeRollupPublicInputs`
// So we convert the output of base to look like output of merge and add the `rollup_type` field.
MergeRollupPublicInputs<NT> convert_base_public_inputs_to_merge_public_inputs(
    BaseRollupPublicInputs<NT> baseRollupPublicInputs)
{
    MergeRollupPublicInputs<NT> mergeRollupPublicInputs = {
        .rollup_type = abis::BASE_ROLLUP_TYPE,
        .rollup_subtree_height = baseRollupPublicInputs.rollup_subtree_height,
        .end_aggregation_object = baseRollupPublicInputs.end_aggregation_object,
        .constants = baseRollupPublicInputs.constants,
        .start_private_data_tree_snapshot = baseRollupPublicInputs.start_private_data_tree_snapshot,
        .end_private_data_tree_snapshot = baseRollupPublicInputs.end_private_data_tree_snapshot,
        .start_nullifier_tree_snapshot = baseRollupPublicInputs.start_nullifier_tree_snapshot,
        .end_nullifier_tree_snapshot = baseRollupPublicInputs.end_nullifier_tree_snapshot,
        .start_contract_tree_snapshot = baseRollupPublicInputs.start_contract_tree_snapshot,
        .end_contract_tree_snapshot = baseRollupPublicInputs.end_contract_tree_snapshot,
        .calldata_hash = baseRollupPublicInputs.calldata_hash
    };
    return mergeRollupPublicInputs;
}

std::array<PreviousRollupData<NT>, 2> previous_rollups_with_vk_proof_that_follow_on()
{
    auto input1 = base::utils::dummy_base_rollup_inputs_with_vk_proof();
    BaseRollupPublicInputs base_public_input1 =
        aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(input1);

    auto input2 = input1;
    input2.start_private_data_tree_snapshot = base_public_input1.end_private_data_tree_snapshot;
    input2.start_nullifier_tree_snapshot = base_public_input1.end_nullifier_tree_snapshot;
    input2.start_contract_tree_snapshot = base_public_input1.end_contract_tree_snapshot;
    BaseRollupPublicInputs base_public_input2 =
        aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(input2);

    // just for mocked vk and proof
    // Need a way to extract a proof from Base Rollup Circuit. Until then use kernel as a hack.
    PreviousKernelData<NT> mocked_kernel = dummy_previous_kernel_with_vk_proof();

    PreviousRollupData<NT> previous_rollup1 = {
        .merge_rollup_public_inputs = convert_base_public_inputs_to_merge_public_inputs(base_public_input1),
        .proof = mocked_kernel.proof,
        .vk = mocked_kernel.vk,
        .vk_index = 0,
        .vk_sibling_path = MembershipWitness<NT, ROLLUP_VK_TREE_HEIGHT>(),
    };
    PreviousRollupData<NT> previous_rollup2 = {
        .merge_rollup_public_inputs = convert_base_public_inputs_to_merge_public_inputs(base_public_input2),
        .proof = mocked_kernel.proof,
        .vk = mocked_kernel.vk,
        .vk_index = 0,
        .vk_sibling_path = MembershipWitness<NT, ROLLUP_VK_TREE_HEIGHT>(),
    };

    return { previous_rollup1, previous_rollup2 };
}

MergeRollupInputs<NT> dummy_merge_rollup_inputs_with_vk_proof()
{
    MergeRollupInputs<NT> merge_rollup_inputs = { .previous_rollup_data =
                                                      previous_rollups_with_vk_proof_that_follow_on() };
    return merge_rollup_inputs;
}
} // namespace aztec3::circuits::rollup::merge::utils