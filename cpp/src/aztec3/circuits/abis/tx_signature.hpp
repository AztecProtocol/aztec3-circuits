#pragma once
#include <barretenberg/crypto/generators/generator_data.hpp>
#include <barretenberg/stdlib/hash/pedersen/pedersen.hpp>
#include <barretenberg/stdlib/primitives/witness/witness.hpp>
#include <aztec3/utils/types/circuit_types.hpp>
#include <aztec3/utils/types/convert.hpp>
#include <aztec3/utils/types/native_types.hpp>
#include <aztec3/constants.hpp>

namespace aztec3::circuits::abis {

using aztec3::utils::types::CircuitTypes;
using aztec3::utils::types::NativeTypes;
using plonk::stdlib::witness_t;
using std::is_same;

template <typename NCT> struct TxSignature {
    typedef typename NCT::ecdsa_signature ecdsa_signature;
    typedef typename NCT::uint32 uint32;
    typedef typename NCT::boolean boolean;
    typedef typename NCT::grumpkin_point
        grumpkin_public_key; // TODO: We use grumpkin curve for now, change to secp256k1 later.
    typedef typename NCT::fr fr;

    grumpkin_public_key public_key; // signing public key
    ecdsa_signature signature;      // signature data
    std::string message;            // message to be signed over

    bool operator==(TxSignature<NCT> const&) const = default;

    static TxSignature<NCT> empty()
    {
        std::string empty_message = "empty";
        grumpkin_public_key generator = NCT::grumpkin_group::affine_one;
        std::vector<uint8_t> zeroes(32, 0);
        ecdsa_signature zero_signature = { zeroes, zeroes };
        return { generator, zero_signature, empty_message };
    };

    template <typename Composer> TxSignature<CircuitTypes<Composer>> to_circuit_type(Composer& composer) const
    {
        static_assert((std::is_same<NativeTypes, NCT>::value));

        // Capture the composer:
        auto to_ct = [&](auto& e) { return aztec3::utils::types::to_ct(composer, e); };

        TxSignature<CircuitTypes<Composer>> tx_signature = {
            to_ct(public_key),
            to_ct(signature),
            to_ct(message),
        };

        return tx_signature;
    };

    template <typename Composer> TxSignature<NativeTypes> to_native_type() const
    {
        static_assert(std::is_same<CircuitTypes<Composer>, NCT>::value);
        auto to_nt = [&](auto& e) { return aztec3::utils::types::to_nt<Composer>(e); };

        TxSignature<NativeTypes> fs = {
            to_nt(function_encoding),
            to_nt(is_private),
            to_nt(is_constructor),
        };

        return fs;
    };

    void set_public()
    {
        static_assert(!(std::is_same<NativeTypes, NCT>::value));

        fr(function_encoding).set_public();
        fr(is_private).set_public();
        fr(is_constructor).set_public();
    }

    // TODO: this can all be packed into 1 field element, so this `hash` function should just return that field element.
    fr hash() const
    {
        std::vector<fr> inputs = {
            fr(function_encoding),
            fr(is_private),
            fr(is_constructor),
        };

        return NCT::compress(inputs, GeneratorIndex::FUNCTION_SIGNATURE);
    }
};

template <typename NCT> void read(uint8_t const*& it, TxSignature<NCT>& function_signature)
{
    using serialize::read;

    read(it, function_signature.function_encoding);
    read(it, function_signature.is_private);
    read(it, function_signature.is_constructor);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, TxSignature<NCT> const& function_signature)
{
    using serialize::write;

    write(buf, function_signature.function_encoding);
    write(buf, function_signature.is_private);
    write(buf, function_signature.is_constructor);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, TxSignature<NCT> const& function_signature)
{
    return os << "function_encoding: " << function_signature.function_encoding << "\n"
              << "is_private: " << function_signature.is_private << "\n"
              << "is_constructor: " << function_signature.is_constructor << "\n";
}

} // namespace aztec3::circuits::abis