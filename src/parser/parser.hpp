//
// Created by dte on 2/24/2025.
//
#pragma once
#include "enums/token_type.hpp"
#include <vector>
#include <iostream>
#include "composer/composer.hpp"
#include "composer/vm/composer.hpp"
#include "exceptions/semantic_error.hpp"
#include "exceptions/syntax_error.hpp"
#include "types/stack.hpp"
#include "types/heap.hpp"

using namespace enums;
extern std::vector<zen::token> tokens;
static bool pragma_ignore_missing_symbols = false;
static bool pragma_skip_assignment_because_of_conversion_special_call = false;
static std::string pragma_ignore_missing_symbols_data;

// namespace parser
// {
typedef enums::token_type SYMBOL;
#include "ilc/include/ilc.hpp"

BEGIN_ILC_CODEGEN

zen::composer::composer* get_composer()
{
    static auto composer = std::make_unique<zen::composer::vm::composer>(ILC::offset);
    return (zen::composer::composer*)(composer.get());
}

namespace parser
{
    enum state_t
    {
        STATEMENT,
        DEFAULT = STATEMENT,
        VARIABLE,
        CONSTANT
    };

    // state_t state = STATEMENT;
    std::string id, type, value;

    inline void reset()
    {
        id.clear();
        type.clear();
        value.clear();
    }
}

// std::cerr <<  "syntax error: expected " << ITEM << " found " << tokens[ILC::offset].value << " @" << (int)tokens[ILC::offset].type << " at " << tokens[ILC::offset-1].get_location_string();

#define EXPECTED(ITEM) []() \
        {\
            throw zen::exceptions::syntax_error(ITEM, ILC::offset);\
        }

