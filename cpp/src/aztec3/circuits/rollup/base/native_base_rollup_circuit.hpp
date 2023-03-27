#pragma once

#include "init.hpp"

// TODO: not needed right at this moment for native impl
#include <barretenberg/stdlib/types/types.hpp>
#include <aztec3/utils/types/convert.hpp>
#include <aztec3/utils/types/circuit_types.hpp>
#include <aztec3/utils/types/native_types.hpp>

#include <aztec3/circuits/abis/base_rollup/base_rollup_inputs.hpp>
#include <aztec3/circuits/abis/base_rollup/base_rollup_public_inputs.hpp>
#include <aztec3/circuits/abis/base_rollup/nullifier_leaf_preimage.hpp>
#include <aztec3/circuits/abis/base_rollup/constant_base_rollup_data.hpp>

namespace aztec3::circuits::rollup::native_base_rollup {

void base_rollup_circuit(ConstantBaseRollupData constantRollupData, BaseRollupInputs baseRollupInputs);

} // namespace aztec3::circuits::rollup::native_base_rollup