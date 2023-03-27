#pragma once

#include "init.hpp"

#include <aztec3/oracle/oracle.hpp>
#include <aztec3/circuits/apps/oracle_wrapper.hpp>
#include <aztec3/circuits/apps/function_execution_context.hpp>

#include <aztec3/circuits/recursion/aggregator.hpp>

#include <aztec3/circuits/abis/private_kernel/private_inputs.hpp>

#include <barretenberg/stdlib/types/types.hpp>
#include <aztec3/utils/types/convert.hpp>
#include <aztec3/utils/types/circuit_types.hpp>
#include <aztec3/utils/types/native_types.hpp>

#include <aztec3/circuits/abis/base_rollup/base_rollup_inputs.hpp>
#include <aztec3/circuits/abis/base_rollup/base_rollup_public_inputs.hpp>
#include <aztec3/circuits/abis/base_rollup/nullifier_leaf_preimage.hpp>
#include <aztec3/circuits/abis/base_rollup/constant_base_rollup_data.hpp>

namespace aztec3::circuits::rollup::native_base_rollup {

// Turbo specific, at the moment:
using Composer = plonk::stdlib::types::Composer;
using plonk::stdlib::types::Prover;

using Aggregator = aztec3::circuits::recursion::Aggregator;

// Generic:
using CT = aztec3::utils::types::CircuitTypes<Composer>;
using NT = aztec3::utils::types::NativeTypes;
using aztec3::utils::types::to_ct;

// TODO: not sure if we need this one
using FunctionExecutionContext = aztec3::circuits::apps::FunctionExecutionContext<Composer>;

void base_rollup_circuit(ConstantBaseRollupData<NT> constantRollupData, BaseRollupInputs<NT> baseRollupInputs);

} // namespace aztec3::circuits::rollup::native_base_rollup