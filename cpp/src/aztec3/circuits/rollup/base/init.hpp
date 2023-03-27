
#pragma once

#include "aztec3/circuits/abis/append_only_tree_snapshot.hpp"
#include "aztec3/circuits/abis/base_rollup/constant_base_rollup_data.hpp"
#include "aztec3/circuits/abis/base_rollup/base_rollup_inputs.hpp"
#include "aztec3/circuits/abis/base_rollup/base_rollup_public_inputs.hpp"
#include <aztec3/circuits/recursion/aggregator.hpp>
#include <aztec3/circuits/abis/private_circuit_public_inputs.hpp>

#include <barretenberg/crypto/sha256/sha256.hpp>
#include <barretenberg/stdlib/types/types.hpp>
#include <aztec3/utils/types/convert.hpp>
#include <aztec3/utils/types/circuit_types.hpp>
#include <aztec3/utils/types/native_types.hpp>

namespace aztec3::circuits::rollup::native_base_rollup {

using NT = aztec3::utils::types::NativeTypes;

// Params
using ConstantBaseRollupData = abis::ConstantBaseRollupData<NT>;
using BaseRollupInputs = abis::BaseRollupInputs<NT>;
using BaseRollupPublicInputs = abis::BaseRollupPublicInputs<NT>;

using AggregationObject = stdlib::recursion::native_recursion_output;
using AppendOnlySnapshot = abis::AppendOnlyTreeSnapshot<NT>;

} // namespace aztec3::circuits::rollup::native_base_rollup