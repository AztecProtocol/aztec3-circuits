#pragma once
#include "../optionally_revealed_data.hpp"
#include "./new_contract_data.hpp"
#include <common/map.hpp>
#include <stdlib/primitives/witness/witness.hpp>
#include <stdlib/types/native_types.hpp>
#include <stdlib/types/circuit_types.hpp>
#include <stdlib/types/convert.hpp>

namespace aztec3::circuits::abis::private_kernel {

using plonk::stdlib::witness_t;
using plonk::stdlib::types::CircuitTypes;
using plonk::stdlib::types::NativeTypes;
using std::is_same;

template <typename NCT> struct AccumulatedData {
    typedef typename NCT::fr fr;
    typedef typename NCT::AggregationObject AggregationObject;

    AggregationObject aggregation_object;

    fr private_call_count;

    std::array<fr, KERNEL_NEW_COMMITMENTS_LENGTH> new_commitments;
    std::array<fr, KERNEL_NEW_NULLIFIERS_LENGTH> new_nullifiers;

    std::array<fr, KERNEL_PRIVATE_CALL_STACK_LENGTH> private_call_stack;
    std::array<fr, KERNEL_PUBLIC_CALL_STACK_LENGTH> public_call_stack;
    std::array<fr, KERNEL_L1_MSG_STACK_LENGTH> l1_msg_stack;

    std::array<NewContractData<NCT>, KERNEL_NEW_CONTRACTS_LENGTH> new_contracts;

    std::array<OptionallyRevealedData<NCT>, KERNEL_OPTIONALLY_REVEALED_DATA_LENGTH> optionally_revealed_data;

    template <typename Composer> AccumulatedData<CircuitTypes<Composer>> to_circuit_type(Composer& composer) const
    {
        typedef CircuitTypes<Composer> CT;
        static_assert((std::is_same<NativeTypes, NCT>::value));

        // Capture the composer:
        auto to_ct = [&](auto& e) { return plonk::stdlib::types::to_ct(composer, e); };
        auto to_circuit_type = [&](auto& e) { return e.to_circuit_type(composer); };

        AccumulatedData<CT> acc_data = {
            typename CT::AggregationObject{
                to_ct(aggregation_object.P0),
                to_ct(aggregation_object.P1),
                to_ct(aggregation_object.public_inputs),
                aggregation_object.proof_witness_indices,
                aggregation_object.has_data,
            },

            to_ct(private_call_count),

            to_ct(new_commitments),
            to_ct(new_nullifiers),

            to_ct(private_call_stack),
            to_ct(public_call_stack),
            to_ct(l1_msg_stack),

            map(new_contracts, to_circuit_type),
            map(optionally_revealed_data, to_circuit_type),
        };

        return acc_data;
    };

    template <typename Composer> AccumulatedData<NativeTypes> to_native_type() const
    {
        static_assert(std::is_same<CircuitTypes<Composer>, NCT>::value);
        auto to_nt = [&](auto& e) { return plonk::stdlib::types::to_nt<Composer>(e); };
        auto to_native_type = []<typename T>(T& e) { return e.template to_native_type<Composer>(); };

        AccumulatedData<NativeTypes> acc_data = {
            typename NativeTypes::AggregationObject{
                to_nt(aggregation_object.P0),
                to_nt(aggregation_object.P1),
                to_nt(aggregation_object.public_inputs),
                aggregation_object.proof_witness_indices,
                aggregation_object.has_data,
            },

            to_nt(private_call_count),

            to_nt(new_commitments),
            to_nt(new_nullifiers),

            to_nt(private_call_stack),
            to_nt(public_call_stack),
            to_nt(l1_msg_stack),

            map(new_contracts, to_native_type),
            map(optionally_revealed_data, to_native_type),
        };

        return acc_data;
    };

    void set_public()
    {
        static_assert(!(std::is_same<NativeTypes, NCT>::value));

        aggregation_object.add_proof_outputs_as_public_inputs();

        private_call_count.set_public();

        set_array_public(new_commitments);
        set_array_public(new_nullifiers);

        set_array_public(private_call_stack);
        set_array_public(public_call_stack);
        set_array_public(l1_msg_stack);

        set_array_public(new_contracts);
        set_array_public(optionally_revealed_data);
    }

    template <typename T, size_t SIZE> void set_array_public(std::array<T, SIZE>& arr)
    {
        static_assert(!(std::is_same<NativeTypes, NCT>::value));
        for (T& e : arr) {
            fr(e).set_public();
        }
    }

    template <size_t SIZE> void set_array_public(std::array<OptionallyRevealedData<NCT>, SIZE>& arr)
    {
        static_assert(!(std::is_same<NativeTypes, NCT>::value));
        for (auto& e : arr) {
            e.set_public();
        }
    }

    template <size_t SIZE> void set_array_public(std::array<NewContractData<NCT>, SIZE>& arr)
    {
        static_assert(!(std::is_same<NativeTypes, NCT>::value));
        for (auto& e : arr) {
            e.set_public();
        }
    }
};

template <typename NCT> void read(uint8_t const*& it, AccumulatedData<NCT>& accum_data)
{
    using serialize::read;

    read(it, accum_data.aggregation_object);
    read(it, accum_data.private_call_count);
    read(it, accum_data.new_commitments);
    read(it, accum_data.new_nullifiers);
    read(it, accum_data.private_call_stack);
    read(it, accum_data.public_call_stack);
    read(it, accum_data.l1_msg_stack);
    read(it, accum_data.new_contracts);
    read(it, accum_data.optionally_revealed_data);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, AccumulatedData<NCT> const& accum_data)
{
    using serialize::write;

    write(buf, accum_data.aggregation_object);
    write(buf, accum_data.private_call_count);
    write(buf, accum_data.new_commitments);
    write(buf, accum_data.new_nullifiers);
    write(buf, accum_data.private_call_stack);
    write(buf, accum_data.public_call_stack);
    write(buf, accum_data.l1_msg_stack);
    write(buf, accum_data.new_contracts);
    write(buf, accum_data.optionally_revealed_data);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, AccumulatedData<NCT> const& accum_data)
{
    return os << "aggregation_object: " << accum_data.aggregation_object << "\n"
              << "private_call_count: " << accum_data.private_call_count << "\n"
              << "new_commitments: " << accum_data.new_commitments << "\n"
              << "new_nullifiers: " << accum_data.new_nullifiers << "\n"
              << "private_call_stack: " << accum_data.private_call_stack << "\n"
              << "public_call_stack: " << accum_data.public_call_stack << "\n"
              << "l1_msg_stack: " << accum_data.l1_msg_stack << "\n"
              << "new_contracts: " << accum_data.new_contracts << "\n"
              << "optionally_revealed_data: " << accum_data.optionally_revealed_data << "\n";
}

} // namespace aztec3::circuits::abis::private_kernel