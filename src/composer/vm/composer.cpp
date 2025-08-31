//
// Created by dte on 8/31/2025.
//

#include "composer.hpp"

#include <iostream>

namespace zen
{
    composer::vm::composer::composer()
    {
        types.insert({
            "unit", type{
                .name = "unit",
                .size = 0
            }
        });

        types.insert({
            "int", type{
                .name = "int",
                .size = 4
            }
        });


        types.insert({
            "long", type{
                .name = "long",
                .size = 8
            }
        });

        types.insert({
            "float", type{
                .name = "float",
                .size = 4
            }
        });
        types.insert({
            "double", type{
                .name = "double",
                .size = 8
            }
        });
        types.insert({
            "bool", type{
                .name = "bool",
                .size = 1
            }
        });
    }

    composer::vm::composer::~composer()
    {
    }

    void composer::vm::composer::begin_function(std::string name)
    {
        functions.push_back(
            std::make_tuple<std::string, signature, size_t>(std::move(name),
                                                            signature{
                                                                .type = std::ref(get_type("unit"))
                                                            }, code.size()));
    }

    void composer::vm::composer::set_function_parameter(std::string name, const type& type)
    {
        std::get<signature>(functions.back()).parameters.emplace_back(type);
        set_function_local(name, type);
    }

    void composer::vm::composer::set_function_return(const type& type)
    {
        if (scope.local_most_size)
        {
            throw std::logic_error("Function returned more than once or return is being set after parameters");
        }
        if (type.size != 0)
        {
            scope.return_value.emplace(value(type, scope.local_most_size));
            code.push_back(zen::most);
            code.push_back(-type.size);
        }
        std::get<signature>(functions.back()).type = type;
    }

    void composer::vm::composer::set_function_return_name(std::string name)
    {
        scope.return_name = name;
    }

    void composer::vm::composer::end_function()
    {
        if (scope.return_name)
        {
            set_function_return_value(get_function_local(scope.return_name.value()));
        }
        if (scope.local_most_size)
        {
            code.push_back(zen::most);
            code.push_back(scope.local_most_size);
            scope.local_most_size = 0;
        }
        code.push_back(zen::hlt);
        scope.clear();
    }

    const composer::type& composer::vm::composer::get_type(const std::string& name)
    {
        if (types.contains(name))
        {
            return types.at(name);
        }
        throw std::out_of_range("No such type: " + name);
    }

    void composer::vm::composer::bake()
    {
        for (const auto & function : functions)
        {
            std::cout << std::get<0>(function) << std::endl;
            for (int i = std::get<2>(function) ; i < code.size() ; i++)
            {
                const auto & _code = code[i];
                std::cout << _code << ' ';
                if (_code == zen::hlt) break;
            }
            std::cout << std::endl;
        }
    }

    void composer::vm::composer::set_function_return_value(const value& value)
    {
        operation_assign(scope.return_value.value(),value);
    }

    void composer::vm::composer::push_local_temp(const type& type)
    {
        const i64 address = scope.local_most_size;
        code.push_back(most);
        code.push_back(-type.size);
        scope.local_most_size += type.size;
        stack.push({value(type, address), true});
    }

    void composer::vm::composer::pop()
    {
        if (const auto & top = stack.top(); std::get<1>(top))
        {
            code.push_back(most);
            code.push_back(std::get<0>(top).type.get().size);
        }
        stack.pop();
    }

    void composer::vm::composer::operation_arith_plus(const value& destination, const value& first, const value& second)
    {
        if (first.type.get() == second.type.get())
        {
            if (first.type.get().name == "long")
            {
                code.push_back(add_i64);
            } else if (first.type.get().name == "double")
            {
                code.push_back(add_f64);
            } else
            {
                throw std::logic_error("Unsupported operation for type: " + first.type.get().name);
            }
        }
        code.push_back(destination.address);
        code.push_back(first.address);
        code.push_back(second.address);
    }

    void composer::vm::composer::operation_arith_minus(const value& destination, const value& first,
        const value& second)
    {
        if (first.type.get() == second.type.get())
        {
            if (first.type.get().name == "long")
            {
                code.push_back(sub_i64);
            } else if (first.type.get().name == "double")
            {
                code.push_back(sub_f64);
            } else
            {
                throw std::logic_error("Unsupported operation for type: " + first.type.get().name);
            }
        }
        code.push_back(destination.address);
        code.push_back(first.address);
        code.push_back(second.address);
    }

    void composer::vm::composer::operation_arith_multi(const value& destination, const value& first,
        const value& second)
    {
        if (first.type.get() == second.type.get())
        {
            if (first.type.get().name == "long")
            {
                code.push_back(mul_i64);
            } else if (first.type.get().name == "double")
            {
                code.push_back(mul_f64);
            } else
            {
                throw std::logic_error("Unsupported operation for type: " + first.type.get().name);
            }
        }
        code.push_back(destination.address);
        code.push_back(first.address);
        code.push_back(second.address);
    }

    void composer::vm::composer::operation_arith_divide(const value& destination, const value& first,
        const value& second)
    {
        if (first.type.get() == second.type.get())
        {
            if (first.type.get().name == "long")
            {
                code.push_back(div_i64);
            } else if (first.type.get().name == "double")
            {
                code.push_back(div_f64);
            } else
            {
                throw std::logic_error("Unsupported operation for type: " + first.type.get().name);
            }
        }
        code.push_back(destination.address);
        code.push_back(first.address);
        code.push_back(second.address);
    }

    void composer::vm::composer::operation_arith_mod(const value& destination, const value& first, const value& second)
    {
        if (first.type.get() == second.type.get())
        {
            if (first.type.get().name == "long")
            {
                code.push_back(mod_i64);
            } else
            {
                throw std::logic_error("Unsupported operation for type: " + first.type.get().name);
            }
        }
        code.push_back(destination.address);
        code.push_back(first.address);
        code.push_back(second.address);
    }

    const composer::symbol& composer::vm::composer::get_function_local(const std::string& name)
    {
        if (scope.locals.contains(name))
        {
            return scope.locals.at(name);
        }
            throw std::out_of_range("No such local symbol: " + name);
    }

    void composer::vm::composer::operation_assign(const value& destination, const value& source)
    {
        if (destination.type.get() == source.type.get())
        {
            switch (destination.type.get().size){
            case 8:
                destination.type.get().name == "long" ? code.push_back(i64_to_i64) : code.push_back(f64_to_f64);
                break;
            default:
                throw std::logic_error("Operation not supported for type: " + destination.type.get().name);
            };
            code.push_back(destination.address - scope.local_most_size);
            code.push_back(source.address - scope.local_most_size);
        }
    }

    void composer::vm::composer::set_function_local(std::string name, const type& type)
    {
        code.push_back(zen::most);
        code.push_back(-type.size);
        const i64 address = scope.local_most_size;
        scope.local_most_size += type.size;
        scope.locals.emplace(name, symbol(name, type, address));
    }
} // zen
