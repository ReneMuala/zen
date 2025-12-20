//
// Created by dte on 2/24/2025.
//
#pragma once
#include "enums/token_type.hpp"
#include <vector>

#include "builder/library.hpp"
#include "builder/table.hpp"
#include "composer/composer.hpp"
#include "composer/vm/composer.hpp"
#include "exceptions/semantic_error.hpp"
#include "exceptions/syntax_error.hpp"
#include "types/stack.hpp"
#include "types/heap.hpp"

using namespace enums;
extern std::vector<zen::token> tokens;

// namespace parser
// {
typedef enums::token_type SYMBOL;
#include "ilc/include/ilc.hpp"

BEGIN_ILC_CODEGEN(builder_parser)
#define EXPECTED(ITEM) [this]() { throw zen::exceptions::syntax_error(ITEM, offset); }
    std::shared_ptr<zen::builder::library> lib;
    std::shared_ptr<zen::builder::function> fun;
    std::shared_ptr<zen::builder::table> tab;
    zen::utils::constant_pool pool;
    std::stack<std::shared_ptr<zen::builder::value>> values;
    const bool logging = false;
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
        return values.top();
    }

    void push(const std::shared_ptr<zen::builder::value>& value)
    {
        values.push(value);
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
        while (TRY_REQUIRE_NON_TERMINAL(NCLASS_FIELD) or TRY_REQUIRE_NON_TERMINAL(NFUNCTION_DEFINITION))
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
        if (auto result = tab->get_type(type); result.has_value())
        {
            class_->add_field(field_name, result.value(), offset);
        }
        else
        {
            throw zen::exceptions::semantic_error(result.error(), offset);
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
        composer->begin_if_then();
        REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
        REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
        REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
        REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
        while (TRY_REQUIRE_TERMINAL(TKEYWORD_ELSE))
        {
            composer->close_branch();
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
                REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
                composer->else_if_then();
                REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
                REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
                REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
                REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
            }
            else
            {
                composer->else_then();
                REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
                REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
                REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
                break;
            }
        }
        composer->end_if();
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
        std::vector<std::shared_ptr<zen::builder::type>> params = {class_};
        for (const auto& arg : args)
        {
            params.push_back(arg->type);
        }
        const auto callee = fun->create(name, params, nullptr);
        if (const auto result = fun->call(callee, args); not result.
            has_value())
        {
            throw zen::exceptions::semantic_error(result.error(), offset);
        }
        pragma_dangling_return_value = true;
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NSUFFIX_METHOD_CALL)
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
        std::vector<std::shared_ptr<zen::builder::type>> params = {class_};
        for (const auto& arg : args)
        {
            params.push_back(arg->type);
        }
        const auto callee = fun->create(name, params, nullptr);
        if (const auto result = fun->call(callee, args); not result.
            has_value())
        {
            throw zen::exceptions::semantic_error(result.error(), offset);
        }
        pragma_dangling_return_value = true;
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NFUNCTION_DEFINITION)
        // provides id
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
        fun = zen::builder::function::create(pool, offset, logging,
                                             class_
                                                 ? fmt::format("{}.{}", class_->name, function_name)
                                                 : function_name);
        if (class_)
            fun->set_parameter(class_, "this");
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
                    const std::string name = id;
                    REQUIRE_TERMINAL_CALLBACK(TCOLON, EXPECTED(":"))
                    type.clear();
                    REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
                    if (auto result = tab->get_type(type); result.has_value())
                    {
                        fun->set_parameter(result.value(), name);
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
        if (TRY_REQUIRE_NON_TERMINAL(NTYPE))
        {
            if (auto result = tab->get_type(type); result.has_value())
            {
                fun->set_return(result.value());
            }
            else
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            }
            lib->add(fun);
            if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
            {
                REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
                REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
            }
        }
        else
        {
            lib->add(fun);
        }
        if (TRY_REQUIRE_TERMINAL(TBRACES_OPEN))
        {
            REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
            REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
        }
        else
        {
            fun->return_value(pop());
        }
        fun->build();
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NFOR)
        // fun->loop_for()
        std::vector<std::shared_ptr<zen::builder::value>> for_params;
        REQUIRE_TERMINAL(TKEYWORD_FOR)
        REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
        auto parse_end = [&](auto fn)
        {
            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
            REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
            REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
            REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
        };
        auto handle_iterator = [&](auto fn)
        {
            REQUIRE_TERMINAL_CALLBACK(TID, EXPECTED("ID"))
            std::string iterator = tokens[offset - 1].value;
            type.clear();
            /*if (TRY_REQUIRE_TERMINAL(TCOLON))
            {
                REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
            }*/
            REQUIRE_TERMINAL(TCOLON)
            REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
            if (auto result = tab->get_type(type); result.has_value())
            {
                for_params.push_back(fun->set_local(result.value(), iterator));
            }
            else
            {
                throw zen::exceptions::semantic_error(result.error(), offset);
            }
            REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
            // composer->begin_block();
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
            for_params.push_back(pop());
            REQUIRE_TERMINAL_CALLBACK(TCOMMA, EXPECTED(","))
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
            for_params.push_back(pop());
            if (TRY_REQUIRE_TERMINAL(TCOMMA))
            {
                REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
                for_params.push_back(pop());
                // composer->end_block();
                // composer->set_for_begin_end_step();
            }
        }
        while (TRY_REQUIRE_TERMINAL(TSEMICOLON));

    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NWHILE)

        REQUIRE_TERMINAL(TKEYWORD_WHILE)
        std::vector<std::shared_ptr<zen::builder::value>> params;
        fun->loop_while(params, [&](auto _)
                        {
                            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
                            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
                            params.push_back(pop());
                            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
                        }, [&](auto _)
                        {
                            REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
                            REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
                            REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
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
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::mod");
        fun->mod(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_TIMES_VALUE)

        REQUIRE_TERMINAL(TTIMES)
        REQUIRE_NON_TERMINAL_CALLBACK(NFIRST_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::mul");
        fun->mul(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_SLASH_VALUE)

        REQUIRE_TERMINAL(TSLASH)
        REQUIRE_NON_TERMINAL_CALLBACK(NFIRST_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::div");
        fun->div(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_PLUS_VAL)

        REQUIRE_TERMINAL(TPLUS)
        REQUIRE_NON_TERMINAL_CALLBACK(NSECOND_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::add");
        fun->add(r, lhs, rhs);
        push(r);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_MINUS_VALUE)

        REQUIRE_TERMINAL(TMINUS)
        REQUIRE_NON_TERMINAL_CALLBACK(NTHIRD_PRIORITY_RHS_VAL, EXPECTED("value"))
        const auto rhs = pop();
        const auto lhs = pop();
        const auto r = fun->set_local(zen::builder::function::_bool(), "temp::sub");
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
        push(fun->constant(tokens[offset - 1].value));
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_NOT_VAL)
        REQUIRE_TERMINAL(TNOT)
        REQUIRE_NON_TERMINAL_CALLBACK(NSINGLE_VAL, EXPECTED("single value"))
        const auto it = fun->set_local(zen::builder::function::_bool(), "temp::not");
        fun->not_(it, pop());
        push(it);
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NVAL_NEGATE_VAL)
        REQUIRE_TERMINAL(TMINUS)
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
        const auto it = pop();
        fun->mul(it, it, fun->constant(-1, it->type));
        it->is_negated = true;
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
        // provides id
        if (not TRY_REQUIRE_NON_TERMINAL(NID))
        {
            ROLLBACK_PRODUCTION();
        }
        const auto name = id;
        REQUIRE_TERMINAL(TCOLON)
        type.clear();
        REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))

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
            REQUIRE_NON_TERMINAL_CALLBACK(NSUFFIX_ASGN, EXPECTED("ASSIGNMENT"))
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
            fun->return_value(pop());
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
            const auto val = fun->set_local(it->type, "temp::post_inc");
            fun->add(val, it, one);
            push(val);
        }
        else if (post_decrement)
        {
            const auto val = fun->set_local(it->type, "temp::post_dec");
            fun->sub(val, it, one);
            push(val);
        }
    END_PRODUCTION

    BEGIN_PRODUCTION(PRODUCTION_NMEMBER_ACCESS)
        REQUIRE_TERMINAL(TDOT);
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
                                                         [&]() -> std::shared_ptr<zen::builder::value>
                                                         {
                                                             return fun->set_local(zen::builder::function::_long(), id);
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

    BEGIN_SYMBOL_BINDING(NGLOBAL_STAT)
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
            PRODUCTION_NVAL_NOT_VAL() or
            PRODUCTION_NVAL_NEGATE_VAL() or
            PRODUCTION_NVAL_AS_NUM() or
            PRODUCTION_NVAL_AS_CHAR_ARRAY() or
            (PRODUCTION_NID() and (PRODUCTION_NSUFFIX_FUNCTION_CALL() or push_parser_id())) or
            PRODUCTION_NVAL_BOOLEAN() or
            PRODUCTION_NVAL_WITH_PARENTHESIS() or
            PRODUCTION_NVAL_AS_LIST()
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
          PRODUCTION_NVAL_OR_VALUE() or
        PRODUCTION_NMEMBER_ACCESS()
          // PRODUCTION_NSUFFIX_FUNCTION_CALL()
        END_SYMBOL_BINDING
    BEGIN_SYMBOL_BINDING(NSUFFIX_METHOD_CALL)
        PRODUCTION_NSUFFIX_METHOD_CALL()
    END_SYMBOL_BINDING
    END_BINDINGS

    inline bool parse()
    {
        compilation_id++;
        offset = 0;
        META_PRODUCTION_GLOBAL_STAT();
        return offset == chain_size;
    }

END_ILC_CODEGEN(builder_parser)


// }
