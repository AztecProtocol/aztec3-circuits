#include "aztec3/circuits/abis/rollup/base/base_rollup_public_inputs.hpp"
#include "aztec3/circuits/abis/rollup/merge/merge_rollup_inputs.hpp"
#include "aztec3/circuits/abis/rollup/merge/merge_rollup_public_inputs.hpp"
#include "aztec3/circuits/rollup/merge/utils.hpp"
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

namespace aztec3::circuits::rollup::base::utils {

// Helper method since MergeRollupInputs.PreviousRollupData can only take the type of `MergeRollupPublicInputs`
// So we convert the output of base to look like output of merge and add the `rollup_type` field.
MergeRollupPublicInputs<NT> convert_base_public_inputs_to_merge_public_inputs(
    BaseRollupPublicInputs<NT> baseRollupPublicInputs)
{
    MergeRollupPublicInputs<NT> mergeRollupPublicInputs = {
        .rollup_type = abis::BASE_ROLLUP_TYPE,
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

PreviousRollupData<NT> dummy_previous_rollup_with_vk_proof()
{
    // MergeInput requires base_rollup_public_inputs. So create a dummy BaseRollupInput object and pass it through the
    // base rollup circuit.
    auto emptyInputs = dummy_base_rollup_inputs_with_vk_proof();
    BaseRollupPublicInputs dummy_base_public_inputs =
        aztec3::circuits::rollup::native_base_rollup::base_rollup_circuit(emptyInputs);

    // just for mocked vk and proof
    // Need a way to extract a proof from Base Rollup Circuit. Until then use kernel as a hack.
    PreviousKernelData<NT> mocked_kernel = dummy_previous_kernel_with_vk_proof();

    PreviousRollupData<NT> previous_rollup = {
        .merge_rollup_public_inputs = convert_base_public_inputs_to_merge_public_inputs(dummy_base_public_inputs),
        .proof = mocked_kernel.proof,
        .vk = mocked_kernel.vk,
        .vk_index = 0,
        .vk_sibling_path = MembershipWitness<NT, ROLLUP_VK_TREE_HEIGHT>(),
    };

    return previous_rollup;
}

MergeRollupInputs<NT> dummy_merge_rollup_inputs_with_vk_proof()
{
    MergeRollupInputs<NT> merge_rollup_inputs = { .previous_rollup_data = { dummy_previous_rollup_with_vk_proof(),
                                                                            dummy_previous_rollup_with_vk_proof() } };
    return merge_rollup_inputs;
}
} // namespace aztec3::circuits::rollup::base::utils