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
        std::map<std::string, symbol> locals = {};
        i64 stack_usage = {};
        enum return_status
        {
            no_return = 0,
            branched_return = 1,
            concise_return = 1 << 3,
        } return_status = {no_return};
        std::optional<symbol> get_local(const std::string& name)
        {
            std::optional<symbol> target;
            if (nested_scope)
                target = nested_scope->get_local(name);
            if (not target)
            {
                if (const auto it = locals.find(name); it != locals.end())
                    target = it->second;
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
                locals.emplace(name, symbol(name, t, stack_usage));
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

        /// do not call directly
        i64 __dncd__pop()
        {
            i64 nested_scope_usage = 0;
            if (nested_scope)
            {
                if (nested_scope->nested_scope)
                    return nested_scope->__dncd__pop();
                if (nested_scope->type == in_if and nested_scope->return_status == concise_return)
                    return_status = branched_return;
                else if (nested_scope->type == in_else_if and nested_scope->return_status == concise_return)
                    return_status = branched_return;
                else if (nested_scope->type == in_else and nested_scope->return_status == concise_return and (return_status == branched_return or return_status == concise_return))
                    return_status = concise_return;
                else
                    return_status = no_return;
                nested_scope_usage = nested_scope->stack_usage;
                delete nested_scope;
                nested_scope = nullptr;
            }
            return nested_scope_usage;
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
            // fmt::println("bs {} & {} = {}", int(type), int(t), int(type & t));
            return (type & t)  == t;
        }
    };
}

