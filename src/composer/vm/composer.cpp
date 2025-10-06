//
// Created by dte on 8/31/2025.
//

#include "composer.hpp"

#include <iostream>
#include <queue>
#include <sstream>

#include "for_scope.hpp"
#include "exceptions/semantic_error.hpp"

#define KAIZEN_IF_SCOPE_OPENING(T)\
    scope->__dncd__push(block_scope::__unsafely_make(T));
#define KAIZEN_IF_SCOPE_DESTROY_HEAP_LOCALS()\
    for (const auto & local_set : scope->___dncd__deepest_locals())\
        {\
            /*fmt::println("~(1 {} {}", local_set.first, local_set.second.size());*/\
            for(const auto & local : local_set.second){\
                if (local->type->kind == type::heap and not local->no_destructor)\
                {\
                    try\
                    {\
                        /*fmt::println("[zenDestructor]({})", local->label);*/\
                        push(local);\
                        call("[zenDestructor]",1, call_result::pushed);\
                    } catch (const std::exception & e)\
                    {\
                        throw exceptions::semantic_error(fmt::format("missing destructor implementation for type {}", local->type->name), _ilc_offset);\
                    }\
                }\
            }\
        }
#define KAIZEN_IF_SCOPE_CLOSURE()\
    KAIZEN_IF_SCOPE_DESTROY_HEAP_LOCALS()\
    if (const i64 size = scope->__dncd__pop(scope->return_status); std::abs(size) > 0)\
    {\
        code.push_back(most);\
        code.push_back(std::abs(size));\
    }

namespace zen
{
    std::shared_ptr<composer::value> composer::vm::composer::top()
    {
        if (_stack.empty())
        {
            return std::make_shared<value>(get_type("unit"), 0);
        }
        return _stack.top();
    }

    void composer::vm::composer::push(const std::shared_ptr<value>& v)
    {
        _stack.push(v);
    }

    void composer::vm::composer::reset()
    {
        zen::composer::composer::reset();
        code = {hlt};

        auto string_type = std::make_shared<type>("string", 0, type::kind::heap);
        auto unit_type = std::make_shared<type>("unit", 0);
        types = {
            {"unit", unit_type},
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
        functions = {
            {"int", {{signature{}, 0}}},
            {"long", {{signature{}, 0}}},
            {"short", {{signature{}, 0}}},
            {"float", {{signature{}, 0}}},
            {"double", {{signature{}, 0}}},
            {"bool", {{signature{}, 0}}},
            {"byte", {{signature{}, 0}}},
            {"[zenConstructor]", {{signature{string_type},0, false}}},
            {"zenCopy", {{signature{unit_type, {string_type, string_type}}, 0, false}}},
            {"[zenDestructor]", {{signature{unit_type, {string_type}}, 0, false}}},
        };
    }

    composer::vm::composer::composer(int& ilc_offset):
        zen::composer::composer(ilc_offset)
    {
        this->composer::reset();
    }

    void composer::vm::composer::begin(std::string name)
    {
        const auto definition = std::ref(functions[name].emplace_back(
            signature{
                .type = get_type("unit")
            }, code.size()));
        scope = std::make_unique<function_scope>(definition);
        scope->type = scope::in_function;
        scope->name = name;
        scope->return_data.value = (std::make_shared<value>(get_type("unit"), 0));
    }

    void composer::vm::composer::set_parameter(std::string name, const std::string& type)
    {
        const auto t = get_type(type);
        scope->definition.get().signature.parameters.emplace_back(get_type(type));
        const i64 address = scope->stack_usage - /* jump callee IP */static_cast<i64>(sizeof(i64));
        scope->stack_usage += t->get_size();
        auto sym = std::make_shared<value>(name, t, address);
        sym->no_destructor = true;
        scope->locals[name].push_back(sym);
    }


#define KAIZEN_REQUIRE_SCOPE(S)\
if (not scope->is(S)) throw std::logic_error(fmt::format("cannot invoke {} ousize of {}", __FUNCTION__, #S))

#define KAIZEN_REQUIRE_GLOBAL_SCOPE()\
if (scope and scope->is(scope::in_function)) throw std::logic_error(fmt::format("{} can only be invoked in global scope", __FUNCTION__))

    void composer::vm::composer::set_return_type(const std::string& name)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (scope->stack_usage)
        {
            throw std::logic_error("return type cannot be set after params");
        }
        const auto type = get_type(name);
        if (type->get_size() != 0)
        {
            scope->return_data.value = (
                std::make_shared<value>(type, scope->stack_usage - /* jump callee IP */static_cast<i64>(sizeof(i64))));
            scope->stack_usage += type->get_size();
        }
        scope->definition.get().signature.type = type;
    }

    void composer::vm::composer::return_value()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (scope->get_return_status() == block_scope::concise_return)
            throw exceptions::semantic_error("cannot return values more than once", _ilc_offset);
        scope->set_return_status(block_scope::concise_return);
        const auto rhs = top();
        pop();
        if (not scope->return_data.value->is("unit"))
        {
            push(scope->return_data.value);
            push(rhs);
            assign();
        }
    }

    void composer::vm::composer::assume_returned()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        scope->set_return_status(block_scope::concise_return);
    }

