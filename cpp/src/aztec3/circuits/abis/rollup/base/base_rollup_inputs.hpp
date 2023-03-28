#pragma once
#include "../../append_only_tree_snapshot.hpp"
#include "../../private_kernel/previous_kernel_data.hpp"
#include "../../membership_witness.hpp"
#include "../nullifier_leaf_preimage.hpp"
#include "../constant_rollup_data.hpp"

namespace aztec3::circuits::abis {

using aztec3::utils::types::CircuitTypes;
using aztec3::utils::types::NativeTypes;
using std::is_same;

template <typename NCT> struct MergeRollupInputs {

    typedef typename NCT::fr fr;

    std::array<private_kernel::PreviousKernelData<NCT>, 2> kernel_data;

    AppendOnlyTreeSnapshot<NCT> start_nullifier_tree_snapshot;
    std::array<NullifierLeafPreimage<NCT>, 2 * KERNEL_NEW_NULLIFIERS_LENGTH> low_nullifier_leaf_preimages;
    std::array<MembershipWitness<NCT, NULLIFIER_TREE_HEIGHT>, 2 * KERNEL_NEW_NULLIFIERS_LENGTH>
        low_nullifier_membership_witness;

    std::array<MembershipWitness<NCT, PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT>, 2>
        historic_private_data_tree_root_membership_witnesses;
    std::array<MembershipWitness<NCT, CONTRACT_TREE_ROOTS_TREE_HEIGHT>, 2>
        historic_contract_tree_root_membership_witnesses;

    ConstantRollupData<NCT> constants;

    bool operator==(BaseRollupInputs<NCT> const&) const = default;
};

template <typename NCT> void read(uint8_t const*& it, MergeRollupInputs<NCT>& obj)
{
    using serialize::read;

    read(it, obj.kernel_data);
    read(it, obj.start_nullifier_tree_snapshot);
    read(it, obj.low_nullifier_leaf_preimages);
    read(it, obj.low_nullifier_membership_witness);
    read(it, obj.historic_private_data_tree_root_membership_witnesses);
    read(it, obj.historic_contract_tree_root_membership_witnesses);
    read(it, obj.constants);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, MergeRollupInputs<NCT> const& obj)
{
    using serialize::write;

    write(buf, obj.kernel_data);
    write(buf, obj.start_nullifier_tree_snapshot);
    write(buf, obj.low_nullifier_leaf_preimages);
    write(buf, obj.low_nullifier_membership_witness);
    write(buf, obj.historic_private_data_tree_root_membership_witnesses);
    write(buf, obj.historic_contract_tree_root_membership_witnesses);
    write(buf, obj.constants);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, MergeRollupInputs<NCT> const& obj)
{
    return os << "kernel_data:\n"
              << obj.kernel_data << "\n"
              << "start_nullifier_tree_snapshot:\n"
              << obj.start_nullifier_tree_snapshot << "\n"
              << "low_nullifier_leaf_preimages:\n"
              << obj.low_nullifier_leaf_preimages << "\n"
              << "low_nullifier_membership_witness:\n"
              << obj.low_nullifier_membership_witness << "\n"
              << "historic_private_data_tree_root_membership_witnesses:\n"
              << obj.historic_private_data_tree_root_membership_witnesses << "\n"
              << "historic_contract_tree_root_membership_witnesses:\n"
              << obj.historic_contract_tree_root_membership_witnesses << "\n"
              << "constants:\n"
              << obj.constants << "\n";
}

} // namespace aztec3::circuits::abis