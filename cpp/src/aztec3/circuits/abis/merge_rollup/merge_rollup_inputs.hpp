#pragma once
#include "aztec3/circuits/abis/append_only_tree_snapshot.hpp"
#include "aztec3/circuits/abis/merge_rollup/previous_rollup_data.hpp"
#include <aztec3/utils/types/native_types.hpp>
#include <aztec3/utils/types/circuit_types.hpp>
#include <aztec3/utils/types/convert.hpp>
#include <type_traits>

namespace aztec3::circuits::abis {

using aztec3::utils::types::CircuitTypes;
using aztec3::utils::types::NativeTypes;
using std::is_same;

template <typename NCT> struct MergeRollupInputs {
    typedef typename NCT::fr fr;

    std::array<PreviousRollupData<NCT>, 2> previous_rollup_data;
    fr prover_id;

    bool operator==(MergeRollupInputs<NCT> const&) const = default;
};

template <typename NCT> void read(uint8_t const*& it, MergeRollupInputs<NCT>& obj)
{
    using serialize::read;

    read(it, obj.previous_rollup_data);
    read(it, obj.prover_id);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, MergeRollupInputs<NCT> const& obj)
{
    using serialize::write;

    write(buf, obj.previous_rollup_data);
    write(buf, obj.prover_id);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, MergeRollupInputs<NCT> const& obj)
{
    return os << "previous_rollup_data: " << obj.previous_rollup_data << "\n"
              << "prover_id: " << obj.prover_id << "\n";
};

} // namespace aztec3::circuits::abis
