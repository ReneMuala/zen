//
// Created by dte on 8/31/2025.
//

#include "composer.hpp"

#include <iostream>
#include <fmt/core.h>

namespace zen
{
    void composer::vm::composer::umost(const std::initializer_list<value>&& values)
    {
        i64 changes = 0;
        for (const value& value : values)
        {
            if (value.kind == temporary)
            {
                const auto& type = value.type.get();
                changes += type->size;
            }
        }
        if (changes)
        {
            code.push_back(most);
            code.push_back(changes);
            scope.local_most_size -= changes;
        }
    }

    composer::value composer::vm::composer::top()
    {
        return stack.top();
    }

    void composer::vm::composer::push(const value& v)
    {
        stack.push(v);
    }

    composer::vm::composer::composer(): types({
        {"unit", std::make_shared<type>("unit", 0)},
        {"int", std::make_shared<type>("int", 4)},
        {"long", std::make_shared<type>("long", 8)},
        {"float", std::make_shared<type>("float", 4)},
        {"double", std::make_shared<type>("double", 8)},
        {"bool", std::make_shared<type>("bool", 1)},
        {"byte", std::make_shared<type>("byte", 1)}
    }),
    functions({
            {"int", {signature{}, 0}},
            {"long", {signature{}, 0}},
            {"float", {signature{}, 0}},
            {"double", {signature{}, 0}},
            {"bool", {signature{}, 0}},
            {"byte", {signature{}, 0}}
    })
    {}

    void composer::vm::composer::begin(std::string name)
    {
        scope.function_name = name;
        functions.emplace(std::move(name), std::make_tuple<signature, size_t>(
                                                                               signature{
                                                                                   .type = types.at("unit")
                                                                               }, code.size()));
    }

    void composer::vm::composer::set_parameter(std::string name, const std::string& type)
    {
        std::get<signature>(functions.at(scope.function_name)).parameters.emplace_back(types.at(type));
        set_local(name, type);
    }

    void composer::vm::composer::set_return_type(const std::string& name)
    {
        if (scope.local_most_size)
        {
            throw std::logic_error("Function returned more than once or return is being set after parameters");
        }
        const auto type = types.at(name);
        if (type->size != 0)
        {
            scope.return_value.emplace(value(type, scope.local_most_size));
            code.push_back(zen::most);
            code.push_back(-type->size);
            scope.local_most_size += type->size;
        }
        std::get<signature>(functions.at(scope.function_name)).type = type;
    }

    void composer::vm::composer::return_value()
    {
        const value rhs = top();
        pop();
        push(scope.return_value.value());
        push(rhs);
        assign();
    }

    void composer::vm::composer::set_return_name(const std::string& name)
    {
        scope.return_name = name;
    }

    void composer::vm::composer::set_local(std::string name, const std::string& type)
    {
        const auto& t = get_type(type);
        // if (name != scope.return_name)
        // {
        i64 address = scope.local_most_size;
        code.push_back(zen::most);
        code.push_back(-t->size);
        scope.local_most_size += t->size;
        scope.locals.emplace(name, symbol(name, t, address));
        // } else
        // {
        //     scope.locals.emplace(name, symbol(name, t,scope.return_value->_address));
        // }
    }

    void composer::vm::composer::end()
    {
        if (scope.return_name)
        {
            push(scope.return_name.value());
            return_value();
        }
        if (auto const return_value = scope.return_value)
        {
            if (const auto most_delta = scope.local_most_size -return_value->type->size; most_delta > 0)
            {
                code.push_back(zen::most);
                code.push_back(most_delta);
            }
        }
        code.push_back(zen::ret);
        scope.clear();
    }

    std::shared_ptr<const composer::type>& composer::vm::composer::get_type(const std::string& name)
    {
        if (types.contains(name))
        {
            return types.at(name);
        }
        throw std::out_of_range(fmt::format(
            "[Error: No such type] You are trying to use the type {}, but it has not been declared yet", name));
    }

    void composer::vm::composer::bake()
    {
        code.push_back(zen::hlt);
        for (const auto& function : functions)
        {
            std::cout << function.first << std::endl;
            for (i64 i = std::get<1>(function.second); i < code.size(); i++)
            {
                const auto& _code = code[i];
                if (_code == most)
                {
                    fmt::print("most {} ", code[i + 1]);
                    i += 1;
                }
                else if (_code == add_i64 || _code == add_f64 || _code == add_i8)
                {
                    fmt::print("add {} {} {} ", code[i + 1], code[i + 2], code[i + 3]);
                    i += 3;
                }
                else if (_code == mul_i64 || _code == mul_f64 || _code == mul_i8)
                {
                    fmt::print("mul {} {} {} ", code[i + 1], code[i + 2], code[i + 3]);
                    i += 3;
                }
                else if (_code >= i8_to_i64 && _code <= f64_to_boolean)
                {
                    fmt::print("cp {} {} ", code[i + 1], code[i + 2]);
                    i += 2;
                }
                else if (_code == ret)
                    fmt::print("ret ");
                else
                    std::cout << _code << ' ';
                if (_code == zen::ret) break;
            }
            std::cout << std::endl;
        }
    }

