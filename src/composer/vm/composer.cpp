//
// Created by dte on 8/31/2025.
//

#include "composer.hpp"

#include <iostream>
#include <queue>
#include <sstream>

#include "exceptions/semantic_error.hpp"

namespace zen
{
    composer::value composer::vm::composer::top()
    {
        if (stack.empty())
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "value expected",
                                                 __FUNCTION__, stack.size()), ilc_offset);
        }
        return stack.top();
    }

    void composer::vm::composer::push(const value& v)
    {
        stack.push(v);
    }

    void composer::vm::composer::reset()
    {
        zen::composer::composer::reset();
        code = {hlt};
        functions = {
            {"int", {signature{}, 0}},
            {"long", {signature{}, 0}},
            {"short", {signature{}, 0}},
            {"float", {signature{}, 0}},
            {"double", {signature{}, 0}},
            {"bool", {signature{}, 0}},
            {"byte", {signature{}, 0}}
        };
        auto string_type = std::make_shared<type>("string", 0, type::kind::heap);
        types = {
            {"unit", std::make_shared<type>("unit", 0)},
            {"int", std::make_shared<type>("int", 4)},
            {"short", std::make_shared<type>("short", 2)},
            {"long", std::make_shared<type>("long", 8)},
            {"float", std::make_shared<type>("float", 4)},
            {"double", std::make_shared<type>("double", 8)},
            {"function", std::make_shared<type>("function", 8)},
            {"bool", std::make_shared<type>("bool", 1)},
            {"byte", std::make_shared<type>("byte", 1)},
            {"string", string_type},
        };
        string_type->add_field("len", get_type("long"));
        string_type->add_field("data", get_type("long"));
    }

    composer::vm::composer::composer(int& ilc_offset):
        zen::composer::composer(ilc_offset)
    {
        this->composer::reset();
    }

    void composer::vm::composer::begin(std::string name)
    {
        scope.clear();
        scope.function_name = name;
        functions.emplace(std::move(name), std::make_tuple<signature, size_t>(
                              signature{
                                  .type = get_type("unit")
                              }, code.size()));
        scope.return_value.emplace(value(get_type("unit"), 0));
    }

    void composer::vm::composer::set_parameter(std::string name, const std::string& type)
    {
        std::get<signature>(functions.at(scope.function_name)).parameters.emplace_back(get_type(type));
        const auto& t = get_type(type);
        const i64 address = scope.local_most_size;
        scope.local_most_size += t->get_size();
        scope.locals.emplace(name, symbol(name, t, address));
    }

    void composer::vm::composer::set_return_type(const std::string& name)
    {
        if (scope.local_most_size)
        {
            throw std::logic_error("Function returned more than once or return is being set after parameters");
        }
        const auto type = get_type(name);
        if (type->get_size() != 0)
        {
            scope.return_value.emplace(value(type, scope.local_most_size));
            scope.local_most_size += type->get_size();
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
        const i64 address = scope.local_most_size;
        code.push_back(zen::most);
        code.push_back(-t->get_size());
        scope.local_most_size += t->get_size();
        scope.locals.emplace(name, symbol(name, t, address));
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
            if (const auto most_delta = scope.local_most_size - return_value->type->get_size(); most_delta > 0)
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
        throw exceptions::semantic_error(fmt::format(
                                             "no such type \"{}\"", name), ilc_offset);
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
                else if (_code == modify)
                {
                    fmt::print("modify {} {} ", code[i + 1], code[i + 2]);
                    i += 2;
                }
                else if (_code == zen::call)
                {
                    fmt::print("call {} ", code[i + 1]);
                    i += 1;
                }
                else if (_code == add_i32 || _code == add_i64 || _code == add_f64 || _code == add_i8 || _code ==
                    add_i16)
                {
                    fmt::print("add {} {} {} ", code[i + 1], code[i + 2], code[i + 3]);
                    i += 3;
                }
                else if (_code == sub_i64 || _code == sub_i32 || _code == sub_f64 || _code == sub_i8 || _code ==
                    sub_i16)
                {
                    fmt::print("sub {} {} {} ", code[i + 1], code[i + 2], code[i + 3]);
                    i += 3;
                }
                else if (_code == mul_i32 || _code == mul_i64 || _code == mul_f64 || _code == mul_i8 || _code ==
                    mul_i16)
                {
                    fmt::print("mul {} {} {} ", code[i + 1], code[i + 2], code[i + 3]);
                    i += 3;
                }
                else if (_code == div_i32 || _code == div_i64 || _code == div_f64 || _code == div_i8 || _code ==
                    div_i16)
                {
                    fmt::print("div {} {} {} ", code[i + 1], code[i + 2], code[i + 3]);
                    i += 3;
                }
                else if (_code >= push_i8 && _code <= push_boolean)
                {
                    fmt::print("push {} ", code[i + 1]);
                    i += 1;
                }
                else if (_code >= i8_to_i64 && _code <= f64_to_boolean)
                {
                    fmt::print("cp {} {} ", code[i + 1], code[i + 2]);
                    i += 2;
                }
                else if (_code == write_str)
                {
                    fmt::print("write {} {} ", code[i + 1], code[i + 2]);
                    i += 2;
                }
                else if (_code >= write_i8 && _code <= write_f64)
                {
                    fmt::print("write {} ", code[i + 1]);
                    i += 1;
                }
                else if (_code >= read_i8 && _code <= read_f64)
                {
                    fmt::print("read {} ", code[i + 1]);
                    i += 1;
                }
                else if (_code == ret)
                    fmt::print("ret ");
                else if (_code == hlt)
                    fmt::print("hlt ");
                else
                    std::cout << _code << ' ';
                if (_code == zen::ret || _code == zen::hlt) break;
            }
            std::cout << std::endl;
        }
    }

    void composer::vm::composer::assign()
    {
        if (stack.size() < 2)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "<KAIZEN-INTERNAL-API> Cannot compose operation {} because stack size {} is below expected",
                                                 __FUNCTION__, stack.size()), ilc_offset);
        }
        const value rhs = top();
        pop();
        const value lhs = top();
        pop();
        if (lhs.kind == value::constant or lhs.kind == value::temporary)
        {
            throw exceptions::semantic_error("cannot assign value to a constant or temporary value", ilc_offset,
                                             rhs.kind == value::variable
                                                 ? "please consider changing operands order from x = y to y = x"
                                                 : "");
        }

        if (lhs.is("unit"))
        {
            throw exceptions::semantic_error("cannot return values from unit function", ilc_offset,
                                             fmt::format("please consider changing the return type of \"{}\" to {}",
                                                         scope.function_name, rhs.type->name));
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
        else if (lhs.type->kind == type::heap and rhs.has_same_type_as(lhs))
            code.push_back(zen::i64_to_i64);
        else
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot assign {} to {}", rhs.type->name,
                                                 lhs.type->name), ilc_offset,
                                             fmt::format(
                                                 "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",
                                                 lhs.type->name, rhs.type->name));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
    }

    std::vector<std::string> split_name(const std::string &name)
    {
        std::vector<std::string> result;
        std::istringstream iss(name);
        std::string token;
        while (std::getline(iss, token, '.'))
        {
            if (not token.empty())
                result.push_back(token);
        }
        return result;
    }

    void composer::vm::composer::_push_variable(const std::vector<std::string> & tokens, const std::map<std::string, symbol>::iterator& location)
    {
        value val = location->second;
        std::pair<i64, std::shared_ptr<const type>> item = {0, val.type};
        for (int i = 1; i < tokens.size(); ++i)
        {
            const auto [fst, snd] = item.second->get_field(tokens[i], ilc_offset);
            item.first += fst;
            item.second = snd;
        }
        val.offset = item.first;
        val.type = item.second;
        // fmt::println("{}: {} [{}]", name, val.type->name, val.offset);
        push(val);
    }

    void composer::vm::composer::_push_function(const std::unordered_map<std::string, std::tuple<signature, long long>>::iterator& function)
    {
        long address = std::get<i64>(function->second);
        zen::composer::composer::push(std::move(address), "function");
    }

    void composer::vm::composer::_push_return_value()
    {
        push(scope.return_value.value());
    }

    void composer::vm::composer::_push_temporary_value(const std::string& type_name)
    {
        push(get_type(type_name.substr(1, type_name.size() - 2)));
    }

    void composer::vm::composer::push(const std::string& name)
    {
        std::vector<std::string> tokens = split_name(name);
        if (tokens.empty())
            throw exceptions::semantic_error(fmt::format("invalid name '{}'", name), ilc_offset);

        if (const auto location = scope.locals.find(tokens.front()); location != scope.locals.end())
            _push_variable(tokens, location);
        else if (const auto function = functions.find(tokens.front()); function != functions.end())
            _push_function(function);
        else if (name == "<return>" && scope.return_value)
            _push_return_value();
        else if (name.starts_with("<") and name.ends_with(">"))
            _push_temporary_value(name);
        else
            throw exceptions::semantic_error(fmt::format(
                                                 "no such symbol {}", name), ilc_offset,
                                             fmt::format(
                                                 "please define it in the respective scope. Eg. {0}: T = V // with T & V being it's type and value respectively.",
                                                 name));
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
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot sum type \"{}\"", lhs.type->name), ilc_offset);
        }
        else
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot sum {} with {}", lhs.type->name,
                                                 rhs.type->name), ilc_offset,
                                             fmt::format(
                                                 "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",
                                                 lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
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
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot subtract type \"{}\"", lhs.type->name), ilc_offset);
        }
        else
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot subtract {} with {}", lhs.type->name,
                                                 rhs.type->name), ilc_offset,
                                             fmt::format(
                                                 "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",
                                                 lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
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
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot multiply type \"{}\"", lhs.type->name), ilc_offset);
        }
        else
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot multiply {} with {}", lhs.type->name,
                                                 rhs.type->name), ilc_offset,
                                             fmt::format(
                                                 "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",
                                                 lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
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
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot divide type \"{}\"", lhs.type->name), ilc_offset);
        }
        else
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot divide {} with {}", lhs.type->name,
                                                 rhs.type->name), ilc_offset,
                                             fmt::format(
                                                 "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",
                                                 lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
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
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot compute modulo type \"{}\"", lhs.type->name,
                                                     "please consider casting operands to integers (byte, short, int, long) if applicable"),
                                                 ilc_offset);
        }
        else
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot compute modulo of {} with {}", lhs.type->name,
                                                 rhs.type->name), ilc_offset,
                                             fmt::format(
                                                 "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",
                                                 lhs.type->name, rhs.type->name));
        }
        code.push_back(top().address(scope.local_most_size));
        code.push_back(lhs.address(scope.local_most_size));
        code.push_back(rhs.address(scope.local_most_size));
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

    std::optional<composer::value> composer::vm::composer::_push_calle_return_value(const signature& sig)
    {
        std::optional<value> result;
        if (sig.type->get_size())
        {
            const i64 address = scope.local_most_size;
            code.push_back(zen::most);
            code.push_back(-sig.type->get_size());
            scope.local_most_size += sig.type->get_size();
            return value(sig.type, address, value::temporary);
        }
        return std::nullopt;
    }

    void composer::vm::composer::_push_calle_arguments(const signature& sig, const i8& args_count)
    {
        std::deque<value> values;
        for (int i = 0; i < args_count; i++)
        {
            values.push_front(top());
            pop();
        }
        for (int i = 0; i < args_count; i++)
        {
            value& value = values.at(i);
            if (const auto& param_type = sig.parameters.at(i); value.is(param_type->name))
            {
                if (value.is("byte"))
                {
                    code.push_back(zen::push_i8);
                } else if (value.is("bool"))
                {
                    code.push_back(zen::push_boolean);
                } else if (value.is("short"))
                {
                    code.push_back(zen::push_i16);
                } else if (value.is("int"))
                {
                    code.push_back(zen::push_i32);
                } else if (value.is("long"))
                {
                    code.push_back(zen::push_i64);
                } else if (value.is("float"))
                {
                    code.push_back(zen::push_f32);
                } else if (value.is("double"))
                {
                    code.push_back(zen::push_f64);
                } else
                {
                    code.push_back(zen::push_i64);
                }
                code.push_back(value.address(scope.local_most_size));
            }
            else
            {
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot pass {} argument to {} parameter in {} place at call",
                                                     value.type->name,
                                                     param_type->name, i + 1), ilc_offset,
                                                 fmt::format(
                                                     "please consider using type casting. Eg. {}(y) // with y: {}, if applicable.",
                                                     param_type->name, value.type->name));
            }
        }
    }

    composer::call_result composer::vm::composer::_call_caster(const std::string& name, const i8& args_count, const std::unordered_map<std::string, std::unordered_map<std::string, i64>>::iterator & caster_set)
    {
        if (stack.size() < 2)
        {
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, stack.size()));
        }

        if (args_count != 1 and args_count != -1)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "wrong number of arguments for caster of {}", name), ilc_offset,
                                             fmt::format(
                                                 "please consider using multiple instructions when converting multiple values.\nEg. v = {0}(w) x = {0}(y)",
                                                 name));
        }
        const value rhs = top();
        pop();
        if (top().is("function"))
        {
            pop(); // pop the caster itself from composer's stack
            if (args_count < 0)
            {
                push(get_type(name));
            }
            else if (stack.empty())
            {
                throw exceptions::semantic_error(fmt::format(
                                                     "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                                                     __FUNCTION__, stack.size()), ilc_offset);
            }
        }
        const value lhs = top();
        pop();

        if (lhs.is(name))
        {
            if (const auto caster = (*caster_set).second.find(rhs.type->name); caster != caster_set->second.end())
            {
                code.push_back((*caster).second);
                code.push_back(lhs.address(scope.local_most_size));
                code.push_back(rhs.address(scope.local_most_size));
                if (args_count == -1)
                {
                    push(lhs);
                }
                return call_result::casting;
            }
            else
            {
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot cast from {} to {}", rhs.type->name, name),
                                                 ilc_offset);
            }
        }
        else
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot cast from {} to {} using {} caster", rhs.type->name,
                                                 lhs.type->name, name), ilc_offset,
                                             fmt::format("please consider using a {} caster if applicable",
                                                         lhs.type->name));
        }
    }

    composer::call_result composer::vm::composer::_call_function(const std::string& name, const i8& args_count, const std::unordered_map<std::string, std::tuple<signature, long long>>::iterator& func_it)
    {
        const i8 abs_args_count = abs(args_count);
        const i64& addr = std::get<i64>(func_it->second);
        const signature& sig = std::get<signature>(func_it->second);
        if (stack.size() < abs_args_count + 1 || abs_args_count != sig.parameters.size())
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "wrong number of arguments for \"{}\"", name), ilc_offset);
        }
        // if (args_count < 0) // when assigning, copy the value directly to lhs
        // {
        auto returned = _push_calle_return_value(sig);
        // }
        _push_calle_arguments(sig, abs_args_count);
        if (top().is("function"))
        {
            pop(); // pop the function itself from composer's stack
        }
        if (returned)
        {
            push(returned.value());
        }
        code.push_back(zen::call);
        code.push_back(addr);
        return call_result::result;
    }

    composer::call_result composer::vm::composer::_call_instruction_write_str(const std::string& name, const i8& args_count)
    {
        if (stack.size() < 3 or args_count != 3)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "wrong number of arguments for <native>\"{}\" 3 expected", name), ilc_offset);
        }
        std::stack<value> args;
        for (int i = 0 ; i < args_count; i++)
        {
            value v = top();
            pop();
            v.prepare(code, scope.local_most_size);
            args.push(v);
        }
        code.push_back(zen::write_str);
        for (int i = 0 ; i < args_count; i++)
        {
            code.push_back(args.top().address(scope.local_most_size));
            args.pop();
        }
        return call_result::result;
    }

    composer::call_result composer::vm::composer::_call_instruction(const zen::instruction& name, const i8& args_count, const i8& expected_args_count)
    {
        if (stack.size() < expected_args_count or args_count != expected_args_count)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "wrong number of arguments for <native>\"{}\" 3 expected", static_cast<i32>(name)), ilc_offset);
        }
        std::stack<value> args;
        for (int i = 0 ; i < args_count; i++)
        {
            value v = top();
            pop();
            v.prepare(code, scope.local_most_size);
            args.push(v);
        }
        code.push_back(name);
        for (int i = 0 ; i < args_count; i++)
        {
            code.push_back(args.top().address(scope.local_most_size));
            args.pop();
        }
        return call_result::result;
    }

    composer::call_result composer::vm::composer::call(const std::string& name, const i8& args_count)
    {
        static std::unordered_map<std::string, std::unordered_map<std::string, i64>> casters{
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
            return _call_caster(name, args_count, caster_set);
        }

        if (const auto func_it = functions.find(name); func_it != functions.end())
        {
            return _call_function(name, args_count, func_it);
        }
        const int name_i32 = strtol(name.c_str(), nullptr, 10);
        if (name_i32 == write_str)
        {
            return _call_instruction_write_str(name, args_count);
        }
        if (name_i32 >= write_i8 and name_i32 <= write_f64)
        {
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 1);
        }
        throw exceptions::semantic_error(fmt::format("function \"{}\" was not found", name), ilc_offset);
    }

    void composer::vm::composer::push(const std::shared_ptr<const type>& type)
    {
        stack.emplace(type, scope.local_most_size, value::temporary);
        code.push_back(most);
        code.push_back(-type->get_size());
        scope.local_most_size += type->get_size();
    }

    void composer::vm::composer::pop()
    {
        stack.pop();
    }
} // zen
