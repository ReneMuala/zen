//
// Created by dte on 8/31/2025.
//

#include "composer.hpp"

#include <iostream>
#include <queue>
#include <ranges>
#include <sstream>

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
                        /*fmt::println("[zenDestructor]({}):{} in {}", local->label, __LINE__, scope->name);*/\
                        push(local);\
                        call("[zenDestructor]",1);\
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

#define KAIZEN_IF_SCOPE_CLOSURE_NON_DESTRUCTIVE()\
    KAIZEN_IF_SCOPE_DESTROY_HEAP_LOCALS()\
    if (const i64 size = scope->__dncd__peek(scope->return_status); std::abs(size) > 0)\
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
        class_.reset();

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
        string_type->add_field("[len]", get_type("long"), 0);
        string_type->add_field("[data]", get_type("long"), 0);
        functions = {
            {"[zenConstructor]", {{signature{string_type, {get_type("bool")}}, 0, false}}},
            {"operator=", {{signature{unit_type, {string_type, string_type}}, 0, false}}},
            {"operator+", {{signature{string_type, {string_type, string_type}}, 0, false}}},
            {"operator==", {{signature{get_type("bool"), {string_type, string_type}}, 0, false}}},
            {"operator!=", {{signature{get_type("bool"), {string_type, string_type}}, 0, false}}},
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
        const i64 address = scope->get_stack_usage() - /* jump callee IP */static_cast<i64>(sizeof(i64));
        scope->use_stack(t->get_size());
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
        if (scope->get_stack_usage())
        {
            throw std::logic_error("return type cannot be set after params");
        }
        const auto type = get_type(name);
        if (type->get_size() != 0)
        {
            scope->return_data.value = (
                std::make_shared<value>(
                    type, scope->get_stack_usage() - /* jump callee IP */static_cast<i64>(sizeof(i64))));
            scope->use_stack(type->get_size());
        }
        scope->definition.get().signature.type = type;
    }

    void composer::vm::composer::return_value()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (scope->get_return_status() == block_scope::concise_return)
            throw exceptions::semantic_error("cannot return values more than once", _ilc_offset);
        if (scope->get_return_status() == block_scope::branched_return and not scope->is(scope::in_else))
            throw exceptions::semantic_error("conflicting returns", _ilc_offset,
                                             "both the 'if' block and the code after it return values\n\tuse 'else' to make return paths mutually exclusive");
        if (scope->in_loop())
        {
            throw exceptions::semantic_error("cannot return inside loop", _ilc_offset,
                                             "return value will be overwritten on each iteration\n\trestructure to return after the loop completes");
        }
        scope->set_return_status(block_scope::concise_return);
        if (not scope->return_data.value->is("unit"))
        {
            const auto rhs = pop_operand();
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

    void composer::vm::composer::construct(const std::shared_ptr<const zen::composer::type>& t, bool allocate)
    {
        if (const auto func_it = functions.find("[zenConstructor]"); func_it != functions.end())
        {
            for (auto& zen_allocator : func_it->second)
            {
                if (zen_allocator.signature.type == t)
                {
                    zen::composer::composer::push<zen::boolean>(allocate, "bool");
                    auto arg0 = top();
                    pop();
                    _call_function_overload({arg0}, zen_allocator, true); // , call_result::pushed_from_constructor;
                    return;
                }
            }
            throw zen::exceptions::semantic_error(fmt::format("no constructor found for type {}", t->name),
                                                  _ilc_offset);
        }
        else
        {
            throw exceptions::semantic_error(fmt::format("missing construtor implementation for type {}", t->name),
                                             _ilc_offset);
        }
    }

    void composer::vm::composer::set_local(std::string name, const std::string& type)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        std::string no_constructor = "::noConstructor";
        std::shared_ptr<zen::composer::type> t;
        const bool no_construtor_mode = type.ends_with(no_constructor);
        if (no_construtor_mode)
            t = get_type(type.substr(0, type.length() - no_constructor.size()));
        else
            t = get_type(type);
        scope->set_local(name, t, scope->get_stack_usage());
        if (t->get_size() != 0)
        {
            code.push_back(zen::most);
            code.push_back(-t->get_size());
            if (t->kind == type::heap and not no_construtor_mode)
            {
                construct(t, true);
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
        const auto& sig = scope->definition.get().signature;
        const auto sizes = get_return_size(sig) + get_parameters_size(sig);
        if (const auto most_delta = scope->get_stack_usage() - sizes; most_delta > 0)
        {
            code.push_back(zen::most);
            code.push_back(most_delta);
        }
        code.push_back(zen::ret);
        for (auto & f : functions.at(scope->name))
        {
            if (not f.defined && f.signature == scope->definition.get().signature)
            {
                for (auto & l : f.labels)
                {
                    l.bind(code, scope->definition.get().address);
                }
                f.address = scope->definition.get().address;
                f.labels.clear();
                f.defined = true;
                break;
            }
        }
        scope = nullptr;
        while (not _stack.empty())
            _stack.pop();
    }

    std::shared_ptr<composer::type>& composer::vm::composer::get_type(const std::string& name)
    {
        if (types.contains(name))
        {
            return types.at(name);
        }
        throw exceptions::semantic_error(fmt::format(
                                             "no such type \"{}\"", name), _ilc_offset);
    }

    void composer::vm::composer::begin_type(std::shared_ptr<type>& type)
    {
        if (types.contains(type->name))
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "type redefinition \"{}\"", type->name), _ilc_offset);
        }
        types.emplace(type->name, type);
        class_ = type;

        // declare auto generated functions
        functions["operator="].emplace_back(signature{get_type("unit"), {class_, class_}}, 0, false);
        functions["operator=="].emplace_back(signature{get_type("bool"), {class_, class_}}, 0, false);
        functions["operator!="].emplace_back(signature{get_type("bool"), {class_, class_}}, 0, false);
        functions["[zenDestructor]"].emplace_back(signature{get_type("unit"), {class_}}, 0, false);
        functions["[zenConstructor]"].emplace_back(signature{class_, {get_type("bool")}}, 0, false);
    }

    void composer::vm::composer::end_type()
    {
        auto type = class_;
        begin("[zenConstructor]");
        set_return_type(type->name);
        set_parameter("allocate", "bool");
        push("allocate");
        begin_if_then();
        push("<return>");
        auto return_ = top();
        zen::composer::composer::push<i64>(type->get_full_size(), "long");
        call(std::to_string(zen::allocate), 2);
        end_if();
        /*
        i64 offset = 0;
        for (const auto& field : type->fields)
        {
            if (field.second->kind == type::kind::heap)
            {
                set_local("pointer", "long");
                push("pointer");
                code.push_back(zen::add_i64);
                code.push_back(top()->address(scope->get_stack_usage()));
                code.push_back(return_->address(scope->get_stack_usage()));
                code.push_back((i64)_pool.get<i64>(offset).get());
                pop();
                // construct(field.second, false); // right now this is useless as nested objects will always do the same (allocate memory and initialize fields with zero)
            }
            offset += field.second->get_full_size();
        }*/
        assume_returned();
        end();

        begin("[zenDestructor]");
        set_parameter("it", type->name);
        push("it");
        call(std::to_string(zen::deallocate), 1);
        end();

        begin("operator=");
        set_parameter("a", type->name);
        set_parameter("b", type->name);
        for (const auto& field : type->fields)
        {
            push("a." + field.first);
            push("b." + field.first);
            assign();
        }
        end();

        begin("operator==");
        set_return_type("bool");
        set_parameter("a", type->name);
        set_parameter("b", type->name);
        push("<return>");
        zen::composer::composer::push<boolean>(true, "bool");
        assign();
        for (const auto& field : type->fields)
        {
            push("<return>");
            push("<return>");
            push("a." + field.first);
            push("b." + field.first);
            equal();
            and_();
            assign();
        }
        assume_returned();
        end();

        begin("operator!=");
        set_return_type("bool");
        set_parameter("a", type->name);
        set_parameter("b", type->name);
        push("a");
        push("b");
        call("operator==", 2);
        not_();
        return_value();
        end();
        class_.reset();
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
                fmt::println("{}({}) = {} [{}]", function.first,
                             params.empty() ? "" : params.substr(0, params.length() - 2),
                             overload.signature.type ? overload.signature.type->name : "*", overload.address);
                for (i64 i = overload.address; i < code.size(); i++)
                {
                    auto _i = i;
                    fmt::print("\t");
                    const auto& _code = code[i];
                    if (_code == most)
                    {
                        fmt::print("most, {}, ", code[i + 1]);
                        i += 1;
                    }
                    else if (_code == zen::refer)
                    {
                        fmt::print("refer, {}, {} ", code[i + 1], code[i + 2]);
                        i += 2;
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
                    else if (_code == mul_f32 || _code == mul_i32 || _code == mul_i64 || _code == mul_f64 || _code ==
                        mul_i8 || _code ==
                        mul_i16)
                    {
                        fmt::print("mul, {}, {}, {}, ", code[i + 1], code[i + 2], code[i + 3]);
                        i += 3;
                    }
                    else if (_code == div_f32 || _code == div_i32 || _code == div_i64 || _code == div_f64 || _code ==
                        div_i8 || _code ==
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
                    else if (_code == boolean_not)
                    {
                        fmt::print("not, {}, {}, ", code[i + 1], code[i + 2]);
                        i += 2;
                    }
                    else if (_code == eq_str)
                    {
                        fmt::print("eq_str, {}, {}, ", code[i + 1], code[i + 2]);
                        i += 2;
                    }
                    else if (_code == ret)
                        fmt::print("ret, ");
                    else if (_code == hlt)
                        fmt::print("hlt, ");
                    else if (_code == placeholder)
                        fmt::print("placeholder, ");
                    else
                        std::cout << _code << ' ';
                    if (_code == zen::ret || _code == zen::hlt) break;
                    fmt::println("[{}] ", _i);
                }
                std::cout << std::endl;
            }
        }
    }

    void composer::vm::composer::assign()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (not _stack.empty() and _stack.top()->is("unit"))
        {
            pop();
            return;
        }
        if (_stack.size() < 2)
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "<KAIZEN-INTERNAL-API> Cannot compose operation {} because stack size {} is below expected",
                                                 __FUNCTION__, _stack.size()), _ilc_offset);
        }
        auto rhs = pop_operand(false);
        auto lhs = pop_operand(false);
        if (lhs->is("unit"))
        {
            throw exceptions::semantic_error("cannot return values from unit function", _ilc_offset,
                                             fmt::format("please consider changing the return type of \"{}\" to {}",
                                                         scope->name, rhs->type->name));
        }
        if (lhs->kind == value::constant)
        {
            throw exceptions::semantic_error("cannot assign value to constant", _ilc_offset,
                                             rhs->kind == value::variable
                                                 ? "please consider changing operands order from x = y to y = x if applicable"
                                                 : "");
        }
        if (not rhs->has_same_type_as(*lhs))
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot assign {} to {}", rhs->type->name,
                                                 lhs->type->name), _ilc_offset,
                                             fmt::format(
                                                 "please consider using type casting. Eg. x = {}(y) or define a custom copy operator",
                                                 lhs->type->name, rhs->type->name));
        }
        const bool dereference_assignment = lhs->is_reference or rhs->is_reference;

        if (lhs->type->kind == type::kind::heap and lhs->no_destructor)
        {
            fmt::println("no_destructor being assigned {}, shadowing parameter", lhs->label);
            set_local(lhs->label, lhs->type->name);
            lhs = scope->get_local(lhs->label);
        }
        if (lhs->type->kind == type::kind::stack)
        {
            if (dereference_assignment)
            {
                if (not lhs->is_reference)
                {
                    push("<long>");
                    const auto new_lhs = top();
                    assign_reference(new_lhs, lhs);
                    lhs = new_lhs;
                }
                if (not rhs->is_reference)
                {
                    push("<long>");
                    const auto new_rhs = top();
                    assign_reference(new_rhs, rhs);
                    rhs = new_rhs;
                }
            }
            if (dereference_assignment)
                code.push_back(copy);
            else if (rhs->is("short"))
                code.push_back(zen::i16_to_i16);
            else if (rhs->is("int"))
                code.push_back(zen::i32_to_i32);
            else if (rhs->is("long"))
                code.push_back(zen::i64_to_i64);
            else if (lhs->is("float"))
                code.push_back(zen::f32_to_f32);
            else if (lhs->is("double"))
                code.push_back(zen::f64_to_f64);
            else if ((lhs->is("byte") or lhs->is("bool")))
                code.push_back(zen::i8_to_i8);
            else
                throw exceptions::semantic_error(
                    fmt::format("stack type {} is not supported for assignment", lhs->type->name), _ilc_offset);
            code.push_back(lhs->address(scope->get_stack_usage()));
            code.push_back(rhs->address(scope->get_stack_usage()));
            if (dereference_assignment)
            {
                code.push_back((i64)_pool.get<i64>(lhs->type->get_size()).get());
            }
        }
        else
        {
            push(lhs);
            push(rhs);
            call("operator=", 2);
        }
    }

    void composer::vm::composer::_report_field_not_found(const std::string& name, const std::shared_ptr<type>& type)
    {
        std::string hint;
        for (const auto& field : type->fields)
        {
            if (not field.first.starts_with('['))
            {
                hint += fmt::format("\n\t- {}:{}", field.first, field.second->name);
            }
        }
        const auto prefix = type->name + ".";
        for (const auto& function : functions)
        {
            if (function.first.starts_with(prefix))
            {
                hint += fmt::format("\n\t- {}(...)", function.first.substr((prefix.length())));
            }
        }

        if (not hint.empty())
        {
            hint = "options:" + hint + ",";
        }
        throw exceptions::semantic_error(fmt::format("no such field {} in type {}", name, type->name), _ilc_offset,
                                         hint);
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
        if (tokens.size() > 1)
        {
            std::shared_ptr<value> val = std::make_shared<value>(*location);
            std::pair<i64, std::shared_ptr<type>> item = {0, val->type};
            for (int i = 1; i < tokens.size(); ++i)
            {
                /*
                * if (not hint.empty())
                {
                    hint = "\n\toptions:" + hint + ",";
                }
                throw exceptions::semantic_error(fmt::format("no such field {} in type {}{}", name, this->name, hint), ilc_offset);

                 */
                if (const auto field = item.second->get_field(tokens[i]))
                {
                    item.first += field->first;
                    item.second = field->second;
                }
                else
                {
                    _report_field_not_found(tokens[i], item.second);
                }
            }
            val->offset = item.first;
            val->type = item.second;

            push(get_type("long"));
            top()->type = val->type;
            top()->is_reference = true;
            code.push_back(zen::add_i64);
            code.push_back(top()->address(scope->get_stack_usage()));
            code.push_back(location->address(scope->get_stack_usage()));
            code.push_back((i64)(_pool.get<i64>(val->offset).get()));
        }
        else
            push(location);
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
            throw exceptions::semantic_error(fmt::format("invalid name '{}' in function {}", name, scope->name),
                                             _ilc_offset);
        if (const auto location = scope->get_local(tokens.front()))
            _push_variable(tokens, location);
        else if (name.starts_with("<return>") && scope->return_data.value)
            _push_variable(tokens, scope->return_data.value);
        else if (name.starts_with("<") and name.ends_with(">"))
            _push_temporary_value(name);
        else if (class_ and not name.starts_with("this."))
        {
            push("this." + name);
        }
        else
            throw exceptions::semantic_error(fmt::format(
                                                 "no such symbol {}", tokens.front()), _ilc_offset,
                                             fmt::format(
                                                 "please define it in the respective scope. Eg. {0}: T = V // with T & V being it's type and value respectively.",
                                                 tokens.front()));
    }

    void composer::vm::composer::access(const std::string& dot_fields)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (_stack.empty())
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "<KAIZEN-INTERNAL-API> Cannot compose operation {} because stack size {} is below expected",
                                                 __FUNCTION__, _stack.size()), _ilc_offset);
        }
        std::vector<std::string> tokens = split_name(dot_fields);
        if (tokens.size() <= 1)
            throw exceptions::semantic_error(fmt::format("invalid field '{}' in function {}", dot_fields, scope->name),
                                             _ilc_offset);
        const auto location = top();
        pop();
        tokens.erase(tokens.begin(), tokens.begin() + 1);
        _push_variable(tokens, location);
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

#define KAIZEN_DEFINE_ARITH_COMPOSITION(N, A, IP, CG,EOP)\
    void composer::vm::composer::N()\
    {\
        KAIZEN_REQUIRE_SCOPE(scope::in_function);\
        if (_stack.size() < 2)\
            throw std::logic_error(fmt::format(\
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",\
                __FUNCTION__, _stack.size()));\
        if (top()->type->kind == type::kind::stack)\
        {\
            const auto rhs = pop_operand();\
            const auto lhs = pop_operand();\
            if(lhs->has_same_type_as(*rhs)){\
                push(lhs->type);\
                CG(IP)\
                else\
                    throw exceptions::semantic_error(fmt::format(\
                                                         "cannot "#A" type \"{}\"", lhs->type->name), _ilc_offset);\
                code.push_back(top()->address(scope->get_stack_usage()));\
                code.push_back(lhs->address(scope->get_stack_usage()));\
                code.push_back(rhs->address(scope->get_stack_usage()));\
            } else {\
                throw exceptions::semantic_error(fmt::format(\
                                                     "cannot "#A" {} with {}", lhs->type->name,\
                                                     rhs->type->name), _ilc_offset,\
                                                 fmt::format(\
                                                     "please consider using type casting. Eg. x = {}(y) // with y: {}, if applicable.",\
                                                     lhs->type->name, rhs->type->name));\
            }\
        } else {\
                const auto rhs = pop_operand(false);\
                const auto lhs = pop_operand(false);\
                push(lhs);\
                push(rhs);\
                call("operator"#EOP, 2);\
        }\
    }

#define KAIZEN_DEFINE_SINGLE_OP_LOGIC_COMPOSITION(N, I, EOP)\
    void composer::vm::composer::N()\
    {\
        KAIZEN_REQUIRE_SCOPE(scope::in_function);\
        if (_stack.empty())\
            throw std::logic_error(fmt::format(\
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",\
                __FUNCTION__, _stack.size()));\
        if (top()->type->kind == type::kind::stack)\
        {\
            const auto val = pop_operand();\
            if (val->is("bool")) {\
                push("<bool>");\
                code.push_back(zen::boolean_##I);\
            } else\
                throw exceptions::semantic_error(fmt::format(\
                                                         "cannot apply "#I" to type \"{}\"", val->type->name), _ilc_offset, "please consider using type casting. Eg. bool(x)");\
            code.push_back(top()->address(scope->get_stack_usage()));\
            code.push_back(val->address(scope->get_stack_usage()));\
        } else {\
            call("operator"#EOP, 1);\
        }\
    }

#define KAIZEN_DEFINE_DOUBLE_OP_LOGIC_COMPOSITION(N, I, EOP)\
    void composer::vm::composer::N()\
    {\
        KAIZEN_REQUIRE_SCOPE(scope::in_function);\
        if (_stack.empty())\
            throw std::logic_error(fmt::format(\
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",\
                __FUNCTION__, _stack.size()));\
        if (top()->type->kind == type::kind::stack)\
        {\
            const auto rhs = pop_operand();\
            const auto lhs = pop_operand();\
            if (rhs->is("bool")) {\
                push("<bool>");\
                code.push_back(zen::boolean_##I);\
            } else\
                throw exceptions::semantic_error(fmt::format(\
                                                         "cannot apply "#I" to for types \"{}\" and \"{}\"", lhs->type->name, rhs->type->name), _ilc_offset, "please consider using type casting. Eg. bool(x)");\
            code.push_back(top()->address(scope->get_stack_usage()));\
            code.push_back(lhs->address(scope->get_stack_usage()));\
            code.push_back(rhs->address(scope->get_stack_usage()));\
        } else {\
            const auto rhs = pop_operand();\
            const auto lhs = pop_operand();\
            call("operator"#EOP, 1);\
        }\
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

#define KAIZEN_IF_RELATIONAL_CHAIN_EQUALITY(F)\
    KAIZEN_IF_RELATIONAL_CHAIN(F)\
    else if (rhs->is("bool")) \
        code.push_back(zen::F##_i8);

#define KAIZEN_DEFINE_RELATIONAL_COMPOSITION(N, A, I, CH)\
    void composer::vm::composer::N()\
    {\
        KAIZEN_REQUIRE_SCOPE(scope::in_function);\
        if (_stack.size() < 2)\
            throw std::logic_error(fmt::format(\
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",\
                __FUNCTION__, _stack.size()));\
        if(top()->type->kind == type::kind::stack){\
            const auto rhs = pop_operand();\
            const auto lhs = pop_operand();\
            push("<bool>");\
            if (lhs->has_same_type_as(*rhs))\
            {\
                CH(I)\
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
            code.push_back(top()->address(scope->get_stack_usage()));\
            code.push_back(lhs->address(scope->get_stack_usage()));\
            code.push_back(rhs->address(scope->get_stack_usage()));\
        } else {\
            const auto rhs = pop_operand(false);\
            const auto lhs = pop_operand(false);\
            push(lhs);\
            push(rhs);\
            call("operator"#A, 2);\
        }\
    }

    std::shared_ptr<composer::value> composer::vm::composer::pop_operand(const bool allow_dereferencing)
    {
        auto operand = top();
        if (operand->is("unit"))
            throw exceptions::semantic_error("expected value", _ilc_offset);
        pop();
        if (not allow_dereferencing or not operand->is_reference)
            return operand;
        return dereference(operand);
    }

    std::shared_ptr<composer::value> composer::vm::composer::dereference(const std::shared_ptr<value>& value)
    {
        push(value->type);
        const auto destination = top();
        pop();
        push(get_type("long"));
        const auto pointer = top();
        pop();
        assign_reference(pointer, destination);
        code.push_back(zen::copy);
        code.push_back(pointer->address(scope->get_stack_usage()));
        code.push_back(value->address(scope->get_stack_usage()));
        code.push_back((i64)_pool.get<i64>(value->type->get_size()).get());
        scope->use_stack(-pointer->type->get_size());
        code.push_back(most);
        code.push_back(pointer->type->get_size());
        return destination;
    }

    void composer::vm::composer::begin_block()
    {
        fmt::println("scope->depth() {}", scope->depth());
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        KAIZEN_IF_SCOPE_OPENING(scope::in_block);
        fmt::println("scope->depth() {}", scope->depth());
    }

    void composer::vm::composer::end_block()
    {
        fmt::println("1. scope->depth() {}", scope->depth());
        KAIZEN_REQUIRE_SCOPE(scope::in_block);
        KAIZEN_IF_SCOPE_CLOSURE();
        scope->__dncd__pop(scope->return_status);
        fmt::println("2. scope->depth() {}", scope->depth());
    }


    KAIZEN_DEFINE_ARITH_COMPOSITION(plus, sum, add, KAIZEN_IF_ARITHMETICS_CHAIN, +);
    KAIZEN_DEFINE_ARITH_COMPOSITION(minus, subtract, sub, KAIZEN_IF_ARITHMETICS_CHAIN, -);
    KAIZEN_DEFINE_ARITH_COMPOSITION(times, multiply, mul, KAIZEN_IF_ARITHMETICS_CHAIN, *);
    KAIZEN_DEFINE_ARITH_COMPOSITION(slash, divide, div, KAIZEN_IF_ARITHMETICS_CHAIN, /);
    KAIZEN_DEFINE_ARITH_COMPOSITION(modulo, compute modulo, mod, KAIZEN_IF_ARITHMETICS_CHAIN_FLOAT, %);
    KAIZEN_DEFINE_DOUBLE_OP_LOGIC_COMPOSITION(and_, and, &&);
    KAIZEN_DEFINE_DOUBLE_OP_LOGIC_COMPOSITION(or_, or, ||);
    KAIZEN_DEFINE_SINGLE_OP_LOGIC_COMPOSITION(not_, not, !)

#define KAIZEN_IF_NEGATE_CONSTANT_NEGATE_FOR(T, NT)\
    if (top()->is(#T))\
    {\
        auto val = top();\
        pop();\
        const NT* ptr = (NT*)val->address(0);\
        zen::composer::composer::push<NT>(-*ptr, #T);\
        top()->is_negated = true;\
        return;\
    }

#define KAIZEN_IF_NEGATE_FOR(T, NT)\
if (top()->is(#T))\
{\
    auto it = top();\
    it->is_negated = not it->is_negated;\
    if (it->is_reference)\
    {\
        pop();\
        push(it->type);\
        const auto result = top();\
        push(dereference(it));\
        zen::composer::composer::push<NT>(-1, it->type->name);\
        _call_instruction(mul_ ## NT, 3, 3);\
        push(it);\
        push(result);\
        assign();\
    } else\
    {\
        push(it);\
        zen::composer::composer::push<NT>(-1, it->type->name);\
        _call_instruction(mul_ ## NT, 3, 3);\
    }\
    push(it);\
    return;\
}

    void composer::vm::composer::negate()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        if (_stack.empty())
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        if (top()->kind == value::constant)
        {
            KAIZEN_IF_NEGATE_CONSTANT_NEGATE_FOR(byte, i8)
            KAIZEN_IF_NEGATE_CONSTANT_NEGATE_FOR(short, i16)
            KAIZEN_IF_NEGATE_CONSTANT_NEGATE_FOR(int, i32)
            KAIZEN_IF_NEGATE_CONSTANT_NEGATE_FOR(long, i64)
            KAIZEN_IF_NEGATE_CONSTANT_NEGATE_FOR(float, f32)
            KAIZEN_IF_NEGATE_CONSTANT_NEGATE_FOR(double, f64)
        }
        else
        {
            KAIZEN_IF_NEGATE_FOR(byte, i8);
            KAIZEN_IF_NEGATE_FOR(short, i16);
            KAIZEN_IF_NEGATE_FOR(int, i32);
            KAIZEN_IF_NEGATE_FOR(long, i64);
            KAIZEN_IF_NEGATE_FOR(float, f32);
            KAIZEN_IF_NEGATE_FOR(double, f64);
        }
        throw exceptions::semantic_error(fmt::format("unsupported operation for type {}", top()->type->name),
                                         _ilc_offset);
    };

    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(greater, >, gt, KAIZEN_IF_RELATIONAL_CHAIN);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(greater_or_equal, >=, gte, KAIZEN_IF_RELATIONAL_CHAIN);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(lower, <, lt, KAIZEN_IF_RELATIONAL_CHAIN);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(lower_or_equal, <=, lte, KAIZEN_IF_RELATIONAL_CHAIN);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(equal, ==, eq, KAIZEN_IF_RELATIONAL_CHAIN_EQUALITY);
    KAIZEN_DEFINE_RELATIONAL_COMPOSITION(not_equal, !=, neq, KAIZEN_IF_RELATIONAL_CHAIN_EQUALITY);

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
        const auto condition = pop_operand();
        label end;
        code.push_back(go_if_not);
        code.push_back(condition->address(scope->get_stack_usage()));
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
        KAIZEN_IF_SCOPE_CLOSURE_NON_DESTRUCTIVE(); // in_while_prologue, keeping it for cases where condition is false
        code.push_back(go);
        code.push_back(0);
        begin.use(code);
        end.bind(code);
        KAIZEN_IF_SCOPE_CLOSURE(); // in_while_prologue
    }

#define KAIZEN_IF_PREINCREMENT_FOR(T, NT)\
if (top()->is(#T))\
{\
    auto it = top();\
    if (it->is_reference)\
    {\
        pop();\
        push(it->type);\
        const auto result = top();\
        push(dereference(it));\
        zen::composer::composer::push<NT>(1, it->type->name);\
        _call_instruction(add_ ## NT, 3, 3);\
        push(it);\
        push(result);\
        assign();\
        push(result);\
    } else\
    {\
        push(it);\
        zen::composer::composer::push<NT>(1, it->type->name);\
        _call_instruction(add_ ## NT, 3, 3);\
        push(it->type);\
        auto copy = top();\
        push(it);\
        assign();\
        push(copy);\
    }\
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
        if (top()->kind != value::kind::variable)
        {
            throw exceptions::semantic_error("invalid operand for increment", _ilc_offset);
        }
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
    if (it->is_reference)\
    {\
        pop();\
        push(it->type);\
        const auto result = top();\
        push(dereference(it));\
        zen::composer::composer::push<NT>(1, it->type->name);\
        _call_instruction(sub_ ## NT, 3, 3);\
        push(it);\
        push(result);\
        assign();\
        push(result);\
    } else\
    {\
        push(it);\
        zen::composer::composer::push<NT>(1, it->type->name);\
        _call_instruction(sub_ ## NT, 3, 3);\
        push(it->type);\
        auto copy = top();\
        push(it);\
        assign();\
        push(copy);\
    }\
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
        if (top()->kind != value::kind::variable)
        {
            throw exceptions::semantic_error("invalid operand for decrement", _ilc_offset);
        }
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
    pop();\
    push(it->type);\
    auto old = top();\
    push(it);\
    assign();\
    if (it->is_reference)\
    {\
        push(it->type);\
        const auto _new = top();\
        push(dereference(it));\
        zen::composer::composer::push<NT>(1, it->type->name);\
        _call_instruction(add_ ## NT, 3, 3);\
        push(it);\
        push(_new);\
        assign();\
    } else\
    {\
        push(it);\
        push(it);\
        zen::composer::composer::push<NT>(1, it->type->name);\
        _call_instruction(add_ ## NT, 3, 3);\
    }\
    push(old);\
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
        if (top()->kind != value::kind::variable)
        {
            throw exceptions::semantic_error("invalid operand for increment", _ilc_offset);
        }
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
    pop();\
    push(it->type);\
    auto old = top();\
    push(it);\
    assign();\
    if (it->is_reference)\
    {\
        push(it->type);\
        const auto _new = top();\
        push(dereference(it));\
        zen::composer::composer::push<NT>(1, it->type->name);\
        _call_instruction(sub_ ## NT, 3, 3);\
        push(it);\
        push(_new);\
        assign();\
    } else\
    {\
        push(it);\
        push(it);\
        zen::composer::composer::push<NT>(1, it->type->name);\
        _call_instruction(sub_ ## NT, 3, 3);\
    }\
    push(old);\
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
        if (top()->kind != value::kind::variable)
        {
            throw exceptions::semantic_error("invalid operand for decrement", _ilc_offset);
        }
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

#define KAIZEN_IF_INCREMENT_FOR(T, NT)\
     if (top()->is(#T))\
     {\
         const auto it = top();\
         push(it);\
         zen::composer::composer::push<NT>(1, it->type->name);\
         _call_instruction(add_ ## NT, 3, 3);\
         return;\
    }

    void composer::vm::composer::increment_by_one()
    {
        KAIZEN_IF_INCREMENT_FOR(byte, i8);
        KAIZEN_IF_INCREMENT_FOR(short, i16);
        KAIZEN_IF_INCREMENT_FOR(int, i32);
        KAIZEN_IF_INCREMENT_FOR(long, i64);
        KAIZEN_IF_INCREMENT_FOR(float, f32);
        KAIZEN_IF_INCREMENT_FOR(double, f64);
    }

    void composer::vm::composer::set_for_begin_end()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_for);
        if (_stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        KAIZEN_IF_SCOPE_OPENING(scope::in_for); // for iterator related memory elements
        const auto last = pop_operand();
        const auto first = pop_operand();
        const auto iterator = pop_operand();

        push(iterator);
        push(first);
        assign();
        code.push_back(zen::go);
        code.push_back(0);


        label begin, end, condition;
        condition.use(code);
        begin.bind(code);
        push(iterator);
        increment_by_one();
        push(iterator);
        push(last);
        condition.bind(code); // skip increment on the first run
        lower_or_equal();
        code.push_back(go_if_not);
        code.push_back(pop_operand()->address(scope->get_stack_usage()));
        code.push_back(0);
        end.use(code);

        scope->labels.push(begin);
        scope->labels.push(end);
        KAIZEN_IF_SCOPE_OPENING(scope::in_for_body);
    }

    void composer::vm::composer::set_for_begin_end_step()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_for);
        if (_stack.size() < 2)
            throw std::logic_error(fmt::format(
                "[Error: Invalid state] Cannot compose operation {} because stack size {} is below expected",
                __FUNCTION__,
                _stack.size()));
        KAIZEN_IF_SCOPE_OPENING(scope::in_for); // for iterator related memory elements
        const auto step = pop_operand();
        const auto last = pop_operand();
        const auto first = pop_operand();
        const auto iterator = pop_operand();

        push(iterator);
        push(first);
        assign();
        code.push_back(zen::go);
        code.push_back(0);


        label begin, end, condition;
        condition.use(code);
        begin.bind(code);
        KAIZEN_IF_SCOPE_OPENING(scope::in_function); // because of that plus bellow, it keeps pushing data :[
        push(iterator);
        push(iterator);
        push(step);
        plus();
        assign();
        KAIZEN_IF_SCOPE_CLOSURE();

        push(iterator);
        push(last);
        condition.bind(code); // skip increment on the first run
        if (step->is_negated)
            greater_or_equal();
        else
            lower_or_equal();
        code.push_back(go_if_not);
        code.push_back(pop_operand()->address(scope->get_stack_usage()));
        code.push_back(0);
        end.use(code);

        scope->labels.push(begin);
        scope->labels.push(end);
        KAIZEN_IF_SCOPE_OPENING(scope::in_for_body);
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
        {"string", str_to_##T}\
    }

#define KAIZEN_CASTER_MAP_FOR_STRING(T)\
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
        const signature& sig, const bool construtor_call)
    {
        if (sig.type->get_size())
        {
            if (construtor_call)
            {
                return nullptr;
            }
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
                auto rr = top();
                pop();
                code.push_back(push_i64);
                code.push_back(rr->address(scope->get_stack_usage()));
                scope->use_stack(sig.type->get_size());
                return rr;
            }
            else
            {
                const i64 address = scope->get_stack_usage();
                code.push_back(zen::most);
                code.push_back(-sig.type->get_size());
                // scope->get_stack_usage() += sig.type->get_size();
                scope->use_stack(sig.type->get_size());
                return std::make_shared<value>(sig.type, address, value::temporary);
            }
        }
        return nullptr;
    }

    void composer::vm::composer::_push_callee_arguments(const std::deque<std::shared_ptr<value>>& arguments)
    {
        for (const auto& value : arguments)
        {
            if (value->type->kind == type::kind::stack)
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
                else if (value->is("long"))
                    code.push_back(zen::push_i64);
                else
                    throw exceptions::semantic_error("unknown scalar type", _ilc_offset);
                code.push_back(value->address(scope->get_stack_usage()));
                scope->use_stack(value->type->get_size());
            }
            else
            {
                code.push_back(push_i64);
                code.push_back(value->address(scope->get_stack_usage()));
                scope->use_stack(get_type("long")->get_size());
            }
        }
    }

    void composer::vm::composer::_call_caster(const std::string& name, const i8& args_count,
                                              const std::unordered_map<
                                                  std::string, std::unordered_map<
                                                      std::string, i64>>::iterator& caster_set)
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
        const auto rhs = pop_operand();
        auto const type = get_type(name);
        if (type->kind == type::kind::stack)
        {
            push(get_type(name));
        }
        else
        {
            set_local("[cc]", name);
            push("[cc]");
        }
        const auto lhs = pop_operand(false);
        if (const auto caster = caster_set->second.find(rhs->type->name); caster != caster_set->second.end())
        {
            code.push_back(caster->second);
            code.push_back(lhs->address(scope->get_stack_usage()));
            code.push_back(rhs->address(scope->get_stack_usage()));
            push(lhs);
        }
        else
        {
            throw exceptions::semantic_error(fmt::format(
                                                 "cannot cast from {} to {}", rhs->type->name, name),
                                             _ilc_offset);
        }
    }

    bool composer::vm::composer::_call_function_overload(
        const std::deque<std::shared_ptr<value>>& arguments, function& func, const bool construtor_call)
    {
        const auto returned = _push_callee_return_value(func.signature, construtor_call);
        if (arguments.size() != func.signature.parameters.size())
        {
            throw exceptions::semantic_error(fmt::format("<KAIZEN-INTERNAL-API> bad overload calling"), _ilc_offset);
        }
        _push_callee_arguments(arguments);
        const bool returned_result = returned and not construtor_call;
        if (returned_result)
        {
            push(returned);
        }
        code.push_back(zen::call);
        code.push_back(func.address);
        if (not func.defined)
        {
            global_label lab;
            lab.use(code);
            func.labels.push_back(lab);
        }
        if (const i64 call_params_cost = get_parameters_size(func.signature); call_params_cost && not construtor_call)
        {
            code.push_back(zen::most);
            code.push_back(call_params_cost);
            scope->use_stack(-call_params_cost);
        }
        return returned_result;
    }

    bool composer::vm::composer::_call_function(const std::string& name, const i8& args_count,
                                                const std::unordered_map<
                                                    std::string, std::list<function>>::iterator&
                                                func_it, std::shared_ptr<value>& caster_arg_buffer)
    {
        std::deque<std::shared_ptr<value>> arguments;
        std::shared_ptr<value> this_;
        if (func_it->first.contains('.'))
        {
            this_ = pop_operand(false); // this is always an object do we dont need to dereference it
        }
        for (int i = 0; i < args_count; i++)
        {
            auto value = pop_operand(false);
            if (value->type->kind == type::kind::heap and value->kind == value::kind::constant)
            {
                code.push_back(placeholder);
                /// TODO: optimize using the same logic as [rr]
                set_local("[cha]", value->type->name);
                push("[cha]");
                const auto cha = top();
                push("<long>");
                const auto ptr = top();
                assign_reference(ptr, value);
                ptr->type = value->type;
                ptr->is_reference = false;
                call("operator=", 2);
                value = cha;
                code.push_back(placeholder);
            }
            else if (value->type->kind == type::kind::stack and value->is_reference)
            // deference fields only if they are stack allocated since nested objects are stored in a contiguous memory block
            {
                value = dereference(value);
            }
            arguments.push_front(value);
        }
        if (this_)
        {
            arguments.push_front(this_);
        }
        std::optional<std::reference_wrapper<function>> candidate;
        for (auto& overload : std::ranges::reverse_view(func_it->second))
        {
            if (overload.signature.parameters.size() != arguments.size()) continue;
            if (arguments.empty())
            {
                candidate = overload;
                break;
            }
            for (int i = 0; i < arguments.size(); i++)
            {
                if (not(overload.signature.parameters.at(i) == arguments.at(i)->type))
                    break;
                if (i + 1 == arguments.size())
                    candidate = overload;
            }
            if (candidate)
                break;
        }

        /*
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
        */
        if (not candidate)
        {
            if (arguments.size() == 1)
            {
                caster_arg_buffer = arguments.at(0);
            }
            throw exceptions::semantic_error(fmt::format("no function overload matched for \'{}\'", func_it->first),
                                             _ilc_offset);
        }
        return _call_function_overload(arguments, candidate->get(), false);
    }

    bool composer::vm::composer::_call_instruction_write_str(
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
            auto v = pop_operand(false);
            // v.prepare(code, scope->get_stack_usage());
            args.push(v);
        }
        code.push_back(zen::write_str);
        for (int i = 0; i < args_count; i++)
        {
            code.push_back(args.top()->address(scope->get_stack_usage()));
            args.pop();
        }
        return false;
    }

    bool composer::vm::composer::_call_instruction(const zen::instruction& name, const i8& args_count,
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
            auto v = pop_operand();
            args.push(v);
        }
        code.push_back(name);
        for (int i = 0; i < args_count; i++)
        {
            code.push_back(args.top()->address(scope->get_stack_usage()));
            args.pop();
        }
        return false;
    }

    std::optional<std::unordered_map<std::string, std::list<composer::vm::function>>::iterator>
    composer::vm::composer::fetch_function(const std::string& name)
    {
        if (not name.empty() and std::isdigit(name.front()))
        {
            return std::nullopt;
        }
        if (not name.contains('.'))
        {
            if (const auto func_it = functions.find(name); func_it != functions.end())
                return func_it;
            if (class_)
                return fetch_function("this." + name);
        }
        else if (name.length() >= 3)
        {
            const auto object = name.substr(0, name.find_last_of('.'));
            const auto function = name.substr(name.find_last_of('.') + 1);
            push(object);
            std::string method = top()->type->name + "." + function;
            if (const auto func_it = functions.find(method); func_it != functions.end())
            {
                return func_it;
            }
            pop();
        }
        return std::nullopt;
    }

    void composer::vm::composer::end_for()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_for);
        while (scope->is(scope::in_for_body))
        {
            label end = scope->labels.top();
            scope->labels.pop();
            label begin = scope->labels.top();
            scope->labels.pop();
            KAIZEN_IF_SCOPE_CLOSURE();
            KAIZEN_IF_SCOPE_CLOSURE_NON_DESTRUCTIVE(); // iterator related stuff
            code.push_back(go);
            code.push_back(0);
            begin.use(code);
            end.bind(code);
            KAIZEN_IF_SCOPE_CLOSURE(); // iterator related stuff
        }
        KAIZEN_IF_SCOPE_CLOSURE();
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
        begin("[zenConstructor]");
        set_return_type("string");
        set_parameter("allocate", "bool"); // ignored since string has no object fields
        push("allocate");
        begin_if_then();
        push("<return>");
        zen::composer::composer::push<i64>(get_type("string")->get_full_size(), "long");
        call(std::to_string(zen::allocate), 2);
        end_if();

        push("<return>.[len]");
        zen::composer::composer::push<i64>(0, "long");
        assign();

        push("<return>.[data]");
        zen::composer::composer::push<i64>(0, "long");
        assign();
        assume_returned();
        end();
    }

    void composer::vm::composer::_link_string_destructor()
    {
        begin("[zenDestructor]");
        set_parameter("it", "string");
        push("it.[data]");
        call("bool", 1);
        begin_if_then();
        push("<int>");
        {
            push("it.[data]");
            const auto deref = dereference(top());
            pop();
            push(deref);
        }
        call(std::to_string(zen::deallocate), 1);
        end_if();
        push("it");
        call(std::to_string(zen::deallocate), 1);
        end();
    }

    void composer::vm::composer::_link_string_copy()
    {
        begin("operator=");
        set_parameter("to", "string");
        set_parameter("from", "string");
        push("to.[data]");
        // push("bool");
        // call("bool", 1, zen::composer::call_result::pushed);
        zen::composer::composer::push<i64>(0, "long");
        not_equal();
        begin_if_then();
        push("to.[data]");
        call(std::to_string(zen::deallocate), 1);
        end_if();
        set_local("data", "long");
        code.push_back(placeholder);
        push("data");
        push("from.[len]");
        call(std::to_string(zen::allocate), 2);
        code.push_back(placeholder);

        push("data");
        {
            push("from.[data]");
            const auto deref = dereference(top());
            pop();
            push(deref);
        }
        {
            push("from.[len]");
            const auto deref = dereference(top());
            pop();
            push(deref);
        }
        call(std::to_string(zen::copy), 3);

        push("to.[data]");
        push("data");
        assign();


        push("to.[len]");
        push("from.[len]");
        assign();
        end();
    }

    void composer::vm::composer::_link_string_equals()
    {
        begin("operator==");
        set_return_type("bool");
        set_parameter("a", "string");
        set_parameter("b", "string");
        set_local("result", "bool");
        push("result");
        push("a");
        push("b");
        call(std::to_string(zen::eq_str), 3);
        push("result");
        return_value();
        end();
    }

    void composer::vm::composer::_link_string_not_equals()
    {
        begin("operator!=");
        set_return_type("bool");
        set_parameter("a", "string");
        set_parameter("b", "string");
        push("a");
        push("b");
        call("operator==", 2);
        not_();
        return_value();
        end();
    }

    void composer::vm::composer::_link_string_plus()
    {
        begin("operator+");
        set_return_type("string");
        set_parameter("a", "string");
        set_parameter("b", "string");
        set_local("result", "string");
        push("result");
        push("a");
        push("b");
        call(std::to_string(zen::add_str), 3);
        push("result");
        return_value();
        end();
    }

    void composer::vm::composer::_link_string_methods()
    {
        class_ = get_type("string");

        begin("string.len");
        set_return_type("long");
        set_parameter("this", "string");
        push("this.[len]");
        return_value();
        end();

        begin("string.empty");
        set_return_type("bool");
        set_parameter("this", "string");
        push("this.[len]");
        zen::composer::composer::push<i64>(0, "long");
        equal();
        return_value();
        end();

        begin("string.at");
        set_return_type("byte");
        set_parameter("this", "string");
        set_parameter("pos", "long");
        push("pos");
        push("this.[len]");
        lower();
        begin_if_then();
        push(get_type("long"));
        auto ptr = top();
        push("this.[data]");
        push("pos");
        plus();
        assign();
        push("<return>");
        push(ptr);
        ptr->is_reference = true;
        ptr->type = get_type("byte");
        assign();
        assume_returned();
        close_branch();
        else_then();
        zen::composer::composer::push<i8>(0, "byte");
        return_value();
        end_if();
        end();

        begin("string.slice");
        set_return_type("string");
        set_parameter("this", "string");
        set_parameter("from", "long");
        set_parameter("to", "long");
        push("from");
        zen::composer::composer::push<i64>(0, "long");
        lower();
        begin_if_then();
        push("from");
        zen::composer::composer::push<i64>(0, "long");
        assign();
        end_if();
        push("to");
        push("this.[len]");
        greater_or_equal();
        begin_if_then();
        push("to");
        push("this.[len]");
        assign();
        end_if();
        set_local("len", "long");
        push("len");
        push("to");
        push("from");
        minus();
        assign();

        push("len");
        zen::composer::composer::push<i64>(0, "long");
        greater();
        begin_if_then();
        {
            set_local("sub", "string");
            push("sub");
            const auto sub = top();
            sub->no_destructor = true;
            pop();
            push(get_type("long"));
            auto ptr = top();
            push("this.[data]");
            push("from");
            plus();
            assign();
            push("sub.[data]");
            push(ptr);
            assign();

            push("sub.[len]");
            push("len");
            assign();

            push("sub");
            return_value();
        }
        close_branch();
        else_then();
        {
            assume_returned(); // because if automatic resource allocation this will leave the return buffer empty
        }
        end_if();
        end();

        begin("string.sub");
        set_return_type("string");
        set_parameter("this", "string");
        set_parameter("from", "long");
        set_parameter("len", "long");
        push("from");
        push("from");
        push("len");
        plus();
        call("this.slice", 2);
        return_value();
        end();


        // begin("string.num");
        // set_return_type("bool");
        // set_parameter("this", "string");
        // push("this.[len]");
        // zen::composer::composer::push<i64>(0,"long");
        // equal();
        // return_value();
        // end();


        /*
        {"string.has", {{signature{get_type("bool"), {string_type}}, 0, false}}},
        {"string.trim", {{signature{unit_type, {string_type}}, 0, false}}},
        {"string.upper", {{signature{unit_type, {string_type}}, 0, false}}},
        {"string.lower", {{signature{unit_type, {string_type}}, 0, false}}},
        {"string.swap", {{signature{unit_type, {string_type, string_type}}, 0, false}}},
        {"string.slice", {{signature{string_type, {get_type("long"), get_type("long")}}, 0, false}}},
        {"string.starts", {{signature{get_type("bool"), {string_type}}, 0, false}}},
        {"string.ends", {{signature{get_type("bool"), {string_type}}, 0, false}}},
        {"string.first", {{signature{get_type("long"), {string_type}}, 0, false}}},
        {"string.last", {{signature{get_type("long"), {string_type}}, 0, false}}},
        {"string.count", {{signature{get_type("long"), {string_type}}, 0, false}}},
        {"string.at", {{signature{get_type("byte"), {string_type}}, 0, false}}},
        {"string.reverse", {{signature{unit_type, {string_type}}, 0, false}}},
        {"string.repeat", {{signature{unit_type, {get_type("long")}}, 0, false}}},
        {"string.capitalize", {{signature{unit_type, {}}, 0, false}}},
        {"string.title", {{signature{unit_type, {}}, 0, false}}},
        {"string.shift", {{signature{get_type("bool"), {string_type}}, 0, false}}},
        {"string.shift", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.shift", {{signature{get_type("bool"), {long}}, 0, false}}},
        {"string.pop", {{signature{get_type("bool"), {string}}, 0, false}}},
        {"string.pop", {{signature{get_type("bool"), {long}}, 0, false}}},
        {"string.pop", {{signature{get_type("bool"), {}}, 0, false}}},

        {"string.num", {{signature{get_type("bool"), {}}, 0, false}}},
        if (c >= '0' && c <= '9') {
        return 1; // Non-zero value indicates true
    } else {
        return 0; // Zero indicates false
    }
        {"string.nums", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.letter", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.letters", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.mix", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.mixed", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.space", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.spaces", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.control", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.controls", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.symbol", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.symbols", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.visible", {{signature{get_type("bool"), {}}, 0, false}}},
        {"string.visibles", {{signature{get_type("bool"), {}}, 0, false}}},
        */
        class_.reset();
    }

    void composer::vm::composer::link()
    {
        KAIZEN_REQUIRE_GLOBAL_SCOPE();
        _link_string_constructor();
        _link_string_destructor();
        _link_string_copy();
        _link_string_equals();
        _link_string_not_equals();
        _link_string_plus();
        _link_string_methods();
    }

    void composer::vm::composer::self_assign_reference(const std::shared_ptr<value>& value)
    {
        value->is_reference = true;
        code.push_back(zen::refer);
        code.push_back(value->address(scope->get_stack_usage()));
        code.push_back(value->address(scope->get_stack_usage()));
    }

    void composer::vm::composer::assign_reference(const std::shared_ptr<value>& to, const std::shared_ptr<value>& from)
    {
        to->is_reference = true;
        code.push_back(zen::refer);
        code.push_back(to->address(scope->get_stack_usage()));
        code.push_back(from->address(scope->get_stack_usage()));
    }

    void composer::vm::composer::push(const std::shared_ptr<type>& type)
    {
        _stack.emplace(std::make_shared<value>(type, scope->get_stack_usage(), value::temporary));
        code.push_back(most);
        code.push_back(-type->get_size());
        scope->use_stack(type->get_size());
    }

    bool composer::vm::composer::call(const std::string& name, const i8& args_count)
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        static std::unordered_map<std::string, std::unordered_map<std::string, i64>> casters{
            {"bool", KAIZEN_CASTER_MAP_FOR(i8)},
            {"byte", KAIZEN_CASTER_MAP_FOR(i8)},
            {"short", KAIZEN_CASTER_MAP_FOR(i16)},
            {"int", KAIZEN_CASTER_MAP_FOR(i32)},
            {"long", KAIZEN_CASTER_MAP_FOR(i64)},
            {"float", KAIZEN_CASTER_MAP_FOR(f32)},
            {"double", KAIZEN_CASTER_MAP_FOR(f64)},
            {"string", KAIZEN_CASTER_MAP_FOR_STRING(str)},
        };
        // deez will allow us to have functions and casters with the same name
        std::shared_ptr<value> caster_arg_buffer;
        if (const auto func_it = fetch_function(name); func_it)
        {
            try
            {
                return _call_function(name, args_count, func_it.value(), caster_arg_buffer);
            }
            catch (exceptions::semantic_error& e)
            {
                if (not casters.contains(name))
                {
                    throw e;
                }
            }
        }

        if (const auto caster_set = casters.find(name); caster_set != casters.end())
        {
            if (caster_arg_buffer)
                push(caster_arg_buffer);
            _call_caster(name, args_count, caster_set);
            return true;
        }
        const int name_i32 = strtol(name.c_str(), nullptr, 10);
        if (name_i32 == write_str)
            return _call_instruction_write_str(name, args_count);
        if (name_i32 >= write_i8 and name_i32 <= write_f64)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 2);
        if (name_i32 == allocate || name_i32 == i64_to_i64)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 2);
        if (name_i32 == deallocate)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 1);
        if (name_i32 == copy || name_i32 == add_str || name_i32 == eq_str)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 3);
        if (name_i32 == placeholder)
            return _call_instruction(static_cast<zen::instruction>(name_i32), args_count, 0);
        throw exceptions::semantic_error(fmt::format("function \"{}\" was not found", name), _ilc_offset);
    }

    bool composer::vm::composer::call_method(const std::string& dot_method, const i8& args_count)
    {
        std::shared_ptr<value> caster_arg_buffer;
        const std::string method = top()->type->name + dot_method;
        if (const auto func_it = functions.find(method); func_it != functions.end())
        {
            return _call_function("", args_count, func_it, caster_arg_buffer);
        }
        throw exceptions::semantic_error(fmt::format("method \"{}\" was not found", dot_method), _ilc_offset);
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

    void composer::vm::composer::peek_pop_pop_push()
    {
        const auto top = _stack.top();
        _stack.pop();
        _stack.pop();
        _stack.push(top);
    }

    void composer::vm::composer::begin_if_then()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        _begin_if_then(false);
    }

    void composer::vm::composer::_begin_if_then(const bool chained)
    {
        if (_stack.empty())
        {
            throw exceptions::semantic_error(fmt::format(R"(cannot use 'if' or 'else if' without providing a value)"),
                                             _ilc_offset);
        }
        code.push_back(zen::instruction::go_if_not);
        const auto condition = pop_operand();
        if (not condition->is("bool"))
        {
            throw exceptions::semantic_error("if condition must be bool", _ilc_offset,
                                             "please use type casting if applicable.");
        }
        label else_label;
        if (not chained)
            scope->labels.emplace();
        else
        {
            else_label = scope->labels.top();
            scope->labels.pop();
        }

        code.push_back(condition->address(scope->get_stack_usage()));
        code.push_back(0);
        else_label.use(code);

        scope->labels.push(else_label);
        if (chained)
        {
            KAIZEN_IF_SCOPE_OPENING(scope::in_else_if);
        }
        else
        {
            KAIZEN_IF_SCOPE_OPENING(scope::in_if);
        }
    }

    void composer::vm::composer::else_if_then()
    {
        KAIZEN_REQUIRE_SCOPE(scope::in_function);
        _else_then(true);
        _begin_if_then(true);
    }

    void composer::vm::composer::else_then()
    {
        _else_then(false);
    }

    void composer::vm::composer::_else_then(bool partial)
    {
        if (not partial)
            KAIZEN_IF_SCOPE_OPENING(scope::in_else);
    }

    void composer::vm::composer::close_branch()
    {
        const bool chained_scope = scope->is(scope::in_else_if) or scope->is(scope::in_else);
        if (chained_scope)
        {
            KAIZEN_IF_SCOPE_CLOSURE(); // remove else's scope
            KAIZEN_IF_SCOPE_CLOSURE_NON_DESTRUCTIVE();
            // remove in between branches' scope but keep it there for next usages
        }
        else
        {
            KAIZEN_REQUIRE_SCOPE(scope::in_if);
            KAIZEN_IF_SCOPE_CLOSURE();
        }

        if (scope->labels.size() < 2)
            throw exceptions::semantic_error(fmt::format("cannot use else if without a prior if"), _ilc_offset);

        // if (scope->return_status != first_branched_return)
        // scope->return_status = no_return;

        label else_label = scope->labels.top();
        scope->labels.pop();

        label& end_label = scope->labels.top();
        scope->labels.emplace();
        code.push_back(zen::instruction::go);
        code.push_back(0);
        end_label.use(code);
        else_label.bind(code);
        if (chained_scope)
        {
            KAIZEN_IF_SCOPE_CLOSURE(); // remove in between branches' scope
        }
        // const auto return_status = scope->get_return_status();
        KAIZEN_IF_SCOPE_OPENING(scope::in_between_branches); // for other chained ifs temp values
        // scope->set_return_status(return_status);
    }

    void composer::vm::composer::end_if()
    {
        if (scope->is(scope::in_else_if) or scope->is(scope::in_else))
        {
            KAIZEN_IF_SCOPE_CLOSURE(); // remove else's scope
            KAIZEN_IF_SCOPE_CLOSURE(); // remove in between branches' scope
        }
        else
        {
            KAIZEN_REQUIRE_SCOPE(scope::in_if);
            KAIZEN_IF_SCOPE_CLOSURE();
        }
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