    void composer::vm::composer::set_return_name(const std::string& name)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        scope->return_data.name = name;
    }

    void composer::vm::composer::set_local(std::string name, const std::string& type)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        constexpr std::string no_constructor = "::noConstructor";
        std::shared_ptr<const zen::composer::type> t;
        const bool no_construtor_mode = type.ends_with(no_constructor);
        if (no_construtor_mode)
            t = get_type(type.substr(0, type.length() - no_constructor.size()));
        else
            t = get_type(type);
        scope->set_local(name, t);
        // scope->locals.emplace(name, symbol(name, t, scope->stack_usage));
        // scope->stack_usage += t->get_size();
        code.push_back(zen::most);
        code.push_back(-t->get_size());
        if (t->kind == type::heap and not no_construtor_mode)
        {
            if (const auto func_it = functions.find("[zenConstructor]"); func_it != functions.end())
            {
                for (auto& zen_allocator : func_it->second)
                {
                    if (zen_allocator.signature.type == t)
                    {
                        push(name);
                        // will push 8 bytes (result of the call)
                        _call_function_overload({}, zen_allocator, call_result::pushed_from_constructor);
                        // _call_instruction(i64_to_i64, 2, 2);
                    }
                }
            }
            else
            {
                throw exceptions::semantic_error(fmt::format("missing construtor implementation for type {}", type),
                                                 _ilc_offset);
            }
        }
    }

    void composer::vm::composer::end()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (scope->return_data.name)
        {
            push(scope->return_data.name.value());
            return_value();
        }
        else if (scope->get_return_status() != block_scope::concise_return and not scope->return_data.value->is("unit"))
            throw exceptions::semantic_error("missing return value", _ilc_offset,
                                             fmt::format(
                                                 "please use a named return or ensure that all control paths return a value of type {}",
                                                 scope->return_data.value->type->name));
        KAIZEN_IF_SCOPE_DESTROY_HEAP_LOCALS();
        const auto&  sig = scope->definition.get().signature;
        const auto sizes = get_return_size(sig) + get_parameters_size(sig);
        if (const auto most_delta = scope->stack_usage - sizes; most_delta > 0)
        {
            code.push_back(zen::most);
            code.push_back(most_delta);
        }
        code.push_back(zen::ret);
        scope = nullptr;
    }

    std::shared_ptr<const composer::type>& composer::vm::composer::get_type(const std::string& name)
    {
        if (types.contains(name))
        {
            return types.at(name);
        }
        throw exceptions::semantic_error(fmt::format(
                                             "no such type \"{}\"", name), _ilc_offset);
    }

    void composer::vm::composer::bake()
    {
        for (const auto& function : functions)
        {
            for (const auto& overload : function.second)
            {
                if (not overload.defined) continue;
                std::string params;
                for (const auto& param : overload.signature.parameters)
                {
                    params += fmt::format("{}, ", param->name);
                }
                fmt::println("{}({}) = {}", function.first, params.empty() ? "" : params.substr(0, params.length() - 2),
                             overload.signature.type ? overload.signature.type->name : "*");
                for (i64 i = overload.address; i < code.size(); i++)
                {
                    const auto& _code = code[i];
                    if (_code == most)
                    {
                        fmt::print("most, {}, ", code[i + 1]);
                        i += 1;
                    }
                    else if (_code == walk)
                    {
                        fmt::print("walk, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == copy)
                    {
                        fmt::print("copy, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == allocate)
                    {
                        fmt::print("allocate, {}, {}, ", code[i + 1], code[i + 2]);
                        i += 2;
                    }
                    else if (_code == deallocate)
                    {
                        fmt::print("deallocate, {}, ", code[i + 1]);
                        i += 1;
                    }
                    else if (_code == zen::call)
                    {
                        fmt::print("call, {}, ", code[i + 1]);
                        i += 1;
                    }
                    else if (_code == add_i32 || _code == add_i64 || _code == add_f64 || _code == add_i8 || _code ==
                        add_i16)
                    {
                        fmt::print("add, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == sub_i64 || _code == sub_i32 || _code == sub_f64 || _code == sub_i8 || _code ==
                        sub_i16)
                    {
                        fmt::print("sub, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == mul_i32 || _code == mul_i64 || _code == mul_f64 || _code == mul_i8 || _code ==
                        mul_i16)
                    {
                        fmt::print("mul, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == div_i32 || _code == div_i64 || _code == div_f64 || _code == div_i8 || _code ==
                        div_i16)
                    {
                        fmt::print("div, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code >= push_i8 && _code <= push_boolean)
                    {
                        fmt::print("push, {}, ", code[i + 1]);
                        i += 1;
                    }
                    else if (_code >= i8_to_i64 && _code <= f64_to_boolean)
                    {
                        fmt::print("cp, {}, {}, ", code[i + 1], code[i + 2]);
                        i += 2;
                    }

                    else if (_code >= gt_i8 && _code <= neq_f64)
                    {
                        fmt::print("cmp, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == write_str)
                    {
                        fmt::print("write, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == read_str)
                    {
                        fmt::print("read, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == go_if_not)
                    {
                        fmt::print("go_if_not, {}, {}, ", code[i + 1], code[i + 2]);
                        i += 2;
                    }
                    else if (_code == go)
                    {
                        fmt::print("go, {}, ", code[i + 1]);
                        i += 1;
                    }
                    else if (_code >= write_i8 && _code <= write_f64)
                    {
                        fmt::print("write, {}, {}, ", code[i + 1], code[i + 2]);
                        i += 2;
                    }
                    else if (_code >= read_i8 && _code <= read_f64)
                    {
                        fmt::print("read, {}, {}, ", code[i + 1], code[i + 2]);
                        i += 2;
                    }
                    else if (_code == ret)
                        fmt::print("ret, ");
                    else if (_code == hlt)
                        fmt::print("hlt, ");
                    else
                        std::cout << _code << ' ';
                    if (_code == zen::ret || _code == zen::hlt) break;
                }
                std::cout << std::endl;
            }
        }
    }

    void composer::vm::composer::assign()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (_stack.size() < 2)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "<KAIZEN-INTERNAL-API> Cannot compose operation {} because stack size {} is below expected",
                                                 __FUNCTION__, _stack.size()), _ilc_offset);
        }
        const auto rhs = top();
        pop();
        auto lhs = top();
        pop();
        if (lhs->kind == value::constant)
        {
            throw exceptions::semantic_error("cannot assign value to constant", _ilc_offset,
                                             rhs->kind == value::variable
                                                 ? "please consider changing operands order from x = y to y = x if applicable"
                                                 : "");
        }

        if (lhs->is("unit"))
        {
            throw exceptions::semantic_error("cannot return values from unit function", _ilc_offset,
                                             fmt::format("please consider changing the return type of \"{}\" to {}",
                                                         scope->name, rhs->type->name));
        }

        if (lhs->no_destructor)
        {
            fmt::println("no_destructor being assigned {}, shadowing parameter", lhs->label);
            set_local(lhs->label, lhs->type->name);
            lhs = scope->get_local(lhs->label);
        }

        if (rhs->is("short") and rhs->has_same_type_as(*lhs))
            code.push_back(zen::i16_to_i16);
        else if (rhs->is("int") and rhs->has_same_type_as(*lhs))
            code.push_back(zen::i32_to_i32);
        else if (rhs->is("long") and rhs->has_same_type_as(*lhs))
            code.push_back(zen::i64_to_i64);
        else if (lhs->is("float") and rhs->has_same_type_as(*lhs))
            code.push_back(zen::f32_to_f32);
        else if (lhs->is("double") and rhs->has_same_type_as(*lhs))
            code.push_back(zen::f64_to_f64);
        else if ((lhs->is("byte") or lhs->is("bool")) and rhs->has_same_type_as(*lhs))
            code.push_back(zen::i8_to_i8);
        // else if (lhs->type->kind == type::heap and rhs->has_same_type_as(*lhs))
        // code.push_back(zen::i64_to_i64);
        else
        {
            try
            {
                push(lhs);
                push(rhs);
                call("zenCopy", 2, call_result::pushed);
                return;
            }
            catch (std::exception& _)
            {
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot assign {} to {}", rhs->type->name,
                                                     lhs->type->name), _ilc_offset,
                                                 fmt::format(
                                                     "please consider using type casting. Eg. x = {}(y) or define a custom copy operator",
                                                     lhs->type->name, rhs->type->name));
            }
        }
        code.push_back(lhs->address(scope->stack_usage));
        code.push_back(rhs->address(scope->stack_usage));
    }

    std::vector<std::string> split_name(const std::string& name)
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

    void composer::vm::composer::_push_variable(const std::vector<std::string>& tokens,
                                                const std::shared_ptr<value>& location)
    {
        std::shared_ptr<value> val = std::make_shared<value>(*location);
        std::pair<i64, std::shared_ptr<const type>> item = {0, val->type};
        for (int i = 1; i < tokens.size(); ++i)
        {
            const auto [fst, snd] = item.second->get_field(tokens[i], _ilc_offset);
            item.first += fst;
            item.second = snd;
            val->no_destructor = false;
        }
        val->offset = item.first;
        val->type = item.second;
        // fmt::println("{}: {} [{}]", name, val.type->name, val.offset);
        if (val->offset)
        {
            push(val->type);
            code.push_back(walk);
            code.push_back(top()->address(scope->stack_usage));
            code.push_back(location->address(scope->stack_usage));
            code.push_back(val->offset);
        }
        else
            push(val);
    }

    void composer::vm::composer::_push_function()
    {
        zen::composer::composer::push(0, "function");
    }

    void composer::vm::composer::_push_temporary_value(const std::string& type_name)
    {
        push(get_type(type_name.substr(1, type_name.size() - 2)));
    }

    void composer::vm::composer::push(const std::string& name)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        std::vector<std::string> tokens = split_name(name);
        if (tokens.empty())
            throw exceptions::semantic_error(fmt::format("invalid name '{}'", name), _ilc_offset);

        if (const auto location = scope->get_local(tokens.front()))
            _push_variable(tokens, location);
        else if (name.starts_with("<return>") && scope->return_data.value)
            _push_variable(tokens, scope->return_data.value);
        else if (const auto function = functions.find(tokens.front()); function != functions.end())
            _push_function();
        else if (name.starts_with("<") and name.ends_with(">"))
            _push_temporary_value(name);
        else
            throw exceptions::semantic_error(fmt::format(
                                                 "no such symbol {}", name), _ilc_offset,
                                             fmt::format(
                                                 "please define it in the respective scope. Eg. {0}: T = V // with T & V being it's type and value respectively.",
                                                 name));
    }

#define KAIZEN_IF_ARITHMETICS_CHAIN(F)\
    if (rhs->is("byte")) \
        code.push_back(zen::F##_i8);\
    else if (lhs->is("short"))\
        code.push_back(zen::F##_i16);\
    else if (lhs->is("int"))\
        code.push_back(zen::F##_i32);\
    else if (lhs->is("long"))\
        code.push_back(zen::F##_i64);\
    else if (rhs->is("float")) \
        code.push_back(zen::F##_f32); \
    else if (rhs->is("double")) \
        code.push_back(zen::F##_f64);

#define KAIZEN_IF_ARITHMETICS_CHAIN_FLOAT(F)\
    if (rhs->is("byte")) \
    code.push_back(zen::F##_i8);\
    else if (lhs->is("short"))\
    code.push_back(zen::F##_i16);\
    else if (lhs->is("int"))\
    code.push_back(zen::F##_i32);\
    else if (lhs->is("long"))\
    code.push_back(zen::F##_i64);

#define KAIZEN_DEFINE_ARITH_COMPOSITION(N, A, IP, CG)\
    void composer::vm::composer::N()\
    {\
        KAIZEN_REQUIRE_SCOPE(scope::in_function);\
        if (_stack.size() < 2)\
            throw std::logic_error(fmt::format(\
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",\
                __FUNCTION__, _stack.size()));\
        const auto rhs = top();\
        pop();\
        const auto lhs = top();\
        pop();\
        push(lhs->type);\
        if (lhs->has_same_type_as(*rhs))\
        {\
            CG(IP)\
            else\
                throw exceptions::semantic_error(fmt::format(\
                                                     "cannot "#A" type \"{}\"", lhs->type->name), _ilc_offset);\
        }\
        else\
        {\
            throw exceptions::semantic_error(fmt::format(\
                                                 "cannot "#A" {} with {}", lhs->type->name,\
                                                 rhs->type->name), _ilc_offset,\
                                             fmt::format(\
                                                 "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",\
                                                 lhs->type->name, rhs->type->name));\
        }\
        code.push_back(top()->address(scope->stack_usage));\
        code.push_back(lhs->address(scope->stack_usage));\
        code.push_back(rhs->address(scope->stack_usage));\
    }


#define KAIZEN_DEFINE_LOGIC_COMPOSITION(N, I)\
    void composer::vm::composer::N()\
    {\
        KAIZEN_REQUIRE_SCOPE(scope::in_function);\
        if (_stack.empty())\
            throw std::logic_error(fmt::format(\
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",\
                __FUNCTION__, _stack.size()));\
        const auto val = top();\
        pop();\
        push("<bool>");\
        if (val->is("bool")) \
                code.push_back(zen::boolean_##I);\
            else\
                throw exceptions::semantic_error(fmt::format(\
                                                     "cannot apply "#I" to type \"{}\"", val->type->name), _ilc_offset, "please consider using type casting. Eg. bool(x)");\
        code.push_back(val->address(scope->stack_usage));\
    }

#define KAIZEN_IF_RELATIONAL_CHAIN(F)\
    if (rhs->is("byte")) \
        code.push_back(zen::F##_i8);\
    else if (lhs->is("short"))\
        code.push_back(zen::F##_i16);\
    else if (lhs->is("int"))\
        code.push_back(zen::F##_i32);\
    else if (lhs->is("long"))\
        code.push_back(zen::F##_i64);\
    else if (rhs->is("float")) \
        code.push_back(zen::F##_f32); \
    else if (rhs->is("double")) \
        code.push_back(zen::F##_f64);

#define KAIZEN_DEFINE_RELATIONAL_COMPOSITION(N, A, I)\
    void composer::vm::composer::N()\
    {\
        KAIZEN_REQUIRE_SCOPE(scope::in_function);\
        if (_stack.size() < 2)\
            throw std::logic_error(fmt::format(\
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",\
                __FUNCTION__, _stack.size()));\
        const auto rhs = top();\
        pop();\
        const auto lhs = top();\
        pop();\
        push("<bool>");\
        if (lhs->has_same_type_as(*rhs))\
        {\
            KAIZEN_IF_RELATIONAL_CHAIN(I)\
            else\
                throw exceptions::semantic_error(fmt::format(\
                                                     "cannot compare ("#A") type \"{}\"", lhs->type->name), _ilc_offset);\
        }\
        else\
        {\
            throw exceptions::semantic_error(fmt::format(\
                                                 "cannot compare ("#A") {} with {}", lhs->type->name,\
                                                 rhs->type->name), _ilc_offset,\
                                             fmt::format(\
                                                 "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",\
                                                 lhs->type->name, rhs->type->name));\
        }\
        code.push_back(top()->address(scope->stack_usage));\
        code.push_back(lhs->address(scope->stack_usage));\
        code.push_back(rhs->address(scope->stack_usage));\
    }


    KAIZEN_DEFINE_ARITH_COMPOSITION(plus, sum, add, KAIZEN_IF_ARITHMETICS_CHAIN);
    KAIZEN_DEFINE_ARITH_COMPOSITION(minus, subtract, sub, KAIZEN_IF_ARITHMETICS_CHAIN);
    KAIZEN_DEFINE_ARITH_COMPOSITION(times, multiply, mul, KAIZEN_IF_ARITHMETICS_CHAIN);
    KAIZEN_DEFINE_ARITH_COMPOSITION(slash, divide, div, KAIZEN_IF_ARITHMETICS_CHAIN);
    KAIZEN_DEFINE_ARITH_COMPOSITION(modulo, compute modulo, mod, KAIZEN_IF_ARITHMETICS_CHAIN_FLOAT);
    KAIZEN_DEFINE_LOGIC_COMPOSITION(and_, and);
    KAIZEN_DEFINE_LOGIC_COMPOSITION(or_, or);
    KAIZEN_DEFINE_LOGIC_COMPOSITION(not_, not);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(greater, >, gt);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(greater_or_equal, >=, gte);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(lower, <, lt);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(lower_or_equal, <=, lte);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(equal, ==, eq);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(not_equal, !=, neq)

    void composer::vm::composer::ternary()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (_stack.size() < 3)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        const auto scd = top();
        pop();
        const auto fst = top();
        pop();
        const auto condition = top();
        pop();
        push(fst->type);
        if (not condition->is("bool"))
            throw exceptions::semantic_error(fmt::format("cannot apply ternary to type \"{}\"", condition->type->name),
                                             _ilc_offset, "please consider using type casting. Eg. bool(x)");
        if (not fst->has_same_type_as(*scd))
            throw exceptions::semantic_error("ternary alternatives differ in type",
                                             _ilc_offset);
        label alt, end;
        code.push_back(go_if_not);
        code.push_back(condition->address(scope->stack_usage));
        code.push_back(0);
        alt.use(code);

        const auto dst = top();
        push(fst);
        assign();
        code.push_back(go);
        code.push_back(0);
        end.use(code);

        alt.bind(code);
        push(dst);
        push(scd);
        assign();
        end.bind(code);

        push(dst);
    }

    void composer::vm::composer::begin_while()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        KAIZEN_IF_SCOPE_OPENING(scope::in_while_prologue);
        label begin;
        begin.bind(code);
        scope->labels.push(begin);
    }

    void composer::vm::composer::set_while_condition()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_while_prologue);
        KAIZEN_IF_SCOPE_OPENING(scope::in_while_body);
        if (_stack.empty())
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        const auto condition = top();
        pop();
        label end;
        code.push_back(go_if_not);
        code.push_back(condition->address(scope->stack_usage));
        code.push_back(0);
        end.use(code);
        scope->labels.push(end);
    }

    void composer::vm::composer::end_while()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_while_body);
        if (scope->labels.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because scope labels size {} is below expected",
                __FUNCTION__,
                scope->labels.size()));
        label end = scope->labels.top();
        scope->labels.pop();
        label begin = scope->labels.top();
        scope->labels.pop();
        KAIZEN_IF_SCOPE_CLOSURE(); // in_while_body
        KAIZEN_IF_SCOPE_CLOSURE(); // in_while_prologue
        code.push_back(go);
        code.push_back(0);
        begin.use(code);
        end.bind(code);
    }

#define KAIZEN_IF_PREINCREMENT_FOR(T, NT)\
if (top()->is(#T))\
{\
    auto it = top();\
    push(it);\
    zen::composer::composer::push<NT>(1, #T);\
    _call_instruction(add_ ## NT, 3,3);\
    push(it);\
    return;\
}


    void composer::vm::composer::pre_increment()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (_stack.empty())
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        KAIZEN_IF_PREINCREMENT_FOR(byte, i8);
        KAIZEN_IF_PREINCREMENT_FOR(short, i16);
        KAIZEN_IF_PREINCREMENT_FOR(int, i32);
        KAIZEN_IF_PREINCREMENT_FOR(long, i64);
        KAIZEN_IF_PREINCREMENT_FOR(float, f32);
        KAIZEN_IF_PREINCREMENT_FOR(double, f64);
        throw exceptions::semantic_error(fmt::format("unsupported operation for type {}", top()->type->name),
                                         _ilc_offset);
    }

#define KAIZEN_IF_PREDECREMENT_FOR(T, NT)\
if (top()->is(#T))\
{\
    auto it = top();\
    push(it);\
    zen::composer::composer::push<NT>(1, #T);\
    _call_instruction(sub_ ## NT, 3,3);\
    push(it);\
    return;\
}


    void composer::vm::composer::pre_decrement()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (_stack.empty())
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        KAIZEN_IF_PREDECREMENT_FOR(byte, i8);
        KAIZEN_IF_PREDECREMENT_FOR(short, i16);
        KAIZEN_IF_PREDECREMENT_FOR(int, i32);
        KAIZEN_IF_PREDECREMENT_FOR(long, i64);
        KAIZEN_IF_PREDECREMENT_FOR(float, f32);
        KAIZEN_IF_PREDECREMENT_FOR(double, f64);
        throw exceptions::semantic_error(fmt::format("unsupported operation for type {}", top()->type->name),
                                         _ilc_offset);
    }

#define KAIZEN_IF_POSTINCREMENT_FOR(T, NT)\
    if (top()->is(#T))\
    {\
        const auto it = top();\
        push(it->type);\
        auto old = top();\
        push(it);\
        assign();\
            \
        push(it);\
        zen::composer::composer::push<NT>(1, #T);\
        _call_instruction(add_##NT, 3, 3);\
        push(it);\
        return;\
    }

    void composer::vm::composer::post_increment()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (_stack.empty())
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        KAIZEN_IF_POSTINCREMENT_FOR(byte, i8);
        KAIZEN_IF_POSTINCREMENT_FOR(short, i16);
        KAIZEN_IF_POSTINCREMENT_FOR(int, i32);
        KAIZEN_IF_POSTINCREMENT_FOR(long, i64);
        KAIZEN_IF_POSTINCREMENT_FOR(float, f32);
        KAIZEN_IF_POSTINCREMENT_FOR(double, f64);
        throw exceptions::semantic_error(fmt::format("unsupported operation for type {}", top()->type->name),
                                         _ilc_offset);
    };

#define KAIZEN_IF_POSTDECREMENT_FOR(T, NT)\
    if (top()->is(#T))\
    {\
        const auto it = top();\
        push(it->type);\
        auto old = top();\
        push(it);\
        assign();\
            \
        push(it);\
        zen::composer::composer::push<NT>(1, #T);\
        _call_instruction(sub_##NT, 3, 3);\
        push(it);\
        return;\
    }

    void composer::vm::composer::post_decrement()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (_stack.empty())
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        KAIZEN_IF_POSTDECREMENT_FOR(byte, i8);
        KAIZEN_IF_POSTDECREMENT_FOR(short, i16);
        KAIZEN_IF_POSTDECREMENT_FOR(int, i32);
        KAIZEN_IF_POSTDECREMENT_FOR(long, i64);
        KAIZEN_IF_POSTDECREMENT_FOR(float, f32);
        KAIZEN_IF_POSTDECREMENT_FOR(double, f64);
        throw exceptions::semantic_error(fmt::format("unsupported operation for type {}", top()->type->name),
                                         _ilc_offset);
    }

    void composer::vm::composer::begin_for()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        KAIZEN_IF_SCOPE_OPENING(scope::in_for);
    }

    void composer::vm::composer::set_for_iterator()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_for);
        throw exceptions::semantic_error("iterator based for is not supported yet", _ilc_offset);
    }

    void composer::vm::composer::set_for_begin_end()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_for);
        if (_stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        const auto last = top();
        pop();
        const auto first = top();
        pop();
        const auto iterator = top();
        pop();

        push(iterator);
        push(first);
        assign();

        label begin, end;
        begin.bind(code);
        push(iterator);
        post_increment();
        push(last);
        lower_or_equal();
        code.push_back(go_if_not);
        code.push_back(top()->address(scope->stack_usage));
        pop();
        code.push_back(0);
        end.use(code);

        scope->labels.push(begin);
        scope->labels.push(end);
        scope->current<for_scope>(scope::in_for)->nested_iterators_count++;
    }

    void composer::vm::composer::set_for_begin_end_step()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_for);
        if (_stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        const auto step = top();
        pop();
        const auto last = top();
        pop();
        const auto first = top();
        pop();
        const auto iterator = top();
        pop();

        push(iterator);
        push(first);
        assign();

        label begin, end;
        begin.bind(code);
        push(iterator);
        push(iterator);
        push(step);
        plus();
        push(last);
        lower_or_equal();
        code.push_back(go_if_not);
        code.push_back(top()->address(scope->stack_usage));
        pop();
        code.push_back(0);
        end.use(code);

        scope->labels.push(begin);
        scope->labels.push(end);
        scope->current<for_scope>(scope::in_for)->nested_iterators_count++;
    };

#define KAIZEN_CASTER_MAP_FOR(T)\
        {\
        {"byte", i8_to_##T},\
        {"bool", i8_to_##T},\
        {"short", i16_to_##T},\
        {"int", i32_to_##T},\
        {"long", i64_to_##T},\
        {"float", f32_to_##T},\
        {"double", f64_to_##T},\
        }

    std::shared_ptr<composer::value> composer::vm::composer::_push_callee_return_value(
        const signature& sig, const call_result& mode)
    {
        if (sig.type->get_size())
        {
            if (mode == assignment)
            {
                if (_stack.empty())
                    throw exceptions::semantic_error("cannot proceed assignment call without stack elements to",
                                                     _ilc_offset);
                fmt::println("assignment call in {} as {} = ...()", scope->name, top()->label);
                const auto r = top();
                if (r->type != sig.type)
                    throw exceptions::semantic_error(
                        fmt::format("cannot assign {} to {}", sig.type->name, r->type->name),
                        _ilc_offset);
                if (r->type->kind == type::heap)
                {
                    fmt::println("assigment call succeeded");
                    code.push_back(push_i64);
                    code.push_back(r->address(scope->stack_usage));
                    scope->stack_usage += sig.type->get_size();
                    pop();
                    return r;
                }
                fmt::println("assigment call failed");
                pop();
            }
            else if (mode == pushed_from_constructor)
            {
                auto result = top();
                pop();
                return result;
            }
            else if (mode == pushed)
            {
                if (sig.type->kind == type::heap)
                // this will allocate data when no assignment is present in a call to a method that returns an object
                {
                    /**
                        TODO: something i can to do improve this is to keep track of all [rr]'s in order to reuse them in a next call of similar type.
                        -> But thats tricky... what if a nested call occurs? Data corruption for sure.
                            -> Maybe if i verify if the [rr] whats already used before trying to utilize it again.
                                -> ok, that should do it.
                    */
                    set_local("[rr]", sig.type->name);
                    push("[rr]");
                    return _push_callee_return_value(sig, assignment);
                }
                else
                {
                    const i64 address = scope->stack_usage;
                    code.push_back(zen::most);
                    code.push_back(-sig.type->get_size());
                    scope->stack_usage += sig.type->get_size();
                    return std::make_shared<value>(sig.type, address, value::temporary);
                }
            }
        }
        return nullptr;
    }

    void composer::vm::composer::_push_callee_arguments(const std::deque<std::shared_ptr<value>>& arguments)
    {
        for (const auto& value : arguments)
        {
            if (value->type->kind == type::kind::heap and value->kind == value::kind::constant)
            {
                /// TODO: optimize using the same logic as [rr]
                set_local("[cha]", value->type->name);
                push("[cha]");
                push(value);
                value->kind = value::kind::fake_constant; // fake it to avoid infinit recursion
                call("zenCopy", 2, call_result::pushed);
                value->kind = value::kind::constant;
                push("[cha]");
                code.push_back(zen::push_i64);
                code.push_back(top()->address(scope->stack_usage));
                pop();
            }
            else
            {
                if (value->is("byte"))
                    code.push_back(zen::push_i8);
                else if (value->is("bool"))
                    code.push_back(zen::push_boolean);
                else if (value->is("short"))
                    code.push_back(zen::push_i16);
                else if (value->is("int"))
                    code.push_back(zen::push_i32);
                else if (value->is("float"))
                    code.push_back(zen::push_f32);
                else if (value->is("double"))
                    code.push_back(zen::push_f64);
                else // if (value.is("long") or true)
                    code.push_back(zen::push_i64);
                code.push_back(value->address(scope->stack_usage));
            }
        }
    }

    composer::call_result composer::vm::composer::_call_caster(const std::string& name, const i8& args_count,
                                                               const std::unordered_map<
                                                                   std::string, std::unordered_map<
                                                                       std::string, i64>>::iterator& caster_set,
                                                               const call_result& mode)
    {
        if (_stack.empty())
        {
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__, _stack.size()));
        }

        if (args_count != 1)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "wrong number of arguments for caster of {}", name), _ilc_offset,
                                             fmt::format(
                                                 "please consider using multiple instructions when converting multiple values.\nEg. v = {0}(w) x = {0}(y)",
                                                 name));
        }
        const auto rhs = top();
        pop();
        if (not mode)
            push(get_type(name));
        else if (_stack.empty())
            throw exceptions::semantic_error(fmt::format(
                                                 "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                                                 __FUNCTION__, _stack.size()), _ilc_offset);
        const auto lhs = top();
        pop();
        if (lhs->is(name))
        {
            if (const auto caster = caster_set->second.find(rhs->type->name); caster != caster_set->second.end())
            {
                code.push_back(caster->second);
                code.push_back(lhs->address(scope->stack_usage));
                code.push_back(rhs->address(scope->stack_usage));
                if (not mode)
                {
                    push(lhs);
                }
                return call_result::assignment;
            }
            else
            {
                throw exceptions::semantic_error(fmt::format(
                                                     "cannot cast from {} to {}", rhs->type->name, name),
                                                 _ilc_offset);
            }
        }
        else
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot cast from {} to {} using {} caster", rhs->type->name,
                                                 lhs->type->name, name), _ilc_offset,
                                             fmt::format("please consider using a {} caster if applicable",
                                                         lhs->type->name));
        }
    }

    composer::call_result composer::vm::composer::_call_function_overload(
        const std::deque<std::shared_ptr<value>>& arguments, function& func,
        const call_result& mode)
    {
        const auto returned = _push_callee_return_value(func.signature, mode);
        // }
        _push_callee_arguments(arguments);

        if (returned)
        {
            push(returned);
        }
        code.push_back(zen::call);
        code.push_back(func.address);
        if (not func.address)
        {
            label lab;
            lab.use(code);
            func.labels.push_back(lab);
        }
        if (const i64 call_params_cost = get_parameters_size(func.signature))
        {
            code.push_back(zen::most);
            code.push_back(call_params_cost);
        }
        return mode ? call_result::assignment : call_result::pushed;
    }

    composer::call_result composer::vm::composer::_call_function(const std::string& name, const i8& args_count,
                                                                 const std::unordered_map<std::string, std::list<function>>::iterator& func_it, const call_result& mode)
    {
        std::deque<std::shared_ptr<value>> arguments;
        for (int i = 0; i < args_count; i++)
        {
            arguments.push_front(top());
            pop();
        }

        std::optional<std::reference_wrapper<function>> candidate;
        for (auto& overload : func_it->second)
        {
            if (overload.signature.parameters.size() != arguments.size()) continue;
            if (arguments.empty())
            {
                candidate = overload;
                break;
            }
            for (int i = 0; i < arguments.size(); i++)
            {
                if (overload.signature.parameters.at(i) != arguments.at(i)->type)
                    break;
                if (i + 1 == arguments.size())
                    candidate = overload;
            }
            if (candidate)
                break;
        }

        if (not candidate)
            throw exceptions::semantic_error(fmt::format("no function overload matched for \'{}\'", name), _ilc_offset);
        // if (args_count < 0) // when assigning, copy the value directly to lhs
        // {
        return _call_function_overload(arguments, candidate->get(), mode);
    }

    composer::call_result composer::vm::composer::_call_instruction_write_str(
        const std::string& name, const i8& args_count)
    {
        if (_stack.size() < 3 or args_count != 3)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "wrong number of arguments for <native>\"{}\" 3 expected", name),
                                             _ilc_offset);
        }
        std::stack<std::shared_ptr<value>> args;
        for (int i = 0; i < args_count; i++)
        {
            auto v = top();
            pop();
            // v.prepare(code, scope->stack_usage);
            args.push(v);
        }
        code.push_back(zen::write_str);
        for (int i = 0; i < args_count; i++)
        {
            code.push_back(args.top()->address(scope->stack_usage));
            args.pop();
        }
        return call_result::pushed;
    }

    composer::call_result composer::vm::composer::_call_instruction(const zen::instruction& name, const i8& args_count,
                                                                    const i8& expected_args_count)
    {
        if (_stack.size() < expected_args_count or args_count != expected_args_count)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "wrong number of arguments for <native>\"{}\" ({} instead of {})",
                                                 static_cast<i32>(name), args_count, expected_args_count), _ilc_offset);
        }
        std::stack<std::shared_ptr<value>> args;
        for (int i = 0; i < args_count; i++)
        {
            auto v = top();
            pop();
            args.push(v);
        }
        code.push_back(name);
        for (int i = 0; i < args_count; i++)
        {
            code.push_back(args.top()->address(scope->stack_usage));
            args.pop();
        }
        return call_result::pushed;
    }

    void composer::vm::composer::end_for()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_for);
        const auto current = scope->current<for_scope>(scope::in_for);
        if (current->nested_iterators_count * 2 < scope->labels.size())
        {
            throw exceptions::semantic_error("cannot finish inconsistent for loop.", _ilc_offset);
        }
        for (int i = 0; i < current->nested_iterators_count; i++)
        {
            label end = scope->labels.top();
            scope->labels.pop();
            label begin = scope->labels.top();
            scope->labels.pop();
            code.push_back(go);
            code.push_back(0);
            begin.use(code);
            end.bind(code);
        }
    }

    void composer::vm::composer::using_(const std::string& alias_function, const std::string& original_function)
    {
        KAIZEN_REQUIRE_GLOBAL_SCOPE();
        if (const auto function = functions.find(original_function); function != functions.end())
            functions[alias_function] = functions[original_function];
        else
            throw exceptions::semantic_error(fmt::format("no such function {}", original_function), _ilc_offset);
    }

    void composer::vm::composer::_link_string_constructor()
    {
        std::optional<std::reference_wrapper<function>> undefined_version;
        auto & zen_constructors = functions.at("[zenConstructor]");
        for (auto & zen_constructor : zen_constructors)
        {
            if (zen_constructor.signature.type->name == "string" and not zen_constructor.defined)
                undefined_version = zen_constructor;
        }

        if (not undefined_version)
            return;

        begin("[zenConstructor]");
        undefined_version->get().address = scope->definition.get().address;
        for (auto& label : undefined_version->get().labels)
            label.bind(code);
        set_return_type("string");
        push("<return>");
        zen::composer::composer::push<i64>(get_type("string")->get_full_size(), "long");
        call(std::to_string(zen::allocate), 2, zen::composer::call_result::pushed);

        push("<return>.len");
        zen::composer::composer::push<i64>(0, "long");
        assign();

        push("<return>.data");
        zen::composer::composer::push<string*>(string::empty(), "string");
        zen::composer::composer::push<i64>(1, "long");
        call(std::to_string(zen::copy), 3, zen::composer::call_result::pushed);

        assume_returned();
        end();
    }

    void composer::vm::composer::_link_string_destructor()
    {
        std::optional<std::reference_wrapper<function>> undefined_version;
        auto & zen_destructors = functions.at("[zenDestructor]");
        for (auto & zen_destructor : zen_destructors)
        {
            if (zen_destructor.signature.parameters.size() == 1 and zen_destructor.signature.parameters.at(0)->name == "string" and not zen_destructor.defined)
                undefined_version = zen_destructor;
        }
        if (not undefined_version)
            return;

        begin("[zenDestructor]");
        undefined_version->get().address = scope->definition.get().address;
        for (auto& label : undefined_version->get().labels)
            label.bind(code);
        set_parameter("it", "string");
        push("it.data");
        push("bool");
        call("bool", 1, zen::composer::call_result::pushed);
        begin_if_then();
        push("it.data");
        call(std::to_string(zen::deallocate), 1, zen::composer::call_result::pushed);
        end_if();
        push("it");
        call(std::to_string(zen::deallocate), 1, zen::composer::call_result::pushed);
        end();
    }

    void composer::vm::composer::_link_string_copy()
    {
        std::optional<std::reference_wrapper<function>> undefined_version;
        auto & overloads = functions.at("zenCopy");
        for (auto & overload : overloads)
        {
            if (overload.signature.parameters.size() == 2 and overload.signature.parameters.at(0)->name == "string" and overload.signature.parameters.at(1)->name == "string" and not overload.defined)
                undefined_version = overload;
        }
        if (not undefined_version)
            return;

        begin("zenCopy");
        undefined_version->get().address = scope->definition.get().address;
        for (auto& label : undefined_version->get().labels)
            label.bind(code);
        set_parameter("to", "string");
        set_parameter("from", "string");

        push("to.data");
        push("bool");
        call("bool", 1, zen::composer::call_result::pushed);
        begin_if_then();
        push("to.data");
        call(std::to_string(zen::deallocate), 1, zen::composer::call_result::pushed);
        end_if();

        set_local("data", "long");
        push("data");
        push("from.len");
        call(std::to_string(zen::allocate), 2, zen::composer::call_result::pushed);

        push("data");
        push("from.data");
        push("from.len");
        call(std::to_string(zen::copy), 3, zen::composer::call_result::pushed);

        push("to.data");
        push("data");
        assign();

        push("to.len");
        push("from.len");
        assign();
        end();
    }

    void composer::vm::composer::link()
    {
        KAIZEN_REQUIRE_GLOBAL_SCOPE();
        _link_string_constructor();
        _link_string_destructor();
        _link_string_copy();
    }

    void composer::vm::composer::push(const std::shared_ptr<const type>& type)
    {
        _stack.emplace(std::make_shared<value>(type, scope->stack_usage, value::temporary));
        code.push_back(most);
        code.push_back(-type->get_size());
        scope->stack_usage += type->get_size();
    }

    composer::call_result composer::vm::composer::call(const std::string& name, const i8& args_count,
                                                       const call_result& mode)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        static std::unordered_map<std::string, std::unordered_map<std::string, i64>> casters{
            {"bool", KAIZEN_CASTER_MAP_FOR(i8)},
            {"byte", KAIZEN_CASTER_MAP_FOR(i8)},
            {"short", KAIZEN_CASTER_MAP_FOR(i16)},
            {"int", KAIZEN_CASTER_MAP_FOR(i32)},
            {"long", KAIZEN_CASTER_MAP_FOR(i64)},
            {"float", KAIZEN_CASTER_MAP_FOR(f32)},
            {"double", KAIZEN_CASTER_MAP_FOR(f64)}
        };
        if (top()->is("function"))
            pop(); // pop the caster itself from composer's stack
        if (const auto caster_set = casters.find(name); caster_set != casters.end())
        {
            return _call_caster(name, args_count, caster_set, mode);
        }

        if (const auto func_it = functions.find(name); func_it != functions.end())
        {
            return _call_function(name, args_count, func_it, mode);
        }
        const int name_i32 = strtol(name.c_str(), nullptr, 10);
        if (name_i32 == write_str)
            return _call_instruction_write_str(name, args_count);
        if (name_i32 >= write_i8 and name_i32 <= write_f64)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 1);
        if (name_i32 == allocate || name_i32 == i64_to_i64)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 2);
        if (name_i32 == deallocate)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 1);
        if (name_i32 == copy)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 3);
        throw exceptions::semantic_error(fmt::format("function \"{}\" was not found", name), _ilc_offset);
    }

    i64 composer::vm::composer::get_parameters_size(const signature& sig)
    {
        i64 size = 0;
        for (const auto& param : sig.parameters)
        {
            size += param->get_size();
        }
        return size;
    }

    i64 composer::vm::composer::get_return_size(const signature& sig)
    {
        return sig.type->get_size();
    }

    void composer::vm::composer::pop()
    {
        _stack.pop();
    }

    void composer::vm::composer::begin_if_then()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        KAIZEN_IF_SCOPE_OPENING(scope::in_if);
        _begin_if_then(false);
    }

    void composer::vm::composer::_begin_if_then(const bool nested)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_if);
        if (_stack.empty())
        {
            throw exceptions::semantic_error(fmt::format(R"(cannot use 'if' or 'else if' without providing a value)"),
                                             _ilc_offset);
        }
        code.push_back(zen::instruction::go_if_not);
        const auto condition = top();
        pop();
        if (not condition->is("bool"))
        {
            throw exceptions::semantic_error("if condition must be bool", _ilc_offset,
                                             "please use type casting if applicable.");
        }
        label else_label;
        if (not nested)
            scope->labels.emplace();
        else
        {
            else_label = scope->labels.top();
            scope->labels.pop();
        }

        code.push_back(condition->address(scope->stack_usage));
        code.push_back(0);
        else_label.use(code);

        scope->labels.push(else_label);
    }

    void composer::vm::composer::else_if_then()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_if);
        else_then();
        _begin_if_then(true);
    }

    void composer::vm::composer::else_then()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_if);
        KAIZEN_IF_SCOPE_CLOSURE();
        KAIZEN_IF_SCOPE_OPENING(scope::in_else);
        if (scope->labels.size() < 2)
            throw exceptions::semantic_error(fmt::format("cannot use else if without a prior if"), _ilc_offset);

        // if (scope->return_status != first_branched_return)
        // scope->return_status = no_return;

        label else_label = scope->labels.top();
        scope->labels.pop();

        label& end_label = scope->labels.top();
        code.push_back(zen::instruction::go);
        code.push_back(0);
        end_label.use(code);
        else_label.bind(code);

        scope->labels.emplace();
    }

    void composer::vm::composer::end_if()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_if);
        KAIZEN_IF_SCOPE_CLOSURE();

        if (scope->labels.size() < 2)
            throw exceptions::semantic_error(fmt::format("cannot end if without a prior if"), _ilc_offset);

        label else_label = scope->labels.top();
        scope->labels.pop();
        label end_label = scope->labels.top();
        scope->labels.pop();

        else_label.bind(code);
        end_label.bind(code);
    }
} // zen
