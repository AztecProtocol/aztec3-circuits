#pragma once
#include <aztec3/utils/types/native_types.hpp>
#include <aztec3/utils/types/circuit_types.hpp>
#include <aztec3/utils/types/convert.hpp>
#include "../../append_only_tree_snapshot.hpp"
#include "../../append_only_tree_snapshot.hpp"
#include "../constant_rollup_data.hpp"
#include "barretenberg/common/serialize.hpp"

namespace aztec3::circuits::abis {

using aztec3::utils::types::CircuitTypes;
using aztec3::utils::types::NativeTypes;
using std::is_same;

const uint32_t BASE_ROLLUP_TYPE = 0;
const uint32_t MERGE_ROLLUP_TYPE = 1;

template <typename NCT> struct MergeRollupPublicInputs {
    typedef typename NCT::fr fr;
    typedef typename NCT::AggregationObject AggregationObject;

    uint32_t rollup_type;
    fr rollup_subtree_height;

    AggregationObject end_aggregation_object;

    ConstantRollupData<NCT> constants;

    AppendOnlyTreeSnapshot<NCT> start_private_data_tree_snapshot;
    AppendOnlyTreeSnapshot<NCT> end_private_data_tree_snapshot;

    // The only tree root actually updated in this circuit is the nullifier tree, because earlier leaves (of
    // low_nullifiers) must be updated to point to the new nullifiers in this circuit.
    AppendOnlyTreeSnapshot<NCT> start_nullifier_tree_snapshot;
    AppendOnlyTreeSnapshot<NCT> end_nullifier_tree_snapshot;

    AppendOnlyTreeSnapshot<NCT> start_contract_tree_snapshot;
    AppendOnlyTreeSnapshot<NCT> end_contract_tree_snapshot;

    std::array<fr, 2> calldata_hash;

    bool operator==(MergeRollupPublicInputs<NCT> const&) const = default;
};

template <typename NCT> void read(uint8_t const*& it, MergeRollupPublicInputs<NCT>& obj)
{
    using serialize::read;

    read(it, obj.rollup_type);
    read(it, obj.rollup_subtree_height);
    read(it, obj.end_aggregation_object);
    read(it, obj.constants);
    read(it, obj.start_private_data_tree_snapshot);
    read(it, obj.end_private_data_tree_snapshot);
    read(it, obj.start_nullifier_tree_snapshot);
    read(it, obj.end_nullifier_tree_snapshot);
    read(it, obj.start_contract_tree_snapshot);
    read(it, obj.end_contract_tree_snapshot);
    read(it, obj.calldata_hash);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, MergeRollupPublicInputs<NCT> const& obj)
{
    using serialize::write;

    write(buf, obj.rollup_type);
    write(buf, obj.rollup_subtree_height);
    write(buf, obj.end_aggregation_object);
    write(buf, obj.constants);
    write(buf, obj.start_private_data_tree_snapshot);
    write(buf, obj.end_private_data_tree_snapshot);
    write(buf, obj.start_nullifier_tree_snapshot);
    write(buf, obj.end_nullifier_tree_snapshot);
    write(buf, obj.start_contract_tree_snapshot);
    write(buf, obj.end_contract_tree_snapshot);
    write(buf, obj.calldata_hash);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, MergeRollupPublicInputs<NCT> const& obj)
{
    return os << "rollup_type: " << obj.rollup_type
              << "\n"
                 "rollup_subtree_height:\n"
              << obj.rollup_subtree_height << "\n"
              << "end_aggregation_object:\n"
              << obj.end_aggregation_object
              << "\n"
                 "constants:\n"
              << obj.constants
              << "\n"
                 "start_private_data_tree_snapshot:\n"
              << obj.start_private_data_tree_snapshot
              << "\n"
                 "end_private_data_tree_snapshot:\n"
              << obj.end_private_data_tree_snapshot
              << "\n"
                 "start_nullifier_tree_snapshot:\n"
              << obj.start_nullifier_tree_snapshot
              << "\n"
                 "end_nullifier_tree_snapshot:\n"
              << obj.end_nullifier_tree_snapshot
              << "\n"
                 "start_contract_tree_snapshot:\n"
              << obj.start_contract_tree_snapshot
              << "\n"
                 "end_contract_tree_snapshot:\n"
              << obj.end_contract_tree_snapshot
              << "\n"
                 "calldata_hash: "
              << obj.calldata_hash << "\n";
}

} // namespace aztec3::circuits::abis