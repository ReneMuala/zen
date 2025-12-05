//
// Created by dte on 12/3/2025.
//

#include "function.hpp"

#include "vm/vm.hpp"

namespace zen::builder
{
    std::shared_ptr<type> function::_bool()
    {
        static std::shared_ptr<type> t = type::create("bool", 1);
        return t;
    }

    std::shared_ptr<type> function::_byte()
    {
        static auto t = type::create("byte", 1);
        return t;
    }

    std::shared_ptr<type> function::_short()
    {
        static auto t = type::create("short", 2);
        return t;
    }

    std::shared_ptr<type> function::_int()
    {
        static auto t = type::create("int", 4);
        return t;
    }

    std::shared_ptr<type> function::_long()
    {
        static auto t = type::create("long", 8);
        return t;
    }

    std::shared_ptr<type> function::_float()
    {
        static auto t = type::create("float", 4);
        return t;
    }

    std::shared_ptr<type> function::_double()
    {
        static auto t = type::create("double", 8);
        return t;
    }

    std::shared_ptr<type> function::_string()
    {
        static auto t = type::create("string", 8);
        t->add_field("[len]", _long(), 0);
        t->add_field("[data]", _long(), 0);
        return t;
    }

    std::shared_ptr<block> function::get_scope()
    {
        if (!scope)
        {
            throw exceptions::semantic_error("scope not found, function was already compiled", offset);
        }
        auto it = scope;
        while (it->nested_scope)
        {
            it = it->nested_scope;
        }
        return it;
    }

     std::shared_ptr<zen::builder::function> function::create(const bool& logging)
    {
        auto it = std::make_shared<function>();
        it->logging = logging;
        it->signature = std::make_shared<zen::builder::signature>();
        it->scope = builder::block::create(scope::in_function);
        return it;
    }

     std::shared_ptr<value> function::set_parameter(const std::shared_ptr<zen::builder::type>& t,
                                                    const std::string& name)
    {
        signature->parameters.push_back(t);
        auto sym = set_local(t, name, true);
        sym->no_destructor = true;
        return sym;
    }

     std::shared_ptr<value> function::set_return(const std::shared_ptr<zen::builder::type>& t)
    {
        if (scope->get_stack_usage())
        {
            throw exceptions::semantic_error("return type cannot be set after params", offset);
        }
        signature->type = t;
        auto v = std::make_shared<value>(
                    t, scope->get_stack_usage() - /* jump callee IP */static_cast<types::stack::i64>(sizeof(types::stack::i64)));
        scope->use_stack(t->get_size());
        return v;
    }

    std::shared_ptr<value> function::set_local(const std::shared_ptr<zen::builder::type>& t, const std::string& name, const bool param)
    {
        const auto & scp = get_scope();
        const types::stack::i64 address = scp->get_stack_usage() - (param ? /* jump callee IP */static_cast<types::stack::i64>(sizeof(types::stack::i64)) : 0);
        scp->use_stack(t->get_size());
        if (not param)
        gen<most>(t->get_size());
        const auto sym = std::make_shared<value>(name, t, address);
        scope->locals[name].push_back(sym);
        return sym;
    }

    inline void assert_same_type(const char * verb, const std::shared_ptr<value>& _1, const std::shared_ptr<value>& _2, const i64& offset)
    {
        if (not _1->has_same_type_as(*_2))
        {
            throw exceptions::semantic_error(fmt::format("cannot {} {} to {}", verb, _1->type->name, _2->type->name), offset);
        }
    }

    void function::add(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
        const std::shared_ptr<value>& rhs)
    {
        assert_same_type("add", lhs, rhs, offset);
        assert_same_type("assign", lhs, r, offset);
        if (lhs->is(_byte()))
            gen<add_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<add_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<add_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<add_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<add_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<add_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("usupported type", offset);
    }
}
