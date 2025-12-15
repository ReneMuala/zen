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

    std::shared_ptr<type> function::_string()
    {
        static auto t = type::create("string", 8);
        t->kind = type::kind::heap;
        t->add_field("string::len", _long(), /* offset */ 0);
        t->add_field("string::data", _long(), /* offset */ 0);
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
                                                             const bool& logging)
    {
        auto it = std::make_shared<function>(pool, offset);
        it->logging = logging;
        it->signature = std::make_shared<zen::builder::signature>();
        it->scope = builder::block::create(scope::in_function);
        it->glabel = std::make_shared<zen::builder::global_label>();
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
        std::shared_ptr<block> scp = get_scope();
        if (get_stack_usage())
        {
            throw exceptions::semantic_error("return type cannot be set after params", offset);
        }
        signature->type = t;
        ret = std::make_shared<value>(
            t, get_stack_usage() - /* jump callee IP */static_cast<types::stack::i64>(sizeof(
                types::stack::i64)));
        ret->name = "@ret";
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
        if (not param)
            gen<most>(-t->get_size());
        const auto sym = std::make_shared<value>(name, t, address);
        scp->locals[name].push_back(sym);
        return sym;
    }

    inline void assert_same_type(const char* verb, const std::shared_ptr<value>& _1, const std::shared_ptr<value>& _2,
                                 const i64& offset)
    {
        if (not _1->has_same_type_as(*_2))
        {
            throw exceptions::semantic_error(fmt::format("cannot {} {} to {}", verb, _2->type->name, _1->type->name),
                                             offset);
        }
    }

    inline void assert_type(const char* verb, const std::shared_ptr<value>& _1, const std::shared_ptr<type>& _t,
                            const i64& offset)
    {
        if (*_1->type != *_t)
        {
            throw exceptions::semantic_error(fmt::format("cannot {} {} to {}", verb, _t->name, _1->type->name), offset);
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
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::sub(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("subtract", lhs, rhs, offset);
        assert_same_type("assign", lhs, r, offset);
        if (lhs->is(_byte()))
            gen<sub_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<sub_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<sub_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<sub_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<sub_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<sub_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::mul(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("multiply", lhs, rhs, offset);
        assert_same_type("assign", lhs, r, offset);
        if (lhs->is(_byte()))
            gen<mul_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<mul_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<mul_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<mul_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<mul_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<mul_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::div(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("divide", lhs, rhs, offset);
        assert_same_type("assign", lhs, r, offset);
        if (lhs->is(_byte()))
            gen<div_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<div_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<div_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<div_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<div_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<div_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::mod(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs)
    {
        assert_same_type("mod", lhs, rhs, offset);
        assert_same_type("assign", lhs, r, offset);
        if (lhs->is(_byte()))
            gen<mod_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<mod_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<mod_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<mod_i64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                         const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        if (lhs->is(_byte()))
            gen<eq_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<eq_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<eq_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<eq_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<eq_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<eq_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::not_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                             const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        if (lhs->is(_byte()))
            gen<neq_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<neq_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<neq_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<neq_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<neq_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<neq_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::lower(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                         const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        if (lhs->is(_byte()))
            gen<lt_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<lt_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<lt_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<lt_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<lt_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<lt_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::lower_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                               const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        if (lhs->is(_byte()))
            gen<lte_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<lte_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<lte_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<lte_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<lte_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<lte_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::greater(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                           const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        if (lhs->is(_byte()))
            gen<gt_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<gt_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<gt_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<gt_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<gt_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<gt_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::greater_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                                 const std::shared_ptr<value>& rhs)
    {
        assert_same_type("compare", lhs, rhs, offset);
        assert_type("assign", r, _bool(), offset);
        if (lhs->is(_byte()))
            gen<gte_i8>(r, lhs, rhs);
        else if (lhs->is(_short()))
            gen<gte_i16>(r, lhs, rhs);
        else if (lhs->is(_int()))
            gen<gte_i32>(r, lhs, rhs);
        else if (lhs->is(_long()))
            gen<gte_i64>(r, lhs, rhs);
        else if (lhs->is(_float()))
            gen<gte_f32>(r, lhs, rhs);
        else if (lhs->is(_double()))
            gen<gte_f64>(r, lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
    }

    void function::move(const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs)
    {
        assert_same_type("assign", lhs, rhs, offset);
        if (lhs->is(_byte()))
            gen<i8_to_i8>(lhs, rhs);
        else if (lhs->is(_short()))
            gen<i16_to_i16>(lhs, rhs);
        else if (lhs->is(_int()))
            gen<i32_to_i32>(lhs, rhs);
        else if (lhs->is(_long()))
            gen<i64_to_i64>(lhs, rhs);
        else if (lhs->is(_float()))
            gen<f32_to_f32>(lhs, rhs);
        else if (lhs->is(_double()))
            gen<f64_to_f64>(lhs, rhs);
        else
            throw exceptions::semantic_error("unsupported type", offset);
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

    void function::go(const std::shared_ptr<builder::label>& l)
    {
        gen<zen::go>(0, fmt::format("%{}", l->id));
        l->use(code);
    }

    void function::go_if_not(const std::shared_ptr<value>& c, const std::shared_ptr<builder::label>& l)
    {
        gen<zen::go_if_not>(c, 0, fmt::format("%{}", l->id));
        l->use(code);
    }

    std::expected<std::shared_ptr<value>, std::string> function::call(const std::shared_ptr<builder::function>& fb,
                                                                      const std::vector<std::shared_ptr<value>>& args)
    {
        if (not fb->signature->check_args(args))
            return std::unexpected(fmt::format("no matching args to call {}, expected {} got {}", fb->name, std::string(*fb->signature), builder::signature::describe_args(args)));
        const auto fb_hash = fb->hash();
        std::shared_ptr<value> return_value;
        if (fb->signature->type)
        {
            return_value = set_local(fb->signature->type, fmt::format("{}/{}", fb_hash, offset));
        }
        i64 call_cost = 0;
        const auto scp = get_scope();
        for (auto & arg : args)
        {
            if (arg->is(_byte()))
                gen<push_i8>(arg);
            else if (arg->is(_short()))
                gen<push_i16>(arg);
            else if (arg->is(_int()))
                gen<push_i32>(arg);
            else if (arg->is(_long()))
                gen<push_i64>(arg);
            else if (arg->is(_float()))
                gen<push_f32>(arg);
            else if (arg->is(_double()))
                gen<push_f64>(arg);
            else
                throw exceptions::semantic_error("unsupported type", offset);
            scp->use_stack(arg->type->get_size());
            call_cost+=arg->type->get_size();
        }
        gen<zen::call>(0, fmt::format("{}", fb_hash));
        fb->glabel->use(shared_from_this());
        if (call_cost)
        {
            gen<most>(call_cost);
        }
        scp->use_stack(-call_cost);
        dependencies[fb->hash()] = fb->get_canonical_name();
        return return_value;
    }

    int function::hash() const
    {
        static constexpr std::hash<std::string> hasher;
        return hasher(get_canonical_name());
    }

    std::string function::get_canonical_name() const
    {
        return fmt::format("{}{}={}", name, std::string(*signature), signature->type ? signature->type->name : "unit");
    }

    std::shared_ptr<value> function::dereference(const std::shared_ptr<value>& r)
    {
        const auto dest = set_local(r->type, "deref::dest");
        const auto ptr = set_local(r->type, "deref::ptr");
        gen<zen::refer>(ptr, dest);
        ptr->is_reference = true;
        gen<zen::copy>(ptr, r, (i64)pool.get(r->type->get_size()).get());
        get_scope()->use_stack(-ptr->type->get_size());
        gen<most>(ptr->type->get_size());
        return dest;
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
            new_params.push_back(constant_of_type(1, params.at(0)->type));
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
                        try
                        {
                            // push(local);
                            // call("[zenDestructor]", 1);
                        }
                        catch (const std::exception& e)
                        {
                            throw exceptions::semantic_error(
                                fmt::format("missing destructor implementation for type {}", local->type->name),
                                offset);
                        }
                    }
                }
            }
            if (const i64 size = scp->__dncd__peek(scp->return_status); std::abs(size) > 0)
            {
                gen<most>(std::abs(size));
            };
        }
    }

    void function::build()
    {
        pop();
        scope = nullptr;
    }

    i64 function::get_stack_usage() const
    {
        return get_scope(true)->get_stack_usage();
    }

    void function::pop()
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
                        try
                        {
                            // push(local);
                            // call("[zenDestructor]", 1);
                        }
                        catch (const std::exception& e)
                        {
                            throw exceptions::semantic_error(
                                fmt::format("missing destructor implementation for type {}", local->type->name),
                                offset);
                        }
                    }
                }
            }
            if (const i64 size = scp->__dncd__pop(scp->return_status); std::abs(size) > 0)
            {
                gen<most>(std::abs(size));
            }; // scope
        }
    }
}
