#include "init.hpp"

#include <aztec3/circuits/abis/base_rollup/base_rollup_inputs.hpp>
#include <aztec3/circuits/abis/base_rollup/base_rollup_public_inputs.hpp>
#include <aztec3/circuits/abis/base_rollup/constant_base_rollup_data.hpp>
#include <aztec3/circuits/abis/base_rollup/nullifier_leaf_preimage.hpp>

namespace aztec3::circuits::rollup::native_base_rollup {

// TODO: can we aggregate proofs if we do not have a working circuit impl

// TODO: change the public inputs array - we wont be using this?

void base_rollup_circuit(ConstantBaseRollupData<NT> constantRollupData, BaseRollupInputs<NT> baseRollupInputs);

// Verify both of the input kernel proofs

// TODO: THE BASE ROLLUP ABI ALREADY EXISTS!

} // namespace aztec3::circuits::rollup::native_base_rollup