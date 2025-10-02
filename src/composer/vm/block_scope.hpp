//
// Created by dte on 9/22/2025.
//

#pragma once
#include "scope.hpp"
#include "composer/composer.hpp"

namespace zen::composer::vm
{
    struct block_scope : public scope
    {
        std::unordered_map<std::string, std::deque<std::shared_ptr<value>>> locals = {};
        i64 stack_usage = {};
        enum return_status
        {
            no_return = 0,
            branched_return = 1,
            concise_return = 1 << 3,
        } return_status = {no_return};
        std::shared_ptr<value> get_local(const std::string& name)
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
        void set_local(const std::string& name, const std::shared_ptr<const zen::composer::type>& t)
        {
            if (nested_scope)
            {
                nested_scope->set_local(name, t);
            } else
            {
                locals[name].push_back(std::make_shared<value>(name, t, stack_usage));
                stack_usage += t->get_size();
            }
        }

        block_scope* nested_scope = {};

        virtual void __dncd__push(block_scope* scope)
        {
            if (nested_scope)
                nested_scope->__dncd__push(scope);
            else
            {
                nested_scope = scope;
            }
        }
        decltype(locals)& ___dncd__deepest_locals()
        {
            if (nested_scope)
            {
                if (nested_scope->nested_scope)
                    return nested_scope->___dncd__deepest_locals();
                // return nested_scope->___dncd__deepest_locals();
            }
            return locals;
        }
        /// do not call directly
        i64 __dncd__pop(enum return_status & root_status)
        {
            if (nested_scope)
            {
                i64 nested_scope_usage = nested_scope->__dncd__pop(root_status);
                if (nested_scope_usage > 0)
                {
                    delete nested_scope;
                    nested_scope = nullptr;
                    nested_scope_usage *= -1;
                }
                return nested_scope_usage;
            }
            if (type == in_if and return_status == concise_return)
                root_status = branched_return;
            else if (type == in_else_if and return_status == concise_return)
                root_status = branched_return;
            else if (type == in_else and return_status == concise_return and (root_status == branched_return or root_status == concise_return))
                root_status = concise_return;
            else
                root_status = no_return;
            return stack_usage;
        }

        enum return_status get_return_status() const
        {
            return nested_scope ? nested_scope->get_return_status() : return_status;
        }

        void set_return_status(enum return_status status)
        {
            if (nested_scope)
                nested_scope->set_return_status(status);
            else
                return_status = status;
        }

        i64 depth() const
        {
            if (nested_scope)
            {
                return 1 + nested_scope->depth();
            }
            return 1;
        }

        template<typename T>
        T * current(const enum type type)
        {
            if (not nested_scope) return nullptr;
            if (nested_scope->nested_scope)
                return nested_scope->current<T>(type);
            return nested_scope->type == type ? static_cast<T*>(nested_scope) : nullptr;
        }

        static block_scope* __unsafely_make(const enum type type);

        [[nodiscard]] bool is(const enum type &t) const
        {
            if (nested_scope) return nested_scope->is(t);
            return (type & t)  == t;
        }
    };
}