BEGIN_PRODUCTION(PRODUCTION_NUSING_STAT)
    REQUIRE_TERMINAL(TKEYWORD_USING)
    REQUIRE_NON_TERMINAL_CALLBACK(NID, EXPECTED("ID"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NCLASS)
    REQUIRE_TERMINAL(TKEYWORD_CLASS)
    REQUIRE_NON_TERMINAL_CALLBACK(NID, EXPECTED("ID"))
    if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
    {
        parser::type.clear();
    }
    REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
    bool is_static;
    while (TRY_REQUIRE_TERMINAL(TID) or ((is_static = TRY_REQUIRE_TERMINAL(TKEYWORD_STATIC))) and
        TRY_REQUIRE_TERMINAL(TID))
    {
        if (TRY_REQUIRE_TERMINAL(TCOLON))
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
            if (TRY_REQUIRE_TERMINAL(TEQU))
            {
                REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
            }
        }
        else
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NSUFFIX_FUNCTION_DEFINITION, EXPECTED("FUNCTION"))
        }
    }
    REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NIF)
    REQUIRE_TERMINAL(TKEYWORD_IF)
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
    bool first = true;
    do
    {
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
        if (ILC::chain[ILC::offset - 1] == TID and (ILC::chain[ILC::offset - 2] == TPARENTHESIS_OPEN or not first) and
            TRY_REQUIRE_TERMINAL(TCOLON))
        {
            first = false;
            REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
            REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
            REQUIRE_NON_TERMINAL_CALLBACK(NSINGLE_VAL, EXPECTED("VALUE"))
        }
    }
    while (TRY_REQUIRE_TERMINAL(TAND));
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
    REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
    while (TRY_REQUIRE_NON_TERMINAL(NSTAT))
    {
    }
    REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
    while (TRY_REQUIRE_TERMINAL(TKEYWORD_ELSE))
    {
        if (TRY_REQUIRE_TERMINAL(TKEYWORD_IF))
        {
            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
            first = true;
            do
            {
                REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
                if (ILC::chain[ILC::offset - 1] == TID and (ILC::chain[ILC::offset - 2] == TPARENTHESIS_OPEN or not
                    first) and TRY_REQUIRE_TERMINAL(TCOLON))
                {
                    first = false;
                    REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
                    REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
                    REQUIRE_NON_TERMINAL_CALLBACK(NSINGLE_VAL, EXPECTED("VALUE"))
                }
            }
            while (TRY_REQUIRE_TERMINAL(TAND));
            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
            REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
            while (TRY_REQUIRE_NON_TERMINAL(NSTAT))
            {
            }
            REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
        }
        else
        {
            REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
            while (TRY_REQUIRE_NON_TERMINAL(NSTAT))
            {
            }
            REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
            break;
        }
    }
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NSUFFIX_FUNCTION_CALL)
    auto composer = get_composer();
    bool assignment_call = ILC::offset > 2 && ILC::chain[ILC::offset - 2] == TEQU;
    const std::string name = parser::id;
    if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
    {
        parser::type.clear();
    }
    REQUIRE_TERMINAL(TPARENTHESIS_OPEN)
    zen::i8 param_count = 0;
    while (TRY_REQUIRE_NON_TERMINAL(NVAL))
    {
        param_count++;
        if (not TRY_REQUIRE_TERMINAL(TCOMMA))
        {
            break;
        }
    }
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
    pragma_skip_assignment_because_of_conversion_special_call = composer->call(
        name, assignment_call ? param_count : -param_count) == zen::composer::call_result::casting && assignment_call;
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NFUNCTION_SUFFIX)
    static auto composer = get_composer();
    static bool begin_function_invoked;
    static std::list<std::tuple<std::string, std::string>> parameters = {};
    if (not parameters.empty())
    {
        parameters.clear();
    }
    // provides parser::id
    // REQUIRE_NON_TERMINAL(NID)
    if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
    {
        parser::type.clear();
    }
    if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
    {
        composer->begin(parser::id);
        begin_function_invoked = true;
        bool first_it = true;
        do
        {
            if (TRY_REQUIRE_NON_TERMINAL(NID))
            {
                const std::string name = parser::id;
                REQUIRE_TERMINAL_CALLBACK(TCOLON, EXPECTED(":"))
                parser::type.clear();
                REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
                parameters.emplace_back(name, parser::type);
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
    else
    {
        begin_function_invoked = false;
    }
    REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
    if (not begin_function_invoked)
    {
        composer->begin(parser::id);
    }
    parser::type.clear();
    if (TRY_REQUIRE_NON_TERMINAL(NTYPE))
    {
        composer->set_return_type(parser::type);
        for (auto parameter : parameters)
        {
            composer->set_parameter(std::get<0>(parameter), std::get<1>(parameter));
        }
        parameters.clear();
        if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
        {
            pragma_ignore_missing_symbols = true;
            pragma_ignore_missing_symbols_data.clear();
            const int offset_before = ILC::offset;
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
            pragma_ignore_missing_symbols = false;
            if (ILC::offset - offset_before == 1 and ILC::chain[ILC::offset - 1] == TID)
            {
                composer->set_return_name(parser::id);
            }
            else if (not pragma_ignore_missing_symbols_data.empty())
            {
                throw zen::exceptions::semantic_error(fmt::format(
                                                          "no such symbol(s) {}", pragma_ignore_missing_symbols_data),
                                                      ILC::offset);
            }
            REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
        }
    }
    else
    {
        composer->set_return_type("unit");
        for (auto parameter : parameters)
        {
            composer->set_parameter(std::get<0>(parameter), std::get<1>(parameter));
        }
        parameters.clear();
    }
    if (TRY_REQUIRE_TERMINAL(TBRACES_OPEN))
    {
        while (TRY_REQUIRE_NON_TERMINAL(NSTAT))
        {
        } // improve return handler
        if (TRY_REQUIRE_NON_TERMINAL(NVAL))
        {
            composer->return_value();
        }
        REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
    }
    else
    {
        composer->return_value();
    }
    composer->end();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NFOR)
    REQUIRE_TERMINAL(TKEYWORD_FOR)
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
    do
    {
        REQUIRE_TERMINAL_CALLBACK(TID, EXPECTED("ID"))
        if (TRY_REQUIRE_TERMINAL(TCOLON))
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
        }
        REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
        REQUIRE_TERMINAL_CALLBACK(TCOMMA, EXPECTED(","))
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
        if (TRY_REQUIRE_TERMINAL(TCOMMA))
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
        }
    }
    while (TRY_REQUIRE_TERMINAL(TSEMICOLON));
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
    REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
    while (TRY_REQUIRE_NON_TERMINAL(NSTAT))
    {
    }
    REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NWHILE)
    REQUIRE_TERMINAL(TKEYWORD_WHILE)
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
    REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
    while (TRY_REQUIRE_NON_TERMINAL(NSTAT))
    {
    }
    REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_PREFIX_VAL)
    REQUIRE_NON_TERMINAL(NSINGLE_VAL)
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_MOD_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TMODULO)
    REQUIRE_NON_TERMINAL_CALLBACK(NFIRST_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->modulo();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_TIMES_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TTIMES)
    REQUIRE_NON_TERMINAL_CALLBACK(NFIRST_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->times();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_SLASH_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TSLASH)
    REQUIRE_NON_TERMINAL_CALLBACK(NFIRST_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->slash();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_PLUS_VAL)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TPLUS)
    REQUIRE_NON_TERMINAL_CALLBACK(NSECOND_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->plus();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_MINUS_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TMINUS)
    REQUIRE_NON_TERMINAL_CALLBACK(NTHIRD_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->minus();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_GREATER_VALUE)
    REQUIRE_TERMINAL(TGREATER)
    REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE)
    REQUIRE_TERMINAL(TGREATER_OR_EQUAL)
    REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_LOWER_VALUE)
    REQUIRE_TERMINAL(TLOWER)
    REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE)
    REQUIRE_TERMINAL(TLOWER_OR_EQUAL)
    REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_EQUAL_VALUE)
    REQUIRE_TERMINAL(TEQUAL)
    REQUIRE_NON_TERMINAL_CALLBACK(NFIFTH_PRIORITY_RHS_VAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_NOT_EQUAL_VALUE)
    REQUIRE_TERMINAL(TNOT_EQUAL)
    REQUIRE_NON_TERMINAL_CALLBACK(NFIFTH_PRIORITY_RHS_VAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_AND_VALUE)
    REQUIRE_TERMINAL(TAND)
    REQUIRE_NON_TERMINAL_CALLBACK(NSIXTH_PRIORITY_RHS_VAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_OR_VALUE)
    REQUIRE_TERMINAL(TOR)
    REQUIRE_NON_TERMINAL_CALLBACK(NSEVENTH_PRIORITY_RHS_VAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_AS_NUM)
    static auto composer = get_composer();
    bool negative = false;
    if (TRY_REQUIRE_TERMINAL(TMINUS))
        negative = true;
    if (TRY_REQUIRE_TERMINAL(TPLUS))
        negative = false;
    using namespace ILC;
    if (TRY_REQUIRE_TERMINAL(TINT_NUM))
    {
        int data = strtol(tokens[ILC::offset - 1].value.c_str(), nullptr, 10);
        if (negative)
        {
            data = -data;
        }
        composer->push<int>(std::move(data), "int");
    }
    else if (TRY_REQUIRE_TERMINAL(TBYTE_NUM))
    {
        char data = static_cast<char>(strtol(tokens[ILC::offset - 1].value.c_str(), nullptr, 10));
        if (negative)
        {
            data = -data;
        }
        composer->push<char>(std::move(data), "byte");
    }
    else if (TRY_REQUIRE_TERMINAL(TSHORT_NUM))
    {
        short data = static_cast<short>(strtol(tokens[ILC::offset - 1].value.c_str(), nullptr, 10));
        if (negative)
        {
            data = -data;
        }
        composer->push<short>(std::move(data), "short");
    }
    else if (TRY_REQUIRE_TERMINAL(TLONG_NUM))
    {
        long data = strtol(tokens[ILC::offset - 1].value.c_str(), nullptr, 10);
        if (negative)
        {
            data = -data;
        }
        composer->push<long>(std::move(data), "long");
    }
    else if (TRY_REQUIRE_TERMINAL(TFLOAT_NUM))
    {
        float data = strtof(tokens[ILC::offset - 1].value.c_str(), nullptr);
        if (negative)
        {
            data = -data;
        }
        composer->push<float>(std::move(data), "float");
    }
    else
    {
        REQUIRE_TERMINAL(TDOUBLE_NUM)
        double data = strtod(tokens[ILC::offset - 1].value.c_str(), nullptr);
        if (negative)
        {
            data = -data;
        }
        composer->push<double>(std::move(data), "double");
    }
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_AS_CHAR_ARRAY)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TCHAR_ARRAY)
    composer->push<zen::types::heap::string*>(zen::types::heap::string::make(tokens[ILC::offset - 1].value), "string");
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_NOT_VAL)
    REQUIRE_TERMINAL(TNOT)
    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NSUFFIX_VAL_NTERNARY)
    REQUIRE_TERMINAL(TQUESTION)
    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
    REQUIRE_TERMINAL(TCOLON)
    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
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

