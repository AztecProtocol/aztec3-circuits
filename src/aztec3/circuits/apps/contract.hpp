#pragma once

#include "function_declaration.hpp"
#include "l1_function_interface.hpp"

#include <aztec3/circuits/abis/function_signature.hpp>

#include <common/container.hpp>

namespace aztec3::circuits::apps {

using aztec3::circuits::abis::FunctionSignature;

using plonk::stdlib::witness_t;
using plonk::stdlib::types::CircuitTypes;
using NT = plonk::stdlib::types::NativeTypes;

template <typename Composer> class FunctionExecutionContext;

template <typename Composer> class Contract {
    typedef CircuitTypes<Composer> CT;
    typedef typename CT::fr fr;
    typedef typename CT::uint32 uint32;

  public:
    FunctionExecutionContext<Composer>& exec_ctx;

    const std::string contract_name;

    fr state_var_counter = 0;
    std::vector<std::string> state_var_names;
    std::map<std::string, fr> start_slots_by_state_var_name;

    std::map<std::string, FunctionSignature<CT>> function_signatures;

    std::map<std::string, L1FunctionInterface<Composer>> l1_functions;

    Contract<Composer>(FunctionExecutionContext<Composer>& exec_ctx, std::string const& contract_name)
        : exec_ctx(exec_ctx)
        , contract_name(contract_name)
    {
        exec_ctx.register_contract(this);
    }

    void set_functions(std::vector<FunctionDeclaration<CT>> const& functions);

    // TODO: return some Function class which has a `call` method...
    // FunctionSignature<CT> get_function(std::string name) { return function_signature[name]; }

    FunctionSignature<CT> get_function_signature_by_name(std::string const& name);

    void import_l1_function(L1FunctionInterfaceStruct<Composer> const& l1_function_struct);

    L1FunctionInterface<Composer>& get_l1_function(std::string const& name);

    // TODO: maybe also declare a type at this stage, so the correct type can be checked-for when the StateVar type is
    // created within the function.
    /**
     * Note: this simply tracks the 'start' storage slots of each state variable at the 'contract scope level'.
     * TODO: maybe we can just keep a vector of names and query the start slot with index_of(), instead.
     */
    void declare_state_var(std::string const& state_var_name)
    {
        push_new_state_var_name(state_var_name);
        start_slots_by_state_var_name[state_var_name] = state_var_counter;
        // state_var_counter++;
        state_var_counter++;
        ASSERT(state_var_counter.get_value() == state_var_names.size());
    };

    fr& get_start_slot(std::string const& state_var_name)
    {
        if (!start_slots_by_state_var_name.contains(state_var_name)) {
            throw_or_abort("Name '" + state_var_name + "' not found. Use `declare_private_state_var`.");
        }
        return start_slots_by_state_var_name.at(state_var_name);
    };

  private:
    void push_new_state_var_name(std::string const& state_var_name)
    {
        if (index_of(state_var_names, state_var_name) == -1) {
            state_var_names.push_back(state_var_name);

            return;
        }
        throw_or_abort("name already exists");
    }
};

} // namespace aztec3::circuits::apps

// Importing in this way (rather than explicit instantiation of a template class at the bottom of a .cpp file) preserves
// the following:
// - We retain implicit instantiation of templates.
// - We don't implement method definitions in this file, to avoid a circular dependency with
// function_execution_context.hpp.
#include "contract.tpp"