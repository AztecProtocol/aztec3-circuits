#pragma once
#include "aztec3/circuits/abis/append_only_tree_snapshot.hpp"
#include <aztec3/utils/types/native_types.hpp>
#include <aztec3/utils/types/circuit_types.hpp>
#include <aztec3/utils/types/convert.hpp>
#include <type_traits>

namespace aztec3::circuits::abis {

using aztec3::utils::types::CircuitTypes;
using aztec3::utils::types::NativeTypes;
using std::is_same;

template <typename NCT> struct MergeRollupPublicInputs {
    typedef typename NCT::fr fr;
    typedef typename NCT::AggregationObject AggregationObject;

    AggregationObject aggregation_object;

    AppendOnlyTreeSnapshot<NCT> start_nullifier_tree_snapshot;
    AppendOnlyTreeSnapshot<NCT> end_nullifier_tree_snapshot;

    fr calldata_hash;
    fr prover_contribution_hash;

    bool operator==(MergeRollupPublicInputs<NCT> const&) const = default;
};

template <typename NCT> void read(uint8_t const*& it, MergeRollupPublicInputs<NCT>& obj)
{
    using serialize::read;

    read(it, obj.aggregation_object);
    read(it, obj.start_nullifier_tree_snapshot);
    read(it, obj.end_nullifier_tree_snapshot);
    read(it, obj.calldata_hash);
    read(it, obj.prover_contribution_hash);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, MergeRollupPublicInputs<NCT> const& obj)
{
    using serialize::write;

    write(buf, obj.aggregation_object);
    write(buf, obj.start_nullifier_tree_snapshot);
    write(buf, obj.end_nullifier_tree_snapshot);
    write(buf, obj.calldata_hash);
    write(buf, obj.prover_contribution_hash);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, MergeRollupPublicInputs<NCT> const& obj)
{
    return os << "aggregation_object: " << obj.aggregation_object << "\n"
              << "start_nullifier_tree_snapshot: " << obj.start_nullifier_tree_snapshot << "\n"
              << "end_nullifier_tree_snapshot: " << obj.end_nullifier_tree_snapshot << "\n"
              << "calldata_hash: " << obj.calldata_hash << "\n"
              << "prover_contribution_hash: " << obj.prover_contribution_hash << "\n";
};
} // namespace aztec3::circuits::abis