BEGIN_PRODUCTION(PRODUCTION_NVAL_AS_ID)
    static auto composer = get_composer();
    bool negative = false;
    if (TRY_REQUIRE_TERMINAL(TMINUS))
        negative = true;
    if (TRY_REQUIRE_TERMINAL(TPLUS))
        negative = false;
    REQUIRE_NON_TERMINAL(NID)
    try
    {
        composer->push(parser::id);
    }
    catch (const zen::exceptions::semantic_error& e)
    {
        if (not pragma_ignore_missing_symbols)
            throw;
        if (not pragma_ignore_missing_symbols_data.empty())
            pragma_ignore_missing_symbols_data += ", ";
        pragma_ignore_missing_symbols_data += parser::id;
    }
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_BOOLEAN)
    static auto composer = get_composer();
    if (not TRY_REQUIRE_TERMINAL(TKEYWORD_TRUE))
    {
        REQUIRE_TERMINAL(TKEYWORD_FALSE)
        composer->push<bool>(false, "bool");
    }
    else
    {
        composer->push<bool>(true, "bool");
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
    parser::type += tokens[ILC::offset - 1].value;
    if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
    {
        parser::type.clear();
    }
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVARIABLE_DEFINITION)
    static auto composer = get_composer();
    // provides parser::id
    pragma_ignore_missing_symbols = true;
    REQUIRE_NON_TERMINAL(NID)
    pragma_ignore_missing_symbols = false;
    const auto name = parser::id;
    REQUIRE_TERMINAL(TCOLON)
    parser::type.clear();
    REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
    composer->set_local(name, parser::type);
    if (TRY_REQUIRE_TERMINAL(TEQU))
    {
        composer->push(name);
        ILC::offset--;
        REQUIRE_NON_TERMINAL_CALLBACK(NSUFFIX_ASGN, EXPECTED("ASSIGNMENT"))
    }
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NASGN_SUFFIX)
    static auto composer = get_composer();
    if (not(TRY_REQUIRE_TERMINAL(TEQU) or
        TRY_REQUIRE_TERMINAL(TPLUS_EQU) or
        TRY_REQUIRE_TERMINAL(TMINUS_EQU) or
        TRY_REQUIRE_TERMINAL(TTIMES_EQU) or
        TRY_REQUIRE_TERMINAL(TSLASH_EQU) or
        TRY_REQUIRE_TERMINAL(TMINUS_EQU)))
    {
        ROLLBACK_PRODUCTION()
    }
    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
    if (not pragma_skip_assignment_because_of_conversion_special_call)
        composer->assign();
    else
        pragma_skip_assignment_because_of_conversion_special_call = false;
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