    void composer::vm::composer::assign()
    {
        if (stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, stack.size()));
        const value lhs = top();
        pop();
        const value rhs = top();
        pop();
        if (lhs.kind == constant)
        {
            throw std::logic_error(fmt::format(
                "[Error: Invalid operation] Cannot compose operation " __FUNCTION__
                " to a constant, consider changing assign operands order"));
        }
        if (rhs.is("short") and rhs.has_same_type_as(lhs))
            code.push_back(zen::i16_to_i16);
        else if (rhs.is("int") and rhs.has_same_type_as(lhs))
            code.push_back(zen::i32_to_i32);
        else if (rhs.is("long") and rhs.has_same_type_as(lhs))
            code.push_back(zen::i64_to_i64);
        else if (lhs.is("float") and rhs.has_same_type_as(lhs))
            code.push_back(zen::f32_to_f32);
        else if (lhs.is("double") and rhs.has_same_type_as(lhs))
            code.push_back(zen::f64_to_f64);
        else if ((lhs.is("byte") or lhs.is("bool")) and rhs.has_same_type_as(lhs))
            code.push_back(zen::i8_to_i8);
        else
            throw std::logic_error(fmt::format(
                "[Error: Invalid types] Cannot compose operation " __FUNCTION__
                " for types {0} and {1}, please use type casting. Eg. x = {1}(y) // with y: {0}", rhs.type->name,
                lhs.type->name));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
        umost({rhs});
    }

    void composer::vm::composer::push(const std::string& name)
    {
        if (const auto location = scope.locals.find(name); location != scope.locals.end())
        {
            push(location->second);
        } else if (const auto function = functions.find(name); function != functions.end())
        {
            long address = std::get<i64>(function->second);
            zen::composer::composer::push(std::move(address), "long");
        } else if (name == "<return>" && scope.return_value)
        {
            push(scope.return_value.value());
        } else if (name.starts_with("<") and name.ends_with(">"))
        {
            push(get_type(name.substr(1, name.size()-2)));
        } else
        {
            throw std::out_of_range(fmt::format(
                "[Error: Symbol not found] Cannot compose operation " __FUNCTION__
                " for symbol \"{0}\" because it was not found, please define it in the respective scope. \nEg. {0}: T = V // with T & B being it's type and value respectively.",
                name));
        }
    }

