//
// Created by dte on 2/24/2025.
//
#pragma once
#include "enums/token_type.hpp"
#include <vector>

#include "builder/library.hpp"
#include "builder/table.hpp"
#include "exceptions/semantic_error.hpp"
#include "exceptions/syntax_error.hpp"
#include "types/stack.hpp"

using namespace enums;
extern std::vector<zen::token> tokens;

// namespace parser
// {
typedef enums::token_type SYMBOL;
#include "ilc/include/ilc.hpp"

BEGIN_ILC_CODEGEN(builder_parser)
#define EXPECTED(ITEM) [this]() { throw zen::exceptions::syntax_error(ITEM, offset); }
    std::shared_ptr<zen::builder::library> lib = zen::builder::library::create("main");
    std::shared_ptr<zen::builder::function> fun;
    std::shared_ptr<zen::builder::table> tab;
    std::shared_ptr<zen::builder::program> prog;
    zen::utils::constant_pool pool;
    std::stack<std::shared_ptr<zen::builder::value>> values;
    std::shared_ptr<zen::builder::label> temp_pel, temp_pen;
    bool at_debug = false;
    bool at_extern = false;
    bool at_test = false;
    bool pragma_dangling_return_value = false;

    std::string id, type, value;
    std::shared_ptr<zen::builder::type> class_;

    inline void reset()
    {
        id.clear();
        type.clear();
        value.clear();
    }

    std::shared_ptr<zen::builder::value> pop()
    {
        if (values.empty())
        {
            throw zen::exceptions::semantic_error("missing value", offset);
        }
        auto top = values.top();
        values.pop();
        return top;
    }

    void push(const std::shared_ptr<zen::builder::value>& val)
    {
        values.push(val);
    }

    BEGIN_PRODUCTION(PRODUCTION_NUSING_STAT)
        REQUIRE_TERMINAL(TKEYWORD_USING)
        REQUIRE_NON_TERMINAL_CALLBACK(NID, EXPECTED("ID"))
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NCLASS)
        REQUIRE_TERMINAL(TKEYWORD_CLASS)
        REQUIRE_NON_TERMINAL_CALLBACK(NID, EXPECTED("ID"))
        class_ = zen::builder::type::create(id, 0);
        class_->kind = zen::builder::type::kind::heap;
        if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
        {
            type.clear();
        }
        REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
        while (TRY_REQUIRE_NON_TERMINAL(NCLASS_FIELD) or TRY_REQUIRE_NON_TERMINAL(NFUNCTION_DEFINITION) or
            TRY_REQUIRE_NON_TERMINAL(NDECORATOR))
        {
        }
        REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
        class_.reset();
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NCLASS_FIELD)
        REQUIRE_TERMINAL(TID)
        std::string field_name = tokens.at(offset - 1).value;
        REQUIRE_TERMINAL(TCOLON)
        type.clear();
        REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
        if (not class_)
        {
            throw zen::exceptions::semantic_error("cannot create class field outside of class", offset);
        }
        if (auto result = zen::builder::table::get_type(type, prog); result.has_value())
        {
            class_->add_field(field_name, result.value(), offset);
        }
        else
        {
            throw zen::exceptions::semantic_error(result.error(), offset);
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(META_PRODUCTION_NELSE)
        REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
        REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
        REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
    END_PRODUCTION

    BEGIN_PRODUCTION(META_PRODUCTION_NIF)
        REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
        REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
        REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
        REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
        if (TRY_REQUIRE_TERMINAL(TKEYWORD_ELSE))
        {
            if (TRY_REQUIRE_TERMINAL(TKEYWORD_IF))
            {
                REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
                /* disable extract for now

                first = true;
                do
                {
                    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
                    if (chain[offset - 1] == TID and (chain[offset - 2] == TPARENTHESIS_OPEN or not
                        first) and TRY_REQUIRE_TERMINAL(TCOLON))
                    {
                        first = false;
                        REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
                        REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
                        REQUIRE_NON_TERMINAL_CALLBACK(NSINGLE_VAL, EXPECTED("VALUE"))
                    }
                }
                while (TRY_REQUIRE_TERMINAL(TAND));
                */
                fun->branch(zen::builder::scope::in_between, nullptr,
                            [&](auto& _, const std::shared_ptr<zen::builder::label>& pel,
                                const std::shared_ptr<zen::builder::label>& pen)-> void
                            {
                                // REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
                                if (!(HANDLE_NON_TERMINAL(NVAL)))
                                {
                                    [this]() { throw zen::exceptions::syntax_error("value", offset); }();
                                }
                                fun->branch(zen::builder::scope::in_else_if, pop(),
                                            [&](auto& _0, const std::shared_ptr<zen::builder::label>& _pel,
                                                const std::shared_ptr<zen::builder::label>& _pen)
                                            {
                                                temp_pel = _pel;
                                                temp_pen = _pen;
                                                META_PRODUCTION_NIF();
                                            }, pel, pen);
                            }, temp_pel, temp_pen);
            }
            else
            {
                fun->branch(zen::builder::scope::in_else, nullptr, [&](auto& _0, auto& _1, auto& _2)
                {
                    META_PRODUCTION_NELSE();
                }, temp_pel, temp_pen);
            }
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NIF)
        REQUIRE_TERMINAL(TKEYWORD_IF)
        REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
        // bool first = true;
        /* disable extract for now
         *do
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
            if (chain[offset - 1] == TID and (chain[offset - 2] == TPARENTHESIS_OPEN or not first) and
                TRY_REQUIRE_TERMINAL(TCOLON))
            {
                REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
                REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
                REQUIRE_NON_TERMINAL_CALLBACK(NSINGLE_VAL, EXPECTED("VALUE"))
            }
            if (not first)
            {
                composer->and_();
            }
            first = false;
        }
        while (TRY_REQUIRE_TERMINAL(TAND));
        */
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
        fun->branch(zen::builder::scope::in_if, pop(),
                    [&](auto& _, const std::shared_ptr<zen::builder::label>& pel,
                        const std::shared_ptr<zen::builder::label>& pen)
                    {
                        temp_pel = pel;
                        temp_pen = pen;
                        META_PRODUCTION_NIF();
                    });
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NSUFFIX_FUNCTION_CALL)
        const std::string name = id;
        if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
        {
            type.clear();
        }
        id = name;
        REQUIRE_TERMINAL(TPARENTHESIS_OPEN)
        std::vector<std::shared_ptr<zen::builder::value>> args;
        while (TRY_REQUIRE_NON_TERMINAL(NVAL))
        {
            args.push_back(pop());
            if (not TRY_REQUIRE_TERMINAL(TCOMMA))
            {
                break;
            }
        }
        REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
        std::vector<std::shared_ptr<zen::builder::type>> params = {};
        for (const auto& arg : args)
        {
            params.push_back(arg->type);
        }
        std::string hint;
        if (auto callee = tab->get_function(name, params, hint); not callee.has_value())
        {
            throw zen::exceptions::semantic_error(callee.error(), offset, hint);
        }
        else
        {
            const auto target = callee.value();
            if (target.first)
            {
                args.insert(args.begin(), target.first);
            }

            if (const auto result = fun->call(target.second, args); not result.
                has_value())
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            }
            else if (result.value())
            {
                push(result.value());
            }
            pragma_dangling_return_value = callee.value().second->signature->type != zen::builder::function::_unit();
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NSUFFIX_METHOD_CALL)
        std::string name = id;
        if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
        {
            type.clear();
        }
        id = name;
        REQUIRE_TERMINAL(TPARENTHESIS_OPEN)
        std::vector<std::shared_ptr<zen::builder::value>> args = {};
        while (TRY_REQUIRE_NON_TERMINAL(NVAL))
        {
            args.push_back(pop());
            if (not TRY_REQUIRE_TERMINAL(TCOMMA))
            {
                break;
            }
        }
        REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
        std::vector<std::shared_ptr<zen::builder::type>> params = {};
        for (const auto& arg : args)
        {
            params.push_back(arg->type);
        }
        std::string hint;
        if (auto callee = tab->get_function(pop(), name, params, hint); not callee.has_value())
        {
            throw zen::exceptions::semantic_error(callee.error(), offset, hint);
        }
        else
        {
            const auto target = callee.value();
            if (target.first)
            {
                args.insert(args.begin(), target.first);
            }

            if (const auto result = fun->call(target.second, args); not result.
                has_value())
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            }
            else if (result.value())
            {
                push(result.value());
            }
            pragma_dangling_return_value = callee.value().second->signature->type != zen::builder::function::_unit();
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NDECORATOR)
        REQUIRE_TERMINAL(TDECORATOR)
        std::string decorator = tokens[offset - 1].value;
        std::vector<std::shared_ptr<zen::builder::value>> args;
        if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
        {
            fun = zen::builder::function::create(pool, offset, at_debug, decorator);
            tab = zen::builder::table::create(fun, class_);
            do
            {
                if (not(PRODUCTION_NVAL_BOOLEAN() or PRODUCTION_NVAL_AS_NUM() or
                    PRODUCTION_NVAL_AS_CHAR_ARRAY()))
                {
                    break;
                }
                args.push_back(pop());
            }
            while (TRY_REQUIRE_TERMINAL(TCOMMA));
            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
        }
        if (decorator == "@debug" and args.empty() or (args.size() == 1 and args.at(0)->type ==
            zen::builder::function::_bool()))
        {
            at_debug = args.empty() or *(bool*)args[0]->address(0);
        }
        else if (decorator == "@extern" and args.empty() or (args.size() == 1 and args.at(0)->type ==
            zen::builder::function::_bool()))
        {
            at_extern = args.empty() or *(bool*)args[0]->address(0);
        }
        else if (decorator == "@test" and args.empty() or (args.size() == 1 and args.at(0)->type ==
                   zen::builder::function::_bool()))
        {
            at_test = args.empty() or *(bool*)args[0]->address(0);
        }
        else
        {
            throw zen::exceptions::semantic_error(
                fmt::format("no such decorator {}{}", decorator, zen::builder::signature::describe_args(args)), offset);
        }

        if (at_debug and at_extern)
        {
            throw zen::exceptions::semantic_error("cannot mix @debug with @extern", offset);
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NFUNCTION_DEFINITION_PREFIX)
        REQUIRE_NON_TERMINAL(NID)
        std::string function_name = id;
        if (function_name == "operator")
        {
            if (class_)
                throw zen::exceptions::semantic_error("cannot define operator in class scope", offset);
            if (TRY_REQUIRE_TERMINAL(TEQU))
                function_name += "=";
            else if (TRY_REQUIRE_TERMINAL(TEQUAL))
                function_name += "==";
            else if (TRY_REQUIRE_TERMINAL(TNOT_EQUAL))
                function_name += "!=";
            else if (TRY_REQUIRE_TERMINAL(TPLUS))
                function_name += "+";
            else if (TRY_REQUIRE_TERMINAL(TMINUS))
                function_name += "-";
            else if (TRY_REQUIRE_TERMINAL(TTIMES))
                function_name += "*";
            else if (TRY_REQUIRE_TERMINAL(TSLASH))
                function_name += "/";
            else if (TRY_REQUIRE_TERMINAL(TMODULO))
                function_name += "%";
            else if (TRY_REQUIRE_TERMINAL(TLOWER))
                function_name += "<";
            else if (TRY_REQUIRE_TERMINAL(TLOWER_OR_EQUAL))
                function_name += "<=";
            else if (TRY_REQUIRE_TERMINAL(TGREATER))
                function_name += ">";
            else if (TRY_REQUIRE_TERMINAL(TGREATER_OR_EQUAL))
                function_name += ">=";
            else if (TRY_REQUIRE_TERMINAL(TNOT))
                function_name += "!";
            else if (TRY_REQUIRE_TERMINAL(TAND))
                function_name += "&&";
            else if (TRY_REQUIRE_TERMINAL(TOR))
                function_name += "||";
        }
        bool is_constructor = false;

        if (class_)
        {
            if (function_name == "new")
            {
                is_constructor = true;
                function_name = class_->name;
            }
            else
            {
                function_name = fmt::format("{}.{}", class_->name, function_name);
            }
        }
        fun = zen::builder::function::create(pool, offset, at_debug, function_name);
        tab = zen::builder::table::create(fun, class_, prog);
        if (at_debug)
        {
            fmt::println("{}", fun->name);
        }
        std::list<std::tuple<std::string, std::shared_ptr<zen::builder::type>>> parameters;
        if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
        {
            type.clear();
        }
        if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
        {
            bool first_it = true;
            do
            {
                if (TRY_REQUIRE_NON_TERMINAL(NID))
                {
                    if (at_test) throw zen::exceptions::semantic_error("@test function cannot have parameters", offset);
                    const std::string name = id;
                    REQUIRE_TERMINAL_CALLBACK(TCOLON, EXPECTED(":"))
                    type.clear();
                    REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
                    if (auto result = tab->get_type(type); result.has_value())
                    {
                        parameters.emplace_back(name, result.value());
                    }
                    else
                    {
                        throw zen::exceptions::semantic_error(result.error(), offset);
                    }
                }
                else if (not first_it)
                {
                    EXPECTED("ID")();
                }
                first_it = false;
            }
            while (TRY_REQUIRE_TERMINAL(TCOMMA));
            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
        }

        REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
        type.clear();
        if (not is_constructor and TRY_REQUIRE_NON_TERMINAL(NTYPE))
        {
            if (auto result = tab->get_type(type); result.has_value())
            {
                fun->set_return(result.value());
            }
            else
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            }
        }
        if (at_test and fun->signature->type != zen::builder::function::_bool())
        {
            throw zen::exceptions::semantic_error("@test function must return bool", offset);
        }
        if (class_)
        {
            if (at_test) throw zen::exceptions::semantic_error("method cannot be marked as @test", offset);
            if (is_constructor)
            {
                fun->set_alias(fun->set_return(class_), "this");
                fun->return_implicitly();
            }
            else
            {
                fun->set_parameter(class_, "this");
            }
        }
        for (auto param : parameters)
        {
            fun->set_parameter(std::get<1>(param), std::get<0>(param));
        }
        lib->add(fun);
        fun->is_extern = at_extern;
        if (at_test)
        {
            lib->add_test(fun);
        }
        at_test = false;
        at_extern = false;
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NFUNCTION_DEFINITION)
        REQUIRE_NON_TERMINAL(NFUNCTION_DEFINITION_PREFIX)
        if (not fun->is_extern)
        {
            if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
            {
                REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
                if (*fun->signature->type != *zen::builder::function::_unit())
                {
                    fun->return_value(pop());
                }
                REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
            }
            else
            {
                REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
                REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
                REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
            }
            fun->build();
        }
        at_debug = false;
    END_PRODUCTION

    BEGIN_PRODUCTION(META_PRODUCTION_NFOR)
        REQUIRE_TERMINAL_CALLBACK(TID, EXPECTED("ID"))
        const std::string iterator = tokens[offset - 1].value;
        type.clear();
        /*if (TRY_REQUIRE_TERMINAL(TCOLON))
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
        }*/
        REQUIRE_TERMINAL(TCOLON)
        std::vector<std::shared_ptr<zen::builder::value>> for_params;
        // REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
        if (TRY_REQUIRE_NON_TERMINAL(NTYPE))
        {
            if (auto result = tab->get_type(type); result.has_value())
            {
                for_params.push_back(fun->set_local(result.value(), iterator));
            }
            else
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            }
            REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
            for_params.push_back(pop());
        } else
        {
            REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
            const auto begin = pop();
            for_params.push_back(fun->set_local(begin->type, iterator));
            for_params.push_back(begin);
        }
        REQUIRE_TERMINAL_CALLBACK(TCOMMA, EXPECTED(","))
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
        for_params.push_back(pop());
        if (TRY_REQUIRE_TERMINAL(TCOMMA))
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
            for_params.push_back(pop());
        }
        if (TRY_REQUIRE_TERMINAL(TSEMICOLON))
        {
            fun->loop_for(for_params, [&](auto& _)
            {
                META_PRODUCTION_NFOR();
            });
        }
        else
        {
            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
            REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
            fun->loop_for(for_params, [&](auto& _)
            {
                TRY_REQUIRE_NON_TERMINAL(NFUNCTION_BODY);
            });
            REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NFOR)
        REQUIRE_TERMINAL(TKEYWORD_FOR)
        REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
        META_PRODUCTION_NFOR();
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NWHILE)
        REQUIRE_TERMINAL(TKEYWORD_WHILE)
        std::vector<std::shared_ptr<zen::builder::value>> params;
        fun->loop_while(params, [&](auto& _)-> void
                        {
                            auto callback = [&]()-> bool
                            {
                                REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
                                REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
                                params.push_back(pop());
                                REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
                                return true;
                            };
                            callback();
                        }, [&](auto& _)-> void
                        {
                            auto callback = [&]()-> bool
                            {
                                REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
                                REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
                                REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
                                return true;
                            };
                            callback();
                        });
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_PREFIX_VAL)
        REQUIRE_NON_TERMINAL(NSINGLE_VAL)
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_MOD_VALUE)
        REQUIRE_TERMINAL(TMODULO)
        REQUIRE_NON_TERMINAL_CALLBACK(NFIRST_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(lhs->type, "temp::mod");
        fun->mod(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_TIMES_VALUE)
        REQUIRE_TERMINAL(TTIMES)
        REQUIRE_NON_TERMINAL_CALLBACK(NFIRST_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(lhs->type, "temp::mul");
        fun->mul(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_SLASH_VALUE)
        REQUIRE_TERMINAL(TSLASH)
        REQUIRE_NON_TERMINAL_CALLBACK(NFIRST_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(lhs->type, "temp::div");
        fun->div(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_PLUS_VAL)
        REQUIRE_TERMINAL(TPLUS)
        REQUIRE_NON_TERMINAL_CALLBACK(NSECOND_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(lhs->type, "temp::add");
        fun->add(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_MINUS_VALUE)
        REQUIRE_TERMINAL(TMINUS)
        REQUIRE_NON_TERMINAL_CALLBACK(NTHIRD_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(lhs->type, "temp::sub");
        fun->sub(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_GREATER_VALUE)
        REQUIRE_TERMINAL(TGREATER)
        REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::greater");
        fun->greater(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE)
        REQUIRE_TERMINAL(TGREATER_OR_EQUAL)
        REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::greater_equal");
        fun->greater_equal(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_LOWER_VALUE)
        REQUIRE_TERMINAL(TLOWER)
        REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::lower");
        fun->lower(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE)
        REQUIRE_TERMINAL(TLOWER_OR_EQUAL)
        REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::lower_equal");
        fun->lower_equal(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_EQUAL_VALUE)
        REQUIRE_TERMINAL(TEQUAL)
        REQUIRE_NON_TERMINAL_CALLBACK(NFIFTH_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::equal");
        fun->equal(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_NOT_EQUAL_VALUE)
        REQUIRE_TERMINAL(TNOT_EQUAL)
        REQUIRE_NON_TERMINAL_CALLBACK(NFIFTH_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::not_equal");
        fun->not_equal(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_AND_VALUE)
        REQUIRE_TERMINAL(TAND)
        REQUIRE_NON_TERMINAL_CALLBACK(NSIXTH_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::and");
        fun->and_(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_OR_VALUE)
        REQUIRE_TERMINAL(TOR)
        REQUIRE_NON_TERMINAL_CALLBACK(NSEVENTH_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::or");
        fun->or_(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_AS_NUM)
        if (TRY_REQUIRE_TERMINAL(TINT_NUM))
            push(fun->constant<zen::i32>(strtol(tokens[offset - 1].value.c_str(), nullptr, 10)));
        else if (TRY_REQUIRE_TERMINAL(TBYTE_NUM))
            push(fun->constant<zen::i8>(static_cast<char>(strtol(tokens[offset - 1].value.c_str(), nullptr, 10))));
        else if (TRY_REQUIRE_TERMINAL(TSHORT_NUM))
            push(fun->constant<zen::i16>(static_cast<short>(strtol(tokens[offset - 1].value.c_str(), nullptr, 10))));
        else if (TRY_REQUIRE_TERMINAL(TLONG_NUM))
            push(fun->constant<zen::i64>(strtoll(tokens[offset - 1].value.c_str(), nullptr, 10)));
        else if (TRY_REQUIRE_TERMINAL(TFLOAT_NUM))
            push(fun->constant<zen::f32>(strtof(tokens[offset - 1].value.c_str(), nullptr)));
        else
        {
            REQUIRE_TERMINAL(TDOUBLE_NUM)
            push(fun->constant<zen::f64>(strtod(tokens[offset - 1].value.c_str(), nullptr)));
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_AS_CHAR_ARRAY)
        REQUIRE_TERMINAL(TCHAR_ARRAY)
        push(fun->constant<std::string>(tokens[offset - 1].value));
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_NOT_VAL)
        REQUIRE_TERMINAL(TNOT)
        REQUIRE_NON_TERMINAL_CALLBACK(NSINGLE_VAL, EXPECTED("single value"))
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::not");
        fun->not_(r, pop());
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_NEGATE_VAL)
        REQUIRE_TERMINAL(TMINUS)
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
        auto it = pop();
        if (it->kind == zen::builder::value::constant)
        {
            auto _it = fun->set_local(it->type, "temp::negate_constant");
            fun->move(_it, it);
            it = _it;
        }
        fun->mul(it, it, fun->constant(-1, it->type));
        it->is_negated = true;
        push(it);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_WITH_PARENTHESIS)
        REQUIRE_TERMINAL(TPARENTHESIS_OPEN)
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
        REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_AS_LIST)
        REQUIRE_TERMINAL(TBRACKETS_OPEN)
        do
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
        }
        while (TRY_REQUIRE_TERMINAL(TCOMMA));
        REQUIRE_TERMINAL_CALLBACK(TBRACKETS_CLOSE, EXPECTED("]"))
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_BOOLEAN)
        if (not TRY_REQUIRE_TERMINAL(TKEYWORD_TRUE))
        {
            REQUIRE_TERMINAL(TKEYWORD_FALSE)
            push(fun->constant(false));
        }
        else
        {
            push(fun->constant(true));
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NGENERIC)
        REQUIRE_TERMINAL(TLOWER)
        do
        {
            REQUIRE_NON_TERMINAL(NTYPE)
        }
        while (TRY_REQUIRE_TERMINAL(TCOMMA));
        REQUIRE_TERMINAL(TGREATER)
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NTYPE)
        if (not TRY_REQUIRE_NON_TERMINAL(NID))
        {
            ROLLBACK_PRODUCTION()
        }
        type += tokens[offset - 1].value;
        if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
        {
            type.clear();
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVARIABLE_DEFINITION)
        REQUIRE_NON_TERMINAL(NID)
        const auto name = id;
        REQUIRE_TERMINAL(TCOLON)
        type.clear();
        // REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
        if (TRY_REQUIRE_NON_TERMINAL(NTYPE))
        {
            std::shared_ptr<zen::builder::value> local;
            if (auto result = tab->get_type(id); result.has_value())
            {
                local = fun->set_local(result.value(), name);
            }
            else
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            }
            if (TRY_REQUIRE_TERMINAL(TEQU))
            {
                push(local);
                offset--;
                REQUIRE_NON_TERMINAL_CALLBACK(NSUFFIX_ASGN, EXPECTED("ASSIGNMENT"))
            }
        }
        else
        {
            REQUIRE_TERMINAL(TEQU);
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
            auto rhs = pop();
            auto local = fun->set_local(rhs->type, name);
            fun->move(local, rhs);
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NASGN_SUFFIX)
        REQUIRE_TERMINAL(TEQU);
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        fun->move(lhs, rhs);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_ENDLESS_SUFFIXES)
        while (TRY_REQUIRE_NON_TERMINAL(NSUFIXED_VAL))
        {
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(META_PRODUCTION_GLOBAL_STAT)
        while (TRY_REQUIRE_NON_TERMINAL(NGLOBAL_STAT))
        {
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(META_PRODUCTION_NANY_BODY)
        zen::i64 parentheses = 0;
        zen::i64 braces = 0;
        if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
        {
            parentheses++;
        }
        else if (TRY_REQUIRE_TERMINAL(TBRACES_OPEN))
        {
            braces++;
        }
        while (parentheses > 0 or braces > 0)
        {
            if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
            {
                parentheses++;
            }
            else if (TRY_REQUIRE_TERMINAL(TBRACES_OPEN))
            {
                braces++;
            }
            else if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_CLOSE))
            {
                parentheses--;
            }
            else if (TRY_REQUIRE_TERMINAL(TBRACES_CLOSE))
            {
                braces--;
            }
            else if (offset < chain_size)
            {
                offset++; // ignore anything else
            } else
            {
                break;
            }
        }
    END_PRODUCTION

    inline std::shared_ptr<zen::builder::function> create_allocator(const std::shared_ptr<zen::builder::type>& type)
    {
        const auto fun = zen::builder::function::create(pool, 0, false, fmt::format("{}::allocate", type->name));
        fun->set_return(type);
        fun->gen<zen::allocate>(fun->ret, fun->constant<zen::i64>(type->get_full_size()));
        fun->return_implicitly();
        fun->build();
        return fun;
    }

    inline std::shared_ptr<zen::builder::function> create_deallocator(const std::shared_ptr<zen::builder::type>& type)
    {
        const auto fun = zen::builder::function::create(pool, 0, false, fmt::format("{}::deallocate", type->name));
        const auto it = fun->set_parameter(type, "it");
        const auto tab = zen::builder::table::create(fun);
        const auto has_data = fun->set_local(zen::builder::function::_bool(), "has_data");
        for (const auto& field_pair : type->fields)
        {
            if (field_pair.second == zen::builder::function::_string())
            {
                const auto field = tab->get_field_or_throw(it, field_pair.first);
                const auto field_data_field = tab->get_field_or_throw(field, "string::data");
                const auto field_data = fun->dereference(field_data_field);
                fun->gen<zen::i64_to_boolean>(has_data, field_data);
                fun->branch(zen::builder::scope::in_if, has_data, [&](auto& fb, auto&, auto&)
                {
                    fun->gen<zen::deallocate>(field_data);
                });
            }
        }
        fun->gen<zen::deallocate>(it);
        fun->build();
        return fun;
    }

    inline std::shared_ptr<zen::builder::function> create_mover(const std::shared_ptr<zen::builder::type>& type)
    {
        const auto fun = zen::builder::function::create(pool, 0, false, fmt::format("operator=", type->name));
        const auto lhs = fun->set_parameter(type, "lhs");
        const auto rhs = fun->set_parameter(type, "rhs");
        const auto tab = zen::builder::table::create(fun);
        for (const auto& field_pair : type->fields)
        {
            const auto lhs_field = tab->get_field_or_throw(lhs, field_pair.first);
            const auto rhs_field = tab->get_field_or_throw(rhs, field_pair.first);
            fun->move(lhs_field, rhs_field);
        }
        fun->build();
        return fun;
    }

inline std::shared_ptr<zen::builder::function> create_equals(const std::shared_ptr<zen::builder::type>& type)
        {
            const auto fun = zen::builder::function::create(pool, 0, false, fmt::format("operator==", type->name));
            fun->set_return(zen::builder::function::_bool());
            const auto lhs = fun->set_parameter(type, "lhs");
            const auto rhs = fun->set_parameter(type, "rhs");
            fun->move(fun->ret, fun->constant<zen::boolean>(true));
            const auto tab = zen::builder::table::create(fun);
            const auto temp = fun->set_local(zen::builder::function::_bool(), "temp");
            for (const auto& field_pair : type->fields)
            {
                const auto lhs_field = tab->get_field_or_throw(lhs, field_pair.first);
                const auto rhs_field = tab->get_field_or_throw(rhs, field_pair.first);
                fun->equal(temp, lhs_field, rhs_field);
                fun->and_(fun->ret, fun->ret, temp);
            }
            fun->return_implicitly();
            fun->build();
            return fun;
        }

inline std::shared_ptr<zen::builder::function> create_not_equals(const std::shared_ptr<zen::builder::type>& type)
        {
            const auto fun = zen::builder::function::create(pool, 0, false, fmt::format("operator!=", type->name));
            fun->set_return(zen::builder::function::_bool());
            const auto lhs = fun->set_parameter(type, "lhs");
            const auto rhs = fun->set_parameter(type, "rhs");
            if (const auto result = fun->call(fun->create("operator==", std::vector<std::shared_ptr<zen::builder::type>>{lhs->type, rhs->type}, fun->ret->type), {lhs, rhs});not result.has_value())
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            } else
            {
                const auto negation = fun->set_local(zen::builder::function::_bool(), "negation");
                fun->not_(negation,result.value());
                fun->return_value(negation);
            }
            fun->build();
            return fun;
        }

    BEGIN_PRODUCTION(META_PRODUCTION_GLOBAL_DISCOVERY)
        while (offset < chain_size)
        {
            if (at_debug) at_debug = false;
            if (TRY_REQUIRE_NON_TERMINAL(NFUNCTION_DEFINITION_PREFIX))
            {
                REQUIRE_NON_TERMINAL(META_NANY_BODY)
            }
            else if (TRY_REQUIRE_TERMINAL(TKEYWORD_CLASS))
            {
                REQUIRE_TERMINAL(TID)
                class_ = zen::builder::type::create(tokens[offset - 1].value);
                class_->kind = zen::builder::type::kind::heap;
                lib->add(class_);
                if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
                {
                }
                REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
                while (TRY_REQUIRE_NON_TERMINAL(NCLASS_FIELD) or TRY_REQUIRE_NON_TERMINAL(NGLOBAL_DISCOVERY_STAT))
                {
                    REQUIRE_NON_TERMINAL(META_NANY_BODY)
                }
                lib->add(create_allocator(class_));
                lib->add(create_deallocator(class_));
                lib->add(create_mover(class_));
                lib->add(create_equals(class_));
                lib->add(create_not_equals(class_));
                class_ = nullptr;
                REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
            }
            else if (not TRY_REQUIRE_NON_TERMINAL(NDECORATOR))
            {
                // fmt::println("broke at {}", offset < chain_size ? tokens[offset].get_location_string() : "undefined");
                return false;
            }
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NFUNCTION_BODY)
        bool dangling_value = false;
        while (true)
        {
            if (TRY_REQUIRE_NON_TERMINAL(NSTAT))
            {
                if (dangling_value)
                {
                    pop();
                    dangling_value = false;
                }
                continue;
            }
            if (TRY_REQUIRE_NON_TERMINAL(NID))
            {
                if (dangling_value)
                    pop();
                if (TRY_REQUIRE_NON_TERMINAL(NSUFFIX_FUNCTION_CALL))
                {
                    dangling_value = pragma_dangling_return_value;
                    if (dangling_value)
                        TRY_REQUIRE_NON_TERMINAL(NENDLESS_SUFIXES);
                    continue;
                }
                if (auto result = tab->get_value(id); result.has_value())
                {
                    push(result.value());
                }
                else
                {
                    throw zen::exceptions::semantic_error(result.error(), offset);
                }
                TRY_REQUIRE_NON_TERMINAL(NENDLESS_SUFIXES);
                dangling_value = true;
                break;
            }
            if (TRY_REQUIRE_NON_TERMINAL(NVAL))
            {
                dangling_value = true;
            }
            break;
        } // improve return handler
        if (dangling_value)
        {
            if (fun->signature->type != zen::builder::function::_unit())
            {
                fun->return_value(pop());
            }
            else
            {
                pop();
            }
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NSTAT_FROM_ASGN)
        REQUIRE_NON_TERMINAL(NID)
        if (TRY_REQUIRE_TERMINAL(TEQU))
        {
            if (auto result = tab->get_value(id); result.has_value())
            {
                push(result.value());
            }
            else
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            }
            offset--;
            REQUIRE_NON_TERMINAL(NSUFFIX_ASGN)
        }
        else
        {
            ROLLBACK_PRODUCTION()
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NSINGLE_VAL)
        const bool pre_increment = TRY_REQUIRE_TERMINAL(TPLUS_PLUS);
        const bool pre_decrement = pre_increment or TRY_REQUIRE_TERMINAL(TMINUS_MINUS);
        bool post_increment = false;
        bool post_decrement = false;
        REQUIRE_NON_TERMINAL(NSINGLE_VAL_PREDICATE)
        if (not(pre_increment or pre_decrement))
        {
            post_increment = TRY_REQUIRE_TERMINAL(TPLUS_PLUS);
            post_decrement = post_increment or TRY_REQUIRE_TERMINAL(TMINUS_MINUS);
        }
        const std::shared_ptr<zen::builder::value> it = pop();
        std::shared_ptr<zen::builder::value> one = (pre_decrement or post_decrement or pre_increment or post_increment)
                                                       ? fun->constant(1, it->type)
                                                       : nullptr;
        if (pre_increment)
        {
            fun->add(it, it, one);
            push(it);
        }
        else if (pre_decrement)
        {
            fun->sub(it, it, one);
            push(it);
        }
        else if (post_increment)
        {
            const auto val = fun->set_local(it->type, "temp::befo_inc");
            fun->move(val, it);
            fun->add(it, it, one);
            push(val);
        }
        else if (post_decrement)
        {
            const auto val = fun->set_local(it->type, "temp::befo_dec");
            fun->move(val, it);
            fun->sub(it, it, one);
            push(val);
        }
        else
        {
            push(it);
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NMEMBER_ACCESS)
        REQUIRE_TERMINAL(TDOT);
        do
        {
            id.clear();
            do
            {
                REQUIRE_TERMINAL_CALLBACK(TID, EXPECTED("identifier"))
                id += "." + tokens[offset - 1].value;
            }
            while (TRY_REQUIRE_TERMINAL(TDOT));
            if (not TRY_REQUIRE_NON_TERMINAL(NSUFFIX_METHOD_CALL))
            {
                auto result = zen::builder::table::get_field(pop(), zen::builder::table::split_name(id.substr(1)),
                                                             [&](std::shared_ptr<zen::builder::value>& ptr,
                                                                 const std::shared_ptr<zen::builder::value>& original)
                                                             {
                                                                 if (not ptr)
                                                                 {
                                                                     ptr = fun->set_local(
                                                                         zen::builder::function::_long(),
                                                                         "temp::field");
                                                                 }
                                                                 else
                                                                 {
                                                                     fun->gen<zen::add_i64>(
                                                                         ptr, original, ptr->offset,
                                                                         fmt::format("@offset:{}", ptr->offset));
                                                                 }
                                                             });

                if (result.has_value())
                {
                    push(result.value());
                }
                else
                {
                    throw zen::exceptions::semantic_error(result.error(), offset);
                }
            }
        }
        while (TRY_REQUIRE_TERMINAL(TDOT));
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NID)
        id.clear();
        REQUIRE_TERMINAL(TID)
        id += tokens[offset - 1].value;
        while (TRY_REQUIRE_TERMINAL(TDOT))
        {
            if (TRY_REQUIRE_TERMINAL(TTIMES))
            {
                id += "." + tokens[offset - 1].value;
                break;
            }
            REQUIRE_TERMINAL_CALLBACK(TID, EXPECTED("identifier"))
            id += "." + tokens[offset - 1].value;
        }
    END_PRODUCTION

    inline bool push_parser_id()
    {
        if (auto result = tab->get_value(id); result.has_value())
        {
            push(result.value());
        }
        else
        {
            throw zen::exceptions::semantic_error(result.error(), offset);
        }
        return true;
    }

    BEGIN_BINDINGS
    BEGIN_SYMBOL_BINDING(NOR_VAL)
            PRODUCTION_NVAL_OR_VALUE()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NAND_VAL)
            PRODUCTION_NVAL_AND_VALUE()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSUFFIX_ASGN)
            PRODUCTION_NASGN_SUFFIX()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NID)
           PRODUCTION_NID()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NTYPE)
           PRODUCTION_NTYPE()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NGENERIC)
           PRODUCTION_NGENERIC()
        END_SYMBOL_BINDING

        // BEGIN_SYMBOL_BINDING(NVARIABLE_DEFINITION)
        //             PRODUCTION_NVARIABLE_DEFINITION()
        //         END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSUFFIX_FUNCTION_CALL)
            PRODUCTION_NSUFFIX_FUNCTION_CALL()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NFUNCTION_DEFINITION)
    PRODUCTION_NFUNCTION_DEFINITION()