BEGIN_PRODUCTION(PRODUCTION_NSTAT_FROM_ASGN)
    static auto composer = get_composer();
    REQUIRE_NON_TERMINAL(NID)
    if (TRY_REQUIRE_TERMINAL(TEQU))
    {
        composer->push(parser::id);
        ILC::offset--;
        REQUIRE_NON_TERMINAL(NSUFFIX_ASGN)
    }
    else
    {
        ROLLBACK_PRODUCTION()
    }
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NSINGLE_VAL)
    const bool pre_buffer = TRY_REQUIRE_TERMINAL(TPLUS_PLUS) or TRY_REQUIRE_TERMINAL(TMINUS_MINUS);
    REQUIRE_NON_TERMINAL(NSINGLE_VAL_PREDICATE)
    if (not pre_buffer)
        TRY_REQUIRE_TERMINAL(TPLUS_PLUS) or TRY_REQUIRE_TERMINAL(TMINUS_MINUS);
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NID)
    parser::id.clear();
    REQUIRE_TERMINAL(TID)
    parser::id += tokens[ILC::offset - 1].value;
    while (TRY_REQUIRE_TERMINAL(TDOT))
    {
        if (TRY_REQUIRE_TERMINAL(TTIMES))
        {
            parser::id += "." + tokens[ILC::offset - 1].value;
            break;
        }
        REQUIRE_TERMINAL_CALLBACK(TID, EXPECTED("identifier"))
        parser::id += "." + tokens[ILC::offset - 1].value;
    }
