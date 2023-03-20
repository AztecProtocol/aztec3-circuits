#pragma once
#include "../append_only_tree_snapshot.hpp"
#include "../kernel/previous_kernel_data.hpp"
#include <stdlib/types/circuit_types.hpp>
#include <stdlib/types/convert.hpp>
#include <stdlib/types/native_types.hpp>
#include <aztec3/constants.hpp>

namespace aztec3::circuits::abis {

using plonk::stdlib::types::CircuitTypes;
using plonk::stdlib::types::NativeTypes;
using std::is_same;

template <typename NCT> struct BaseRollupInputs {

    typedef typename NCT::fr fr;

    std::array<PreviousKernelData<NCT>, 2> kernel_data;

    AppendOnlyTreeSnapshot<NCT> start_nullifier_tree_snapshot;
    std::array<NullifierLeafPreimage<NCT>, 2 * KERNEL_NEW_NULLIFIERS_LENGTH> low_nullifier_leaf_preimages;
    std::array<MembershipWitness<NCT, NULLIFIER_TREE_HEIGHT>, 2 * KERNEL_NEW_NULLIFIERS_LENGTH>
        low_nullifier_membership_witness;

    std::array<MembershipWitness<NCT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT>, 2>
        historic_private_data_tree_root_membership_witnesses;
    std::array<MembershipWitness<NCT, CONTRACT_TREE_ROOTS_TREE_HEIGHT>, 2>
        historic_contract_tree_root_membership_witnesses;

    ConstantBaseRollupData<NCT> constants;

    fr prover_id;

    bool operator==(BaseRollupInputs<NCT> const&) const = default;
};

template <typename NCT> void read(uint8_t const*& it, BaseRollupInputs<NCT>& obj)
{
    using serialize::read;

    read(it, obj.kernel_data);
    read(it, obj.start_nullifier_tree_snapshot);
    read(it, obj.low_nullifier_leaf_preimages);
    read(it, obj.low_nullifier_membership_witness);
    read(it, obj.historic_private_data_tree_root_membership_witnesses);
    read(it, obj.historic_contract_tree_root_membership_witnesses);
    read(it, obj.constants);
    read(it, obj.prover_id);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, BaseRollupInputs<NCT> const& obj)
{
    using serialize::write;

    write(buf, obj.kernel_data);
    write(buf, obj.start_nullifier_tree_snapshot);
    write(buf, obj.low_nullifier_leaf_preimages);
    write(buf, obj.low_nullifier_membership_witness);
    write(buf, obj.historic_private_data_tree_root_membership_witnesses);
    write(buf, obj.historic_contract_tree_root_membership_witnesses);
    write(buf, obj.constants);
    write(buf, obj.prover_id);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, BaseRollupInputs<NCT> const& obj)
{
    return os << "kernel_data: " << obj.kernel_data << "\n"
              << "start_nullifier_tree_snapshot: " << obj.start_nullifier_tree_snapshot << "\n"
              << "low_nullifier_leaf_preimages: " << obj.low_nullifier_leaf_preimages << "\n"
              << "low_nullifier_membership_witness: " << obj.low_nullifier_membership_witness << "\n"
              << "historic_private_data_tree_root_membership_witnesses: "
              << obj.historic_private_data_tree_root_membership_witnesses << "\n"
              << "historic_contract_tree_root_membership_witnesses: "
              << obj.historic_contract_tree_root_membership_witnesses << "\n"
              << "constants: " << obj.constants << "\n"
              << "prover_id: " << obj.prover_id << "\n";
}

} // namespace aztec3::circuits::abis