#define KAIZEN_IF_ARITHMETICS_CHAIN(F)\
    if (rhs.is("byte")) \
        code.push_back(zen::F##_i8);\
    else if (lhs.is("short"))\
        code.push_back(zen::F##_i16);\
    else if (lhs.is("int"))\
        code.push_back(zen::F##_i32);\
    else if (lhs.is("long"))\
        code.push_back(zen::F##_i64);\
    else if (rhs.is("float")) \
        code.push_back(zen::F##_f32); \
    else if (rhs.is("double")) \
        code.push_back(zen::F##_f64);

#define KAIZEN_IF_ARITHMETICS_CHAIN_FLOAT(F)\
    if (rhs.is("byte")) \
    code.push_back(zen::F##_i8);\
    else if (lhs.is("short"))\
    code.push_back(zen::F##_i16);\
    else if (lhs.is("int"))\
    code.push_back(zen::F##_i32);\
    else if (lhs.is("long"))\
    code.push_back(zen::F##_i64);

    void composer::vm::composer::plus()
    {
        if (stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, stack.size()));
        const value rhs = top();
        pop();
        const value lhs = top();
        pop();
        push(lhs.type);
        if (lhs.has_same_type_as(rhs))
        {
            KAIZEN_IF_ARITHMETICS_CHAIN(add)
            else
                throw std::logic_error(fmt::format(
                    "[Error: Invalid types] Cannot compose operation {0} for types {1} because its not supported (try with long, double or byte)",
                    __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        else
        {
            throw std::logic_error(fmt::format(
                "[Error: Invalid types] Cannot compose operation {0} for types {1} and {2}, please use type casting and only work with integers (byte, short long). Eg. x = {2}(y) // with y: {1}",
                __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
        umost({lhs, rhs});
    }

    void composer::vm::composer::minus()
    {
        if (stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, stack.size()));
        const value rhs = top();
        pop();
        const value lhs = top();
        pop();
        push(lhs.type);
        if (lhs.has_same_type_as(rhs))
        {
            KAIZEN_IF_ARITHMETICS_CHAIN(sub)
            else
                throw std::logic_error(fmt::format(
                    "[Error: Invalid types] Cannot compose operation {0} for types {1} because its not supported (try with long, double or byte)",
                    __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        else
        {
            throw std::logic_error(fmt::format(
                "[Error: Invalid types] Cannot compose operation {0} for types {1} and {2}, please use type casting and only work with integers (byte, short long). Eg. x = {2}(y) // with y: {1}",
                __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
        umost({lhs, rhs});
    }

    void composer::vm::composer::times()
    {
        if (stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, stack.size()));
        const value rhs = top();
        pop();
        const value lhs = top();
        pop();
        push(lhs.type);
        if (lhs.has_same_type_as(rhs))
        {
            KAIZEN_IF_ARITHMETICS_CHAIN(mul)
            else
                throw std::logic_error(fmt::format(
                    "[Error: Invalid types] Cannot compose operation {0} for types {1} because its not supported (try with long, double or byte)",
                    __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        else
        {
            throw std::logic_error(fmt::format(
                "[Error: Invalid types] Cannot compose operation {0} for types {1} and {2}, please use type casting and only work with integers (byte, short long). Eg. x = {2}(y) // with y: {1}",
                __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
        umost({lhs, rhs});
    }

    void composer::vm::composer::slash()
    {
        if (stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, stack.size()));
        const value rhs = top();
        pop();
        const value lhs = top();
        pop();
        push(lhs.type);
        if (lhs.has_same_type_as(rhs))
        {
            KAIZEN_IF_ARITHMETICS_CHAIN(div)
            else
                throw std::logic_error(fmt::format(
                    "[Error: Invalid types] Cannot compose operation {0} for types {1} because its not supported (try with long, double or byte)",
                    __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        else
        {
            throw std::logic_error(fmt::format(
                "[Error: Invalid types] Cannot compose operation {0} for types {1} and {2}, please use type casting and only work with integers (byte, short long). Eg. x = {2}(y) // with y: {1}",
                __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
        umost({lhs, rhs});
    }

    void composer::vm::composer::modulo()
    {
        if (stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, stack.size()));
        const value rhs = top();
        pop();
        const value lhs = top();
        pop();
        push(lhs.type);
        if (lhs.has_same_type_as(rhs))
        {
            KAIZEN_IF_ARITHMETICS_CHAIN_FLOAT(mod)
            else
                throw std::logic_error(fmt::format(
                    "[Error: Invalid types] Cannot compose operation {0} for types {1} because its not supported (try with long or byte)",
                    __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        else
        {
            throw std::logic_error(fmt::format(
                "[Error: Invalid types] Cannot compose operation {0} for types {1} and {2}, please use type casting and only work with integers (byte, short long). Eg. x = {2}(y) // with y: {1}",
                __FUNCTION__, lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
        umost({lhs, rhs});
    }


    #define KAIZEN_CASTER_MAP_FOR(T)\
        {\
        {"byte", T##_to_i8},\
        {"bool", T##_to_i8},\
        {"short", T##_to_i16},\
        {"int", T##_to_i32},\
        {"long", T##_to_i64},\
        {"float", T##_to_f32},\
        {"double", T##_to_f64},\
        }

    void composer::vm::composer::call(const std::string& name, const i8& args_count)
    {
        static std::unordered_map<std::string,std::unordered_map<std::string,i64>> casters {
            {"bool", KAIZEN_CASTER_MAP_FOR(i8)},
            {"byte", KAIZEN_CASTER_MAP_FOR(i8)},
            {"short", KAIZEN_CASTER_MAP_FOR(i16)},
            {"int", KAIZEN_CASTER_MAP_FOR(i32)},
            {"long", KAIZEN_CASTER_MAP_FOR(i64)},
            {"float", KAIZEN_CASTER_MAP_FOR(f32)},
            {"double", KAIZEN_CASTER_MAP_FOR(f64)}
        };
        if (const auto caster_set = casters.find(name); caster_set != casters.end())
        {
            if (stack.size() < 2)
            {
                throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, stack.size()));
            }

            if (args_count != 1 and args_count != -1)
            {
                throw std::logic_error(fmt::format(
                    "[Error: Invalid state] wrong number of arguments for caster. If converting multiple values is the objetive, specify in different instructions.\nEg. u = {0}(w) x = {0}(y)",
                    name));
            }

            const value lhs = top();
            pop();
            const value rhs = top();
            pop();

            fmt::println("<{}> detected casting {} <- {}", scope.function_name,lhs.type->name, rhs.type->name);

            if (lhs.is(name))
            {
                if (const auto caster = (*caster_set).second.find(rhs.type->name); caster != caster_set->second.end())
                {
                    code.push_back((*caster).second);
                    code.push_back(rhs.address(scope.local_most_size));
                    umost({rhs});
                    if (args_count == -1)
                    {
                        push(lhs);
                    }
                } else
                {
                    throw std::logic_error(fmt::format(
                    "[Error: Invalid state] Cannot compose operation {} because the '{}' caster cannot convert from type '{}'.",
                    __FUNCTION__, name, rhs.type->name));
                }
            } else
            {
                throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because a wrong caster was found. Try using the '{}' caster instead.",
                __FUNCTION__, lhs.type->name));
            }
        }
        else
        {
            push(name);
        }
    }

    void composer::vm::composer::push(const std::shared_ptr<const type>& type)
    {
        stack.emplace(type, scope.local_most_size, temporary);
        code.push_back(most);
        code.push_back(-type->size);
        scope.local_most_size += type->size;
    }

    void composer::vm::composer::pop()
    {
        stack.pop();
    }
} // zen
