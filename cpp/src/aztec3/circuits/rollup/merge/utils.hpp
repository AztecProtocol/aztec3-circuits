#pragma once
#include "aztec3/circuits/abis/rollup/base/base_rollup_public_inputs.hpp"
#include "index.hpp"
#include "init.hpp"

namespace aztec3::circuits::rollup::merge::utils {

namespace {
using NT = aztec3::utils::types::NativeTypes;
using aztec3::circuits::abis::BaseRollupPublicInputs;
using aztec3::circuits::abis::MergeRollupInputs;
using aztec3::circuits::abis::MergeRollupPublicInputs;
using aztec3::circuits::abis::PreviousRollupData;
} // namespace

MergeRollupPublicInputs<NT> convert_base_public_inputs_to_merge_public_inputs(BaseRollupPublicInputs<NT>);
MergeRollupInputs<NT> dummy_merge_rollup_inputs_with_vk_proof();
PreviousRollupData<NT> dummy_previous_rollup_with_vk_proof();

} // namespace aztec3::circuits::rollup::merge::utils