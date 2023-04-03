#include <aztec3/circuits/abis/function_data.hpp>
#include <aztec3/constants.hpp>

namespace aztec3::circuits {

using abis::FunctionData;

template <typename NCT> typename NCT::fr compute_args_hash(std::array<typename NCT::fr, ARGS_LENGTH> args)
{
    return NCT::compress(args, CONSTRUCTOR_ARGS);
}

template <typename NCT>
typename NCT::fr compute_constructor_hash(FunctionData<NCT> function_data,
                                          std::array<typename NCT::fr, ARGS_LENGTH> args,
                                          typename NCT::fr constructor_vk_hash)
{
    using fr = typename NCT::fr;

    fr function_data_hash = function_data.hash();
    fr args_hash = compute_args_hash<NCT>(args);

    std::vector<fr> inputs = {
        function_data_hash,
        args_hash,
        constructor_vk_hash,
    };

    return NCT::compress(inputs, aztec3::GeneratorIndex::CONSTRUCTOR);
}

template <typename NCT>
typename NCT::address compute_contract_address(typename NCT::address deployer_address,
                                               typename NCT::fr contract_address_salt,
                                               typename NCT::fr function_tree_root,
                                               typename NCT::fr constructor_hash)
{
    using fr = typename NCT::fr;
    using address = typename NCT::address;

    std::vector<fr> inputs = {
        deployer_address.to_field(),
        contract_address_salt,
        function_tree_root,
        constructor_hash,
    };

    return address(NCT::compress(inputs, aztec3::GeneratorIndex::CONTRACT_ADDRESS));
}
} // namespace aztec3::circuits