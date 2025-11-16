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
        bool did_pop = false;
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

        void use_stack(const i64 size)
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

        i64 get_stack_usage() const
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

        i64 get_deep_stack_usage() const
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

        void set_local(const std::string& name, const std::shared_ptr<zen::composer::type>& t,
                       const i64& stack_usage)
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
                return nested_scope->___dncd__deepest_locals();
            return locals;
        }
        enum _pop_state
        {
            searching,
            found,
            popped,
        };
        /// do not call directly
        i64 __dncd__pop(enum return_status& root_status)
        {
            if (did_pop)
            {
                fmt::println("[scope: double-pop detected]");
            }
            _pop_state state = searching;
            return __dncd__pop(root_status, state);
        }

        // improve this
        i64 __dncd__pop(enum return_status& root_status, _pop_state& state)
        {
            if (nested_scope)
            {
                const i64 nested_scope_usage = nested_scope->__dncd__pop(return_status, state);
                if (state == found)
                {
                    delete nested_scope;
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


        void update_return_status(enum return_status& root_status) const
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

        /// do not call directly
        i64 __dncd__peek(enum return_status& root_status)
        {
            if (nested_scope)
            {
                i64 nested_scope_usage = nested_scope->__dncd__peek(root_status);
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

        void print_stack_tree() const
        {
            fmt::print("> {} ", stack_usage);
            if (nested_scope)
            {
                return nested_scope->print_stack_tree();
            }
            fmt::print("\n");
        }

        i64 depth() const
        {
            if (nested_scope)
            {
                return 1 + nested_scope->depth();
            }
            return 1;
        }

        static block_scope* __unsafely_make(const enum type type);

        [[nodiscard]] bool is(const enum type& t) const
        {
            if (nested_scope) return nested_scope->is(t);
            return (type & t) == t;
        }

        bool in_loop() const
        {
            const bool condition = (type & in_for_body) == in_for_body or (type & in_while_body) == in_while_body;
            if (nested_scope) return condition or nested_scope->in_loop();
            return condition;
        }
    };
}
