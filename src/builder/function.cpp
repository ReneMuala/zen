//
// Created by dte on 12/3/2025.
//

#include "function.hpp"

#include <expected>

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

    std::shared_ptr<type> function::_unit()
    {
        static auto t = type::create("unit", 0);
        return t;
    }

    std::shared_ptr<type> function::_string()
    {
        static auto t = type::create("string", 8);
        static bool did_init = false;
        if (!did_init)
        {
            t->kind = type::kind::heap;
            t->add_field("string::len", _long(), /* offset */ 0);
            t->add_field("string::data", _long(), /* offset */ 0);
            did_init = true;
        }
        return t;
    }

    std::shared_ptr<block> function::get_scope(const bool root) const
    {
        if (!scope)
        {
            throw exceptions::semantic_error("scope not found, function was already compiled", offset);
        }
        auto it = scope;
        if (root)
            return it;
        while (it->nested_scope)
        {
            it = it->nested_scope;
        }
        return it;
    }

    std::shared_ptr<zen::builder::function> function::create(utils::constant_pool& pool, const i64& offset,
                                                             const bool& logging, const std::string& name)
    {
        auto it = std::make_shared<function>(pool, offset);
        it->logging = logging;
        it->name = name;
        it->signature = std::make_shared<zen::builder::signature>();
        it->signature->type = _unit();
        it->scope = builder::block::create(scope::in_function);
        return it;
    }

    std::shared_ptr<function> function::create(const std::string& name,
                                               const std::vector<std::shared_ptr<builder::type>>& params,
                                               const std::shared_ptr<builder::type>& type) const
    {
        auto fn = create(pool, offset, false, name);
        fn->signature->parameters = params;
        if (type)
        {
            fn->signature->type = type;
        }
        return fn;
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
        std::shared_ptr<block> scp = get_scope();
        if (get_stack_usage())
        {
            throw exceptions::semantic_error("return type cannot be set after params", offset);
        }
        signature->type = t;
        ret = std::make_shared<value>(
            t, 0 - /* jump callee IP */static_cast<types::stack::i64>(sizeof(
                types::stack::i64)));
        ret->name = "@ret";
        ret->no_destructor = true;
        scp->use_stack(t->get_size());
        return ret;
    }

    std::shared_ptr<value> function::set_local(const std::shared_ptr<zen::builder::type>& t, const std::string& name,
                                               const bool param)
    {
        const auto& scp = get_scope();
        const types::stack::i64 address = get_stack_usage() - (param
                                                                   ? /* jump callee IP */static_cast<
                                                                       types::stack::i64>(sizeof(
                                                                       types::stack::i64))
                                                                   : 0);
        scp->use_stack(t->get_size());
        const auto sym = std::make_shared<value>(name, t, address);
        scp->locals[name].push_back(sym);
        if (not param)
        {
            gen<zen::most>(-t->get_size(), fmt::format("{}:{}", name, -t->get_size()));
            if (t->kind == type::kind::heap)
            {
                const auto allocator = create(fmt::format("{}::allocate", t->name), {}, t);
                allocator->signature->is_allocator = true;
                if (const auto result = call(allocator, {}); not result.
                    has_value())
                {
                    throw exceptions::semantic_error(result.error(), offset);
                }
            }
        } else
        {
            sym->no_destructor = true;
        }
        return sym;
    }

    std::shared_ptr<value> function::set_alias(const std::shared_ptr<zen::builder::value>& val, const std::string& name) const
    {
        const auto& scp = get_scope();
        scp->locals[name].push_back(val);
        return val;
    }

    inline void assert_same_type(const char* verb, const std::shared_ptr<value>& _1, const std::shared_ptr<value>& _2,
                                 const i64& offset)
    {
        if (not _1->has_same_type_as(*_2))
        {
            throw exceptions::semantic_error(fmt::format("cannot {} {}[{}] to {}[{}]", verb, _2->type->name, _2->type->get_full_size(), _1->type->name, _1->type->get_full_size()),
                                             offset);
        }
    }

    inline void assert_type(const char* verb, const std::shared_ptr<value>& _1, const std::shared_ptr<type>& _t,
                            const i64& offset)
    {
        if (_1->type != _t)
        {
            throw exceptions::semantic_error(fmt::format("cannot {} {} to {}", verb, _t->name, _1->type->name), offset);
        }
    }

    void function::add(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("add", lhs, rhs, offset);
        assert_same_type("assign", r, lhs, offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<zen::add_i8>(temp_srh ? temp_srh : temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<zen::add_i16>(temp_srh ? temp_srh : temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<zen::add_i32>(temp_srh ? temp_srh : temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<zen::add_i64>(temp_srh ? temp_srh : temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<zen::add_f32>(temp_srh ? temp_srh : temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<zen::add_f64>(temp_srh ? temp_srh : temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator+", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot add type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::sub(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("subtract", lhs, rhs, offset);
        assert_same_type("assign", r, lhs, offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<zen::sub_i8>(temp_srh ? temp_srh : temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<zen::sub_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<zen::sub_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<zen::sub_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<zen::sub_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<zen::sub_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator-", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot subtract type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::mul(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("multiply", lhs, rhs, offset);
        assert_same_type("assign", r, lhs, offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<zen::mul_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<zen::mul_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<zen::mul_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<zen::mul_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<zen::mul_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<zen::mul_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator*", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot multiply type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::div(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("divide", lhs, rhs, offset);
        assert_same_type("assign", r, lhs, offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<zen::div_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<zen::div_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<zen::div_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<zen::div_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<zen::div_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<zen::div_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator/", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot divide type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::mod(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("mod", lhs, rhs, offset);
        assert_same_type("assign", r, lhs, offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<zen::mod_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<zen::mod_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<zen::mod_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<zen::mod_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator%", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot compute module for type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                         const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()) or lhs->is(_bool()))
            gen<eq_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<eq_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<eq_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<eq_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<eq_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<eq_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator==", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot compare type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::not_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                             const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()) or lhs->is(_bool()))
            gen<neq_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<neq_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<neq_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<neq_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<neq_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<neq_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator!=", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot compare type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::lower(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                         const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<zen::lt_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<zen::lt_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<zen::lt_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<zen::lt_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<zen::lt_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<zen::lt_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator<", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot compare type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::lower_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                               const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<lte_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<lte_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<lte_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<lte_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<lte_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<lte_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator<=", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot compare type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::greater(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                           const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<zen::gt_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<zen::gt_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<zen::gt_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<zen::gt_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<zen::gt_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<zen::gt_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator>", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot compare type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::greater_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                                 const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        if (lhs->is(_byte()))
            gen<gte_i8>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_short()))
            gen<gte_i16>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_int()))
            gen<gte_i32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_long()))
            gen<gte_i64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_float()))
            gen<gte_f32>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->is(_double()))
            gen<gte_f64>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        else if (lhs->type->kind == builder::type::heap)
        {
            if (const auto result = call(create("operator>=", {lhs->type, rhs->type}, r->type), {lhs, rhs}); result.
                has_value())
            {
                move(r, result.value());
            }
            else
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
        else
            throw exceptions::semantic_error(fmt::format("cannot compare type {}", lhs->type->name), offset);
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::move(const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs)
    {
        assert_same_type("assign", lhs, rhs, offset);
        if (lhs->type->kind == builder::type::stack)
        {
            if (not(lhs->is_reference or rhs->is_reference))
            {
                if (lhs->is(_byte()) or lhs->is(_bool()))
                    gen<zen::i8_to_i8>(lhs, rhs);
                else if (lhs->is(_short()))
                    gen<zen::i16_to_i16>(lhs, rhs);
                else if (lhs->is(_int()))
                    gen<zen::i32_to_i32>(lhs, rhs);
                else if (lhs->is(_long()))
                    gen<zen::i64_to_i64>(lhs, rhs);
                else if (lhs->is(_float()))
                    gen<zen::f32_to_f32>(lhs, rhs);
                else if (lhs->is(_double()))
                    gen<zen::f64_to_f64>(lhs, rhs);
                else
                    throw exceptions::semantic_error(fmt::format("cannot copy type {}", lhs->type->name), offset);
            }
            else if (lhs->is_reference and not rhs->is_reference)
            {
                const auto ref = set_local(_long(),"temp::rhs_ref");
                gen<zen::refer>(ref, rhs);
                ref->type = rhs->type;
                ref->is_reference = true;
                ref->no_destructor = true;
                move(lhs, ref);
            }
            else if (not lhs->is_reference and rhs->is_reference)
            {
                const auto ref = set_local(_long(),"temp::lhs_ref");
                gen<zen::refer>(ref, lhs);
                ref->type = lhs->type;
                ref->is_reference = true;
                ref->no_destructor = true;
                move(ref, rhs);
            }
            else
            {
                gen<zen::copy>(lhs, rhs, lhs->type->get_full_size(), fmt::format("@size:{}", lhs->type->get_full_size()));
            }
        }
        else {
            if (const auto result = call(create("operator=", {lhs->type, rhs->type}, nullptr), {lhs, rhs}); not result.
                has_value())
            {
                throw exceptions::semantic_error(result.error(), offset);
            }
        }
    }

    void function::not_(const std::shared_ptr<value>& r, const std::shared_ptr<value>& val)
    {
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        gen<zen::boolean_not>(temp_srh ? temp_srh : r, resolve(val));
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::and_(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
        const std::shared_ptr<value>& rhs)
    {
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        gen<zen::boolean_and>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }


    void function::or_(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
        const std::shared_ptr<value>& rhs)
    {
        const auto temp_srh = (r->is_reference and r->type->kind == type::kind::stack) ? set_local(r->type, "temp::srh") : nullptr;
        gen<zen::boolean_or>(temp_srh ? temp_srh : r, resolve(lhs), resolve(rhs));
        if (temp_srh)
        {
            move(r, temp_srh);
        }
    }

    void function::return_value(const std::shared_ptr<value>& r)
    {
        std::shared_ptr<block> scp = get_scope();
        if (scp->get_return_status() == block::concise_return)
            throw exceptions::semantic_error("cannot return values more than once", offset);
        if (scp->get_return_status() == block::branched_return and not scp->is(scope::in_else))
            throw exceptions::semantic_error("conflicting returns", offset,
                                             "both the 'if' block and the code after it return values\n\tuse 'else' to make return paths mutually exclusive");
        if (scp->in_loop())
        {
            throw exceptions::semantic_error("cannot return inside loop", offset,
                                             "return value will be overwritten on each iteration\n\trestructure to return after the loop completes");
        }
        scp->set_return_status(block::concise_return);
        if (ret)
        {
            move(ret, r);
        }
        else
            throw exceptions::semantic_error("cannot return without specifying return type first", offset);
    }

    void function::return_implicitly() const
    {
        get_scope(true)->set_return_status(block::concise_return);
    }

    void function::go(const std::shared_ptr<builder::label>& l)
    {
        gen<zen::go>(0, fmt::format("%{}", l->id));
        l->use(code);
    }

    void function::go_if_not(const std::shared_ptr<value>& c, const std::shared_ptr<builder::label>& l)
    {
        gen<zen::go_if_not>(resolve(c), 0, fmt::format("%{}", l->id));
        l->use(code);
    }

    std::expected<std::shared_ptr<value>, std::string> function::call(const std::shared_ptr<builder::function>& fb,
                                                                      const std::vector<std::shared_ptr<value>>& args)
    {
        if (not fb->signature->check_args(args))
            return std::unexpected(fmt::format("no matching args to call {}, expected {} got {}", fb->name,
                                               std::string(*fb->signature), builder::signature::describe_args(args)));
        const auto fb_hash = fb->hash();
        std::vector<std::shared_ptr<value>> final_args;
        for (auto& val : args)
        {
            std::shared_ptr<builder::value> arg = val;
            if (arg->type->kind == type::kind::heap and arg->kind == value::kind::constant)
            {
                auto cha = set_local(arg->type, fmt::format("cha::{}", arg->name));
                auto ptr = set_local(_long(), "cha::ptr");
                gen<zen::refer>(ptr, arg);
                ptr->type = arg->type;
                ptr->is_reference = true;
                ptr->no_destructor = true;
                if (const auto result = call(create("operator=", {cha->type, cha->type}, nullptr), {cha, ptr}); not
                    result.has_value())
                {
                    throw exceptions::semantic_error(result.error(), offset);
                }
                arg = cha;
            }
            else if (arg->is_reference)
                // deference fields only if they are stack allocated since nested objects are stored in a contiguous memory block
            {
                if (arg->type->kind == type::kind::stack)
                {
                    arg = dereference(arg);
                } else
                {
                    arg = advance(arg);
                }
            }
            final_args.push_back(arg);
        }
        std::shared_ptr<value> return_value;
        if (fb->signature->type != _unit() and not fb->signature->is_allocator)
        {
            return_value = set_local(fb->signature->type, fmt::format("@ret::{}", fb->name, offset));
        }
        const auto scp = get_scope();
        i64 call_cost = 0;
        for (auto& arg : final_args)
        {
            if (arg->is(_byte()))
                gen<zen::push_i8>(arg);
            else if (arg->is(_bool()))
                gen<zen::push_boolean>(arg);
            else if (arg->is(_short()))
                gen<zen::push_i16>(arg);
            else if (arg->is(_int()))
                gen<zen::push_i32>(arg);
            else if (arg->is(_long()))
                gen<zen::push_i64>(arg);
            else if (arg->is(_float()))
                gen<zen::push_f32>(arg);
            else if (arg->is(_double()))
                gen<zen::push_f64>(arg);
            else
                gen<zen::push_i64>(arg);
            scp->use_stack(arg->type->get_size());
            call_cost += arg->type->get_size();
        }
        gen<zen::call>(0, fmt::format("{}:{}", fb->name, fb_hash));
        // fb->glabel->use(shared_from_this());
        if (not dependencies.contains(fb_hash))
        {
            dependencies[fb_hash] = global_label::create(fb->get_canonical_name());
        }
        dependencies[fb_hash]->use(code);

        if (call_cost)
        {
            gen<zen::most>(call_cost, fmt::format("@cc:{}", call_cost));
        }
        scp->use_stack(-call_cost);
        return return_value;
    }

    i64 function::hash() const
    {
        static constexpr std::hash<std::string> hasher;
        return hasher(get_canonical_name());
    }

    std::string function::get_canonical_name() const
    {
        // because return type should not matter in most cases
        // return fmt::format("{}{}={}", name, std::string(*signature), signature->type ? signature->type->name : "unit");
        return fmt::format("{}{}", name, std::string(*signature));
    }

    std::shared_ptr<value> function::dereference(const std::shared_ptr<value>& r)
    {
        const auto dest = set_local(r->type, "deref::dest");
        const auto ptr = set_local(zen::builder::function::_long(), "deref::ptr");
        gen<zen::refer>(ptr, dest);
        ptr->is_reference = true;
        const auto size = r->type->get_size();
        gen<zen::copy>(ptr, r, size, fmt::format("@size:{}", size));
        get_scope()->use_stack(-ptr->type->get_size());
        const auto dereference_cost = ptr->type->get_size();
        gen<zen::most>(dereference_cost, fmt::format("@dc:{}", dereference_cost));
        return dest;
    }

    std::shared_ptr<value> function::advance(const std::shared_ptr<value>& r)
    {
        const auto adv = set_local(zen::builder::function::_long(), "adv::ptr");
        adv->type = adv->type;
        adv->no_destructor = true;
        gen<zen::add_i64>(adv, r, constant<i64>(adv->offset));
        return adv;
    }

    void function::branch(enum scope::type st,
                          const std::shared_ptr<value>& c,
                          const std::function<void(const std::shared_ptr<builder::function>&, const std::
                                                   shared_ptr<builder::label>&,
                                                   const std::shared_ptr<builder::label>&)>& callback,
                          const std::shared_ptr<builder::label>& pel, const std::shared_ptr<builder::label>& pen)
    {
        std::shared_ptr<block> scp = get_scope();
        switch (st)
        {
        case scope::in_if:
            {
                if (pel or pen)
                    throw std::logic_error("dont specify pel nor pen in if branches");
                const auto end_lab = label();
                const auto else_lab = label();
                if (not c)
                    throw std::logic_error("a condition is needed for if branches");
                assert_type("assign", c, _bool(), offset);
                go_if_not(c, else_lab);

                scp->nested_scope = block::create(st);
                callback(shared_from_this(), else_lab, end_lab);
                if (not else_lab->bound())
                {
                    pop();
                    bind(else_lab);
                }
                bind(end_lab);
            }
            break;
        case scope::in_else:
            {
                if (c)
                    throw std::logic_error("dont specify c in else branches");
                if (not(pen and pel))
                    throw std::logic_error("specify pel and pen in else branches");
                pop();
                // if(get_scope()->type == scope::in_between)
                //     peek();
                go(pen);
                bind(pel);
                get_scope(true)->__dncd__push(block::create(st));
                callback(shared_from_this(), nullptr, pen);
                pop();
            }
            break;
        case scope::in_between:
            {
                if (c)
                    throw std::logic_error("dont specify c in between branches");
                pop();
                // if(get_scope()->type == scope::in_between)
                //     peek();
                go(pen);
                bind(pel);
                const auto end_lab = label();
                get_scope(true)->__dncd__push(block::create(st));
                callback(shared_from_this(), pel, end_lab);
                bind(end_lab);
                pop();
                // go(pen);
            }
            break;
        case scope::in_else_if:
            {
                if (not(pen and pel))
                    throw std::logic_error("specify pel and pen for else if branches");
                // go(pen);
                // bind(pel);
                const auto else_lab = label();
                if (not c)
                    throw std::logic_error("a condition is needed for else if branches");
                assert_type("assign", c, _bool(), offset);
                go_if_not(c, else_lab);
                get_scope(true)->__dncd__push(block::create(st));
                callback(shared_from_this(), else_lab, pen);
                if (not else_lab->bound())
                {
                    pop();
                    bind(else_lab);
                }
            }
            break;
        default:
            throw exceptions::semantic_error("unexpected scope type for branch", offset);
        }
    }

    static std::shared_ptr<value> constant_of_type(const i64 val, const std::shared_ptr<type>& t)
    {
        return std::make_shared<value>(fmt::format("{}", val), t, 0);
    }

    void function::loop_for(const std::vector<std::shared_ptr<value>>& params,
                            const std::function<void(const std::shared_ptr<builder::function>&)>& body)

    {
        if (params.size() == 3)
        {
            std::vector<std::shared_ptr<value>> new_params = params;
            auto it = params.at(0);
            std::shared_ptr<value> step = constant(1, it->type);

            new_params.push_back(step);
            loop_for(new_params, body);
        }
        else if (params.size() == 4)
        {
            const auto cond = set_local(_bool(), "for::cond");
            const std::shared_ptr<value> &iterator = params[0],
                                         &first = params[1],
                                         &last = params[2],
                                         &step = params[3];
            const std::shared_ptr<builder::label> begin = label(),
                                                  end = label(),
                                                  condition = label();
            move(iterator, first);
            go(condition); // skip increment for first iteration
            {
                bind(begin);
                add(iterator, iterator, step);
                bind(condition);
                lower_equal(cond, iterator, last);
                go_if_not(cond, end);
                {
                    get_scope(true)->__dncd__push(block::create(scope::in_for_body));
                    body(shared_from_this());
                    pop();
                }
                go(begin);
                bind(end);
            }
        }
        else
            throw exceptions::semantic_error("unsupported parameter count for for loop", offset);
    }

    void function::loop_while(const std::vector<std::shared_ptr<value>>& params,
                              const std::function<void(const std::shared_ptr<builder::function>&)>& prologue,
                              const std::function<void(const std::shared_ptr<builder::function>&)>& body)
    {
        const auto scp = get_scope(true);
        scp->__dncd__push(block::create(scope::in_while_prologue));
        const std::shared_ptr<builder::label> begin = label(), end = label();
        bind(begin);
        prologue(shared_from_this());
        if (params.size() == 1)
        {
            const auto cond = params[0];
            go_if_not(cond, end);
            {
                scp->__dncd__push(block::create(scope::in_for_body));
                body(shared_from_this());
                pop();
            }
            peek();
            go(begin);
            bind(end);
            pop();
        }
        else
            throw exceptions::semantic_error("unsupported parameter count for while loop", offset);
    }

    void function::peek()
    {
        std::shared_ptr<block> scp = scope;
        if (scp)
        {
            for (const auto& local_set : scp->___dncd__deepest_locals())
            {
                for (const auto& local : local_set.second)
                {
                    if (local->type->kind == type::heap && !local->no_destructor)
                    {
                        if (const auto result = call(create(fmt::format("{}::deallocate", local->type->name), {local->type}, nullptr), {local}); not
                            result.
                            has_value())
                        {
                            throw exceptions::semantic_error(result.error(), offset);
                        }
                    }
                }
            }
            if (const i64 size = scp->__dncd__peek(scp->return_status); std::abs(size) > 0)
            {
                gen<zen::most>(std::abs(size));
            };
        }
    }

    void function::build()
    {
        if (get_scope(true)->get_return_status() != block::concise_return and signature->type != _unit())
            throw exceptions::semantic_error("missing return value", offset,
                                             fmt::format(
                                                 "ensure that all control paths of {} return a value of type {}",
                                                 get_canonical_name(), signature->type->name));
        pop(true);
        gen<zen::ret>();
        scope = nullptr;
    }

    i64 function::get_stack_usage() const
    {
        return get_scope(true)->get_stack_usage();
    }

    void function::pop(bool final)
    {
        std::shared_ptr<block> scp = scope;
        if (scp)
        {
            for (const auto& local_set : scp->___dncd__deepest_locals())
            {
                for (const auto& local : local_set.second)
                {
                    if (local->type->kind == type::heap && !local->no_destructor)
                    {
                        if (const auto result = call(create(fmt::format("{}::deallocate", local->type->name), {local->type}, nullptr), {local}); not
                            result.
                            has_value())
                        {
                            throw exceptions::semantic_error(result.error(), offset);
                        }
                    }
                }
            }
            i64 callee_cost = 0;
            if (final)
            {
                callee_cost = signature->type ? signature->type->get_size() : 0;
                for (const auto & param : signature->parameters)
                {
                    callee_cost += param->get_size();
                }
            }
            if (const i64 size = scp->__dncd__pop(scp->return_status); std::abs(size) > 0)
            {
                gen<zen::most>(std::abs(size) - callee_cost);
            }; // scope
        }
    }
}
