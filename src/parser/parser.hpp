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
static bool pragma_dangling_return_value = false;
static std::string pragma_ignore_missing_symbols_data;

// namespace parser
// {
typedef enums::token_type SYMBOL;
#include "ilc/include/ilc.hpp"

BEGIN_ILC_CODEGEN

inline zen::composer::composer* get_composer()
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
    inline std::string id, type, value;

    inline void reset()
    {
        id.clear();
        type.clear();
        value.clear();
    }
}

#define EXPECTED(ITEM) []() \
        {\
            throw zen::exceptions::syntax_error(ITEM, ILC::offset);\
        }

BEGIN_PRODUCTION(PRODUCTION_NUSING_STAT)
    REQUIRE_TERMINAL(TKEYWORD_USING)
    REQUIRE_NON_TERMINAL_CALLBACK(NID, EXPECTED("ID"))
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NCLASS)
    auto composer = get_composer();
    REQUIRE_TERMINAL(TKEYWORD_CLASS)
    REQUIRE_NON_TERMINAL_CALLBACK(NID, EXPECTED("ID"))
    auto name = parser::id;
    auto type = std::make_shared<zen::composer::type>(name, 0, zen::composer::type::kind::heap);
    composer->begin_type(type);
    if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
    {
        parser::type.clear();
    }
    REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
    bool is_static;
    while (TRY_REQUIRE_TERMINAL(TID))
    {
        std::string field_name = tokens.at(ILC::offset-1).value;
        // type->add_field(parser::id, )
        if (TRY_REQUIRE_TERMINAL(TCOLON))
        {
            parser::type.clear();
            REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
            type->add_field(field_name, composer->get_type(parser::type));
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
    composer->end_type(type);
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NIF)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TKEYWORD_IF)
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
    // bool first = true;
    /* disable extract for now
     *do
    {
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
        if (ILC::chain[ILC::offset - 1] == TID and (ILC::chain[ILC::offset - 2] == TPARENTHESIS_OPEN or not first) and
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
    auto composer = get_composer();
    const std::string name = parser::id;
    if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
    {
        parser::type.clear();
    }
    parser::id = name;
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
    pragma_dangling_return_value = composer->call(name, param_count);
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NFUNCTION_DEFINITION)
    static auto composer = get_composer();
    static bool begin_function_invoked;
    static std::list<std::tuple<std::string, std::string>> parameters = {};
    if (not parameters.empty())
    {
        parameters.clear();
    }
    // provides parser::id
REQUIRE_NON_TERMINAL(NID)

    std::string name = parser::id;
    if (name == "operator")
    {
        if (TRY_REQUIRE_TERMINAL(TEQU))
        {
            name+="=";
        } else if (TRY_REQUIRE_TERMINAL(TEQUAL))
        {
            name+="==";
        } else if (TRY_REQUIRE_TERMINAL(TNOT_EQUAL))
        {
            name+="!=";
        } else if (TRY_REQUIRE_TERMINAL(TPLUS))
        {
            name+="+";
        } else if (TRY_REQUIRE_TERMINAL(TMINUS))
        {
            name+="-";
        } else if (TRY_REQUIRE_TERMINAL(TTIMES))
        {
            name+="*";
        } else if (TRY_REQUIRE_TERMINAL(TSLASH))
        {
            name+="/";
        } else if (TRY_REQUIRE_TERMINAL(TMODULO))
        {
            name+="%";
        } else if (TRY_REQUIRE_TERMINAL(TLOWER))
        {
            name+="<";
        } else if (TRY_REQUIRE_TERMINAL(TLOWER_OR_EQUAL))
        {
            name+="<=";
        } else if (TRY_REQUIRE_TERMINAL(TGREATER))
        {
            name+=">";
        } else if (TRY_REQUIRE_TERMINAL(TGREATER_OR_EQUAL))
        {
            name+=">=";
        }  else if (TRY_REQUIRE_TERMINAL(TNOT))
        {
            name+="!";
        } else if (TRY_REQUIRE_TERMINAL(TAND))
        {
            name+="&&";
        } else if (TRY_REQUIRE_TERMINAL(TOR))
        {
            name+="||";
        }
    }

    if (TRY_REQUIRE_NON_TERMINAL(NGENERIC))
    {
        parser::type.clear();
    }
    if (TRY_REQUIRE_TERMINAL(TPARENTHESIS_OPEN))
    {
        composer->begin(name);
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
        REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
        REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
    }
    else
    {
        composer->return_value();
    }
    composer->end();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NFOR)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TKEYWORD_FOR)
    composer->begin_for();
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
    do
    {
        REQUIRE_TERMINAL_CALLBACK(TID, EXPECTED("ID"))
        std::string iterator = tokens[ILC::offset - 1].value;
        parser::type.clear();
        if (TRY_REQUIRE_TERMINAL(TCOLON))
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NTYPE, EXPECTED("TYPE"))
        }
        std::string type = parser::type;
        composer->set_local(iterator, type);
        composer->push(iterator);
        REQUIRE_TERMINAL_CALLBACK(TEQU, EXPECTED("="))
        // composer->begin_block();
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
        REQUIRE_TERMINAL_CALLBACK(TCOMMA, EXPECTED(","))
        REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
        if (TRY_REQUIRE_TERMINAL(TCOMMA))
        {
            REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("VALUE"))
            // composer->end_block();
            composer->set_for_begin_end_step();
        }
        else
        {
            // composer->end_block();
            composer->set_for_begin_end();
        }
    }
    while (TRY_REQUIRE_TERMINAL(TSEMICOLON));
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
    REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
    REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
    REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
    composer->end_for();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NWHILE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TKEYWORD_WHILE)
    composer->begin_while();
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_OPEN, EXPECTED("("))
    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
    REQUIRE_TERMINAL_CALLBACK(TPARENTHESIS_CLOSE, EXPECTED(")"))
    composer->set_while_condition();
    REQUIRE_TERMINAL_CALLBACK(TBRACES_OPEN, EXPECTED("{"))
    REQUIRE_NON_TERMINAL(NFUNCTION_BODY)
    REQUIRE_TERMINAL_CALLBACK(TBRACES_CLOSE, EXPECTED("}"))
    composer->end_while();
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
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TGREATER)
    REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->greater();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_GREATER_OR_EQUAL_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TGREATER_OR_EQUAL)
    REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->greater_or_equal();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_LOWER_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TLOWER)
    REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->lower();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_LOWER_OR_EQUAL_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TLOWER_OR_EQUAL)
    REQUIRE_NON_TERMINAL_CALLBACK(NFOURTH_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->lower_or_equal();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_EQUAL_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TEQUAL)
    REQUIRE_NON_TERMINAL_CALLBACK(NFIFTH_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->equal();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_NOT_EQUAL_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TNOT_EQUAL)
    REQUIRE_NON_TERMINAL_CALLBACK(NFIFTH_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->not_equal();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_AND_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TAND)
    REQUIRE_NON_TERMINAL_CALLBACK(NSIXTH_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->and_();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_OR_VALUE)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TOR)
    REQUIRE_NON_TERMINAL_CALLBACK(NSEVENTH_PRIORITY_RHS_VAL, EXPECTED("value"))
    composer->or_();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_AS_NUM)
    static auto composer = get_composer();
    using namespace ILC;
    if (TRY_REQUIRE_TERMINAL(TINT_NUM))
    {
        int data = strtol(tokens[ILC::offset - 1].value.c_str(), nullptr, 10);
        composer->push<int>(std::move(data), "int");
    }
    else if (TRY_REQUIRE_TERMINAL(TBYTE_NUM))
    {
        char data = static_cast<char>(strtol(tokens[ILC::offset - 1].value.c_str(), nullptr, 10));
        composer->push<char>(std::move(data), "byte");
    }
    else if (TRY_REQUIRE_TERMINAL(TSHORT_NUM))
    {
        short data = static_cast<short>(strtol(tokens[ILC::offset - 1].value.c_str(), nullptr, 10));
        composer->push<short>(std::move(data), "short");
    }
    else if (TRY_REQUIRE_TERMINAL(TLONG_NUM))
    {
        long data = strtol(tokens[ILC::offset - 1].value.c_str(), nullptr, 10);
        composer->push<long>(std::move(data), "long");
    }
    else if (TRY_REQUIRE_TERMINAL(TFLOAT_NUM))
    {
        float data = strtof(tokens[ILC::offset - 1].value.c_str(), nullptr);
        composer->push<float>(std::move(data), "float");
    }
    else
    {
        REQUIRE_TERMINAL(TDOUBLE_NUM)
        double data = strtod(tokens[ILC::offset - 1].value.c_str(), nullptr);
        composer->push<double>(std::move(data), "double");
    }
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_AS_CHAR_ARRAY)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TCHAR_ARRAY)
    composer->push<zen::types::heap::string*>(zen::types::heap::string::from_string(tokens[ILC::offset - 1].value),
                                              "string");
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_NOT_VAL)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TNOT)
    REQUIRE_NON_TERMINAL_CALLBACK(NSINGLE_VAL, EXPECTED("single value"))
    composer->not_();
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NVAL_NEGATE_VAL)
    static auto composer = get_composer();
    REQUIRE_TERMINAL(TMINUS)
    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
    composer->negate();
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
    if (not TRY_REQUIRE_NON_TERMINAL(NID))
    {
        pragma_ignore_missing_symbols = false;
        ROLLBACK_PRODUCTION();
    }
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
    REQUIRE_TERMINAL(TEQU);
    REQUIRE_NON_TERMINAL_CALLBACK(NVAL, EXPECTED("value"))
    composer->assign();
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
    static auto composer = get_composer();
    bool dangling_value = false;
    while (true)
    {
        if (TRY_REQUIRE_NON_TERMINAL(NSTAT))
        {
            if (dangling_value)
            {
                composer->pop();
                dangling_value = false;
            }
            continue;
        }
        if (TRY_REQUIRE_NON_TERMINAL(NID))
        {
            if (dangling_value)
                composer->pop();
            composer->push(parser::id);
            if (TRY_REQUIRE_NON_TERMINAL(NSUFFIX_FUNCTION_CALL))
            {
                dangling_value = pragma_dangling_return_value;
                if (dangling_value)
                    TRY_REQUIRE_NON_TERMINAL(NENDLESS_SUFIXES);
                continue;
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
        composer->return_value();
    }
END_PRODUCTION

BEGIN_PRODUCTION(PRODUCTION_NSTAT_FROM_ASGN)
    static zen::composer::composer* composer = get_composer();
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
    static auto composer = get_composer();
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
    if (pre_increment)
        composer->pre_increment();
    else if (pre_decrement)
        composer->pre_decrement();
    else if (post_increment)
        composer->post_increment();
    else if (post_decrement)
        composer->post_decrement();
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

inline bool push_parser_id()
    {
        get_composer()->push(parser::id);
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

BEGIN_SYMBOL_BINDING(NSUFFIX_FUNCTION_DEFINITION)
    PRODUCTION_NFUNCTION_DEFINITION()
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
          PRODUCTION_NVAL_OR_VALUE()
          // PRODUCTION_NSUFFIX_FUNCTION_CALL()
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