END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NCLASS_FIELD)
    PRODUCTION_NCLASS_FIELD()
END_SYMBOL_BINDING
    BEGIN_SYMBOL_BINDING(NDECORATOR)
        PRODUCTION_NDECORATOR()
    END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NGLOBAL_DISCOVERY_STAT)
    META_PRODUCTION_GLOBAL_DISCOVERY()
END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NGLOBAL_STAT)
            PRODUCTION_NDECORATOR() or
            PRODUCTION_NFUNCTION_DEFINITION() or
            PRODUCTION_NUSING_STAT() or
            PRODUCTION_NCLASS()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSTAT)
            PRODUCTION_NVARIABLE_DEFINITION() or
            PRODUCTION_NIF() or
            PRODUCTION_NFOR() or
            PRODUCTION_NWHILE() or
            PRODUCTION_NSTAT_FROM_ASGN()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NFUNCTION_BODY)
            PRODUCTION_NFUNCTION_BODY()
        END_SYMBOL_BINDING


    BEGIN_SYMBOL_BINDING(NVAL)
            (PRODUCTION_NVAL_PREFIX_VAL() and
                (PRODUCTION_NVAL_SLASH_VALUE() or
                    PRODUCTION_NVAL_TIMES_VALUE() or
                    PRODUCTION_NVAL_PLUS_VAL() or
                    PRODUCTION_NVAL_MINUS_VALUE() or
                    PRODUCTION_NVAL_GREATER_VALUE() or
                    PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE() or
                    PRODUCTION_NVAL_LOWER_VALUE() or
                    PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE() or
                    PRODUCTION_NVAL_EQUAL_VALUE() or
                    PRODUCTION_NVAL_NOT_EQUAL_VALUE() or
                    PRODUCTION_NVAL_NOT_VAL() or
                    PRODUCTION_NVAL_AND_VALUE() or
                    PRODUCTION_NVAL_OR_VALUE() or
                    // PRODUCTION_NSUFFIX_FUNCTION_CALL() or
                    true)
            ) and PRODUCTION_ENDLESS_SUFFIXES()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSINGLE_VAL_PREDICATE)
            (PRODUCTION_NVAL_NOT_VAL() or
                PRODUCTION_NVAL_NEGATE_VAL() or
                PRODUCTION_NVAL_AS_NUM() or
                PRODUCTION_NVAL_AS_CHAR_ARRAY() or
                (PRODUCTION_NID() and (PRODUCTION_NSUFFIX_FUNCTION_CALL() or push_parser_id())) or
                PRODUCTION_NVAL_BOOLEAN() or
                PRODUCTION_NVAL_WITH_PARENTHESIS() or PRODUCTION_NVAL_AS_LIST()) and (PRODUCTION_NMEMBER_ACCESS() or
                true)
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSINGLE_VAL)
            PRODUCTION_NSINGLE_VAL()
        END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSEVENTH_PRIORITY_RHS_VAL)
            (PRODUCTION_NVAL_PREFIX_VAL() and (
                PRODUCTION_NVAL_TIMES_VALUE() or
                PRODUCTION_NVAL_SLASH_VALUE() or
                PRODUCTION_NVAL_PLUS_VAL() or
                PRODUCTION_NVAL_MINUS_VALUE() or
                PRODUCTION_NVAL_LOWER_VALUE() or
                PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE() or
                PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE() or
                PRODUCTION_NVAL_GREATER_VALUE() or
                PRODUCTION_NVAL_EQUAL_VALUE() or
                PRODUCTION_NVAL_NOT_EQUAL_VALUE() or
                PRODUCTION_NVAL_AND_VALUE() or
                PRODUCTION_NVAL_OR_VALUE() or
                true))
    END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSIXTH_PRIORITY_RHS_VAL)
            (PRODUCTION_NVAL_PREFIX_VAL() and (
                PRODUCTION_NVAL_TIMES_VALUE() or
                PRODUCTION_NVAL_SLASH_VALUE() or
                PRODUCTION_NVAL_PLUS_VAL() or
                PRODUCTION_NVAL_MINUS_VALUE() or
                PRODUCTION_NVAL_LOWER_VALUE() or
                PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE() or
                PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE() or
                PRODUCTION_NVAL_GREATER_VALUE() or
                PRODUCTION_NVAL_EQUAL_VALUE() or
                PRODUCTION_NVAL_NOT_EQUAL_VALUE() or
                PRODUCTION_NVAL_AND_VALUE() or
                true))
      END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NFIFTH_PRIORITY_RHS_VAL)
            (PRODUCTION_NVAL_PREFIX_VAL() and (
                PRODUCTION_NVAL_TIMES_VALUE() or
                PRODUCTION_NVAL_SLASH_VALUE() or
                PRODUCTION_NVAL_PLUS_VAL() or
                PRODUCTION_NVAL_MINUS_VALUE() or
                PRODUCTION_NVAL_LOWER_VALUE() or
                PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE() or
                PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE() or
                PRODUCTION_NVAL_GREATER_VALUE() or
                PRODUCTION_NVAL_EQUAL_VALUE() or
                PRODUCTION_NVAL_NOT_EQUAL_VALUE() or
                true))
      END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NFOURTH_PRIORITY_RHS_VAL)
            (PRODUCTION_NVAL_PREFIX_VAL() and (
                PRODUCTION_NVAL_TIMES_VALUE() or
                PRODUCTION_NVAL_SLASH_VALUE() or
                PRODUCTION_NVAL_PLUS_VAL() or
                PRODUCTION_NVAL_MINUS_VALUE() or
                PRODUCTION_NVAL_LOWER_VALUE() or
                PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE() or
                PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE() or
                PRODUCTION_NVAL_GREATER_VALUE() or
                true))
      END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NTHIRD_PRIORITY_RHS_VAL)
            (PRODUCTION_NVAL_PREFIX_VAL() and (
                PRODUCTION_NVAL_TIMES_VALUE() or
                PRODUCTION_NVAL_SLASH_VALUE() or
                true))
      END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSECOND_PRIORITY_RHS_VAL)
            (PRODUCTION_NVAL_PREFIX_VAL() and (
                PRODUCTION_NVAL_TIMES_VALUE() or
                PRODUCTION_NVAL_SLASH_VALUE() or
                PRODUCTION_NVAL_PLUS_VAL() or
                true))
      END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NFIRST_PRIORITY_RHS_VAL)
            (PRODUCTION_NVAL_PREFIX_VAL() and (
                PRODUCTION_NVAL_TIMES_VALUE() or
                PRODUCTION_NVAL_SLASH_VALUE() or
                PRODUCTION_NVAL_MOD_VALUE() or
                true))
      END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NENDLESS_SUFIXES)
        PRODUCTION_ENDLESS_SUFFIXES()
    END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NFUNCTION_DEFINITION_PREFIX)
        PRODUCTION_NFUNCTION_DEFINITION_PREFIX()
    END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(META_NANY_BODY)
        META_PRODUCTION_NANY_BODY()
    END_SYMBOL_BINDING

    BEGIN_SYMBOL_BINDING(NSUFIXED_VAL)
          PRODUCTION_NVAL_SLASH_VALUE() or
          PRODUCTION_NVAL_TIMES_VALUE() or
          PRODUCTION_NVAL_MOD_VALUE() or
          PRODUCTION_NVAL_PLUS_VAL() or
          PRODUCTION_NVAL_MINUS_VALUE() or
          PRODUCTION_NVAL_GREATER_VALUE() or
          PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE() or
          PRODUCTION_NVAL_LOWER_VALUE() or
          PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE() or
          PRODUCTION_NVAL_EQUAL_VALUE() or
          PRODUCTION_NVAL_NOT_EQUAL_VALUE() or
          PRODUCTION_NVAL_AND_VALUE() or
          PRODUCTION_NVAL_OR_VALUE() or PRODUCTION_NMEMBER_ACCESS()
          // PRODUCTION_NSUFFIX_FUNCTION_CALL()
        END_SYMBOL_BINDING
    BEGIN_SYMBOL_BINDING(NSUFFIX_METHOD_CALL)
        PRODUCTION_NSUFFIX_METHOD_CALL()
    END_SYMBOL_BINDING
    END_BINDINGS

    inline bool discover()
    {
        compilation_id++;
        offset = 0;
        META_PRODUCTION_GLOBAL_DISCOVERY();
        return offset == chain_size;
    }

    inline bool parse()
    {
        compilation_id++;
        offset = 0;
        META_PRODUCTION_GLOBAL_STAT();
            bool success = offset == chain_size;
            if (not success)
            {
                EXPECTED("class, function or decorator")();
            }
        return success;
    }

END_ILC_CODEGEN(builder_parser)
// }