END_PRODUCTION

BEGIN_BINDINGS
BEGIN_SYMBOL_BINDING(NOR_VAL)
            PRODUCTION_NVAL_OR_VALUE()
        END_SYMBOL_BINDING

BEGIN_SYMBOL_BINDING(NAND_VAL)
            PRODUCTION_NVAL_AND_VALUE()
        END_SYMBOL_BINDING

BEGIN_SYMBOL_BINDING(NSUFFIX_VAL_NTERNARY)
            PRODUCTION_NSUFFIX_VAL_NTERNARY()
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

BEGIN_SYMBOL_BINDING(NSUFFIX_FUNCTION_DEFINITION)
    PRODUCTION_NFUNCTION_SUFFIX()
END_SYMBOL_BINDING

BEGIN_SYMBOL_BINDING(NGLOBAL_STAT)
        (PRODUCTION_NID() and PRODUCTION_NFUNCTION_SUFFIX()) or
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
                PRODUCTION_NSUFFIX_FUNCTION_CALL() or
                true)
        ) and PRODUCTION_ENDLESS_SUFFIXES()
        END_SYMBOL_BINDING

BEGIN_SYMBOL_BINDING(NSINGLE_VAL_PREDICATE)
            PRODUCTION_NVAL_NOT_VAL() or
            PRODUCTION_NVAL_AS_NUM() or
            PRODUCTION_NVAL_AS_CHAR_ARRAY() or
        (PRODUCTION_NVAL_AS_ID() and (PRODUCTION_NSUFFIX_FUNCTION_CALL() or true)) or
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
          PRODUCTION_NVAL_NOT_VAL() or
          PRODUCTION_NVAL_AND_VALUE() or
          PRODUCTION_NVAL_OR_VALUE() or
          PRODUCTION_NSUFFIX_VAL_NTERNARY() or PRODUCTION_NSUFFIX_FUNCTION_CALL()
        END_SYMBOL_BINDING
END_BINDINGS

END_ILC_CODEGEN

inline bool parse()
{
    ILC::compilation_id++;
    ILC::offset = 0;
    META_PRODUCTION_GLOBAL_STAT();
    return ILC::offset == ILC::chain_size;
}

// }
