//
// Created by dte on 9/25/2025.
//
#include "block.hpp"

namespace zen::builder
{
    std::shared_ptr<block> block::create(const enum type type)
    {
        auto it = std::make_unique<block>();
        it->type = type;
        return std::move(it);
    }

    std::shared_ptr<value> block::get_local(const std::string& name)
    {
        std::shared_ptr<value> target;
        if (nested_scope)
            target = nested_scope->get_local(name);
        if (not target)
        {
            if (const auto it = locals.find(name); it != locals.end())
                target = it->second.back();
        }
        return target;
    }

    void block::use_stack(const types::stack::i64 size)
    {
        if (nested_scope)
        {
            nested_scope->use_stack(size);
        }
        else
        {
            stack_usage += size;
        }
    }

    types::stack::i64 block::get_stack_usage() const
    {
        if (nested_scope)
        {
            return stack_usage + nested_scope->get_stack_usage();
        }
        else
        {
            return stack_usage;
        }
    }

    types::stack::i64 block::get_deep_stack_usage() const
    {
        if (nested_scope)
        {
            return nested_scope->get_deep_stack_usage();
        }
        else
        {
            return stack_usage;
        }
    }

    void block::set_local(const std::string& name, const std::shared_ptr<builder::type>& t,
        const types::stack::i64& stack_usage)
    {
        if (nested_scope)
        {
            nested_scope->set_local(name, t, stack_usage);
        }
        else
        {
            locals[name].push_back(std::make_shared<value>(name, t, stack_usage));
            this->stack_usage += t->get_size();
        }
    }

    void block::__dncd__push(std::shared_ptr<block> scope)
    {
        if (nested_scope)
            nested_scope->__dncd__push(std::move(scope));
        else
        {
            nested_scope = std::move(scope);
        }
    }

    decltype(block::locals)& block::___dncd__deepest_locals()
    {
        if (nested_scope)
            return nested_scope->___dncd__deepest_locals();
        return locals;
    }

    types::stack::i64 block::__dncd__pop(enum return_status& root_status)
    {
        if (did_pop)
        {
            fmt::println("[scope: double-pop detected]");
        }
        _pop_state state = searching;
        return __dncd__pop(root_status, state);
    }

    types::stack::i64 block::__dncd__pop(enum return_status& root_status, _pop_state& state)
    {
        if (nested_scope)
        {
            const types::stack::i64 nested_scope_usage = nested_scope->__dncd__pop(return_status, state);
            if (state == found)
            {
                nested_scope = nullptr;
                state = popped;
            }
            return nested_scope_usage;
        }
        did_pop = true;
        state = found;
        update_return_status(root_status);
        return stack_usage;
    }

    void block::update_return_status(enum return_status& root_status) const
    {
        if (type == in_if and return_status == concise_return)
        {
            root_status = branched_return;
        } else if (type == in_else_if and return_status == concise_return)
        {
            root_status = branched_return;
        } else if (type == in_else and return_status == concise_return)
        {
            root_status = concise_return;
        } else
        {
            root_status = return_status;
        }
    }

    types::stack::i64 block::__dncd__peek(enum return_status& root_status)
    {
        if (nested_scope)
        {
            types::stack::i64 nested_scope_usage = nested_scope->__dncd__peek(root_status);
            if (nested_scope_usage > 0)
            {
                // delete nested_scope;
                // nested_scope = nullptr;
                nested_scope_usage *= -1;
            }
            return nested_scope_usage;
        }
        update_return_status(root_status);
        return stack_usage;
    }

    enum block::return_status block::get_return_status() const
    {
        return nested_scope ? nested_scope->get_return_status() : return_status;
    }

    void block::set_return_status(enum return_status status)
    {
        if (nested_scope)
            nested_scope->set_return_status(status);
        else
            return_status = status;
    }

    void block::print_stack_tree() const
    {
        fmt::print("> {} ", stack_usage);
        if (nested_scope)
        {
            return nested_scope->print_stack_tree();
        }
        fmt::print("\n");
    }

    types::stack::i64 block::depth() const
    {
        if (nested_scope)
        {
            return 1 + nested_scope->depth();
        }
        return 1;
    }

    bool block::is(const enum type& t) const
    {
        if (nested_scope) return nested_scope->is(t);
        return (type & t) == t;
    }

    bool block::in_loop() const
    {
        const bool condition = (type & in_for_body) == in_for_body or (type & in_while_body) == in_while_body;
        if (nested_scope) return condition or nested_scope->in_loop();
        return condition;
    }
}