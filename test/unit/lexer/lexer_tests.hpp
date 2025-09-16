//
// Created by dte on 9/15/2025.
//
#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

TEST(lexer_unit, numbers)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream0;
    stream0.str(R"(11 22 33 44.3 1)");
    zen::lexer lexer(stream0);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TINT_NUM, enums::TINT_NUM, enums::TDOUBLE_NUM, enums::TINT_NUM}));
    EXPECT_EQ(tokens[0].value, "11");
    EXPECT_EQ(tokens[1].value, "22");
    EXPECT_EQ(tokens[2].value, "33");
    EXPECT_EQ(tokens[3].value, "44.3");
    EXPECT_EQ(tokens[4].value, "1");
}

TEST(lexer_unit, numbers_with_underscores_and_suffixes)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(1_000 123.45_67f 99b 1000s 123456789i 9876543210l)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TFLOAT_NUM, enums::TBYTE_NUM, enums::TSHORT_NUM, enums::TINT_NUM, enums::TLONG_NUM}));
    EXPECT_EQ(tokens[0].value, "1000");
    EXPECT_EQ(tokens[1].value, "123.4567");
    EXPECT_EQ(tokens[2].value, "99");
    EXPECT_EQ(tokens[3].value, "1000");
    EXPECT_EQ(tokens[4].value, "123456789");
    EXPECT_EQ(tokens[5].value, "9876543210");
}

TEST(lexer_unit, strings_and_escape_sequences)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream2;
    stream2.str(R"("hello" "world\n" "tab\t" "quote\"" "backslash\\")");
    zen::lexer lexer(stream2);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TCHAR_ARRAY, enums::TCHAR_ARRAY, enums::TCHAR_ARRAY, enums::TCHAR_ARRAY, enums::TCHAR_ARRAY}));
    EXPECT_EQ(tokens[0].value, "hello");
    EXPECT_EQ(tokens[1].value, "world\n");
    EXPECT_EQ(tokens[2].value, "tab\t");
    EXPECT_EQ(tokens[3].value, "quote\"");
    EXPECT_EQ(tokens[4].value, "backslash\\");
}

TEST(lexer_unit, operators)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream3;
    stream3.str(R"(!= == = > >= < <= + ++ += - -- -= * *= / /= % %= && ||)");
    zen::lexer lexer(stream3);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TNOT_EQUAL, enums::TEQUAL, enums::TEQU, enums::TGREATER, enums::TGREATER_OR_EQUAL,
        enums::TLOWER, enums::TLOWER_OR_EQUAL, enums::TPLUS, enums::TPLUS_PLUS, enums::TPLUS_EQU,
        enums::TMINUS, enums::TMINUS_MINUS, enums::TMINUS_EQU, enums::TTIMES, enums::TTIMES_EQU,
        enums::TSLASH, enums::TSLASH_EQU, enums::TMODULO, enums::TMODULO_EQU, enums::TAND, enums::TOR
    }));
}

TEST(lexer_unit, keywords_and_identifiers)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream4;
    stream4.str(R"(
        for while if else using break continue class static
        myVariable my_function anotherID
        true false
    )");
    zen::lexer lexer(stream4);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TKEYWORD_FOR, enums::TKEYWORD_WHILE, enums::TKEYWORD_IF, enums::TKEYWORD_ELSE,
        enums::TKEYWORD_USING, enums::TKEYWORD_BREAK, enums::TKEYWORD_CONTINUE, enums::TKEYWORD_CLASS,
        enums::TKEYWORD_STATIC, enums::TID, enums::TID, enums::TID, enums::TKEYWORD_TRUE, enums::TKEYWORD_FALSE
    }));
    EXPECT_EQ(tokens[9].value, "myVariable");
    EXPECT_EQ(tokens[10].value, "my_function");
    EXPECT_EQ(tokens[11].value, "anotherID");
}

TEST(lexer_unit, single_character_tokens)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream5;
    stream5.str(R"({ } [ ] ( ) . , ? : ;)");
    zen::lexer lexer(stream5);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TBRACES_OPEN, enums::TBRACES_CLOSE, enums::TBRACKETS_OPEN, enums::TBRACKETS_CLOSE,
        enums::TPARENTHESIS_OPEN, enums::TPARENTHESIS_CLOSE, enums::TDOT, enums::TCOMMA,
        enums::TQUESTION, enums::TCOLON, enums::TSEMICOLON
    }));
}

TEST(lexer_unit, comments)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream6;
    stream6.str(R"(
        // this is a single line comment
        123 // another one
        /* This is a
        multi-line comment */
        456
        /* nested /* comment */ test */
        789
    )");
    zen::lexer lexer(stream6);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TINT_NUM, enums::TINT_NUM, enums::TINT_NUM
    }));
    EXPECT_EQ(tokens[0].value, "123");
    EXPECT_EQ(tokens[1].value, "456");
    EXPECT_EQ(tokens[2].value, "789");
}

TEST(lexer_unit, mixed_content)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream7;
    stream7.str(R"(
        if (x == 100) {
            // Check if x is 100
            my_function(x += 5);
        } else {
            20.5;
        }
    )");
    zen::lexer lexer(stream7);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TKEYWORD_IF, enums::TPARENTHESIS_OPEN, enums::TID, enums::TEQUAL,
        enums::TINT_NUM, enums::TPARENTHESIS_CLOSE, enums::TBRACES_OPEN, enums::TID,
        enums::TPARENTHESIS_OPEN, enums::TID, enums::TPLUS_EQU, enums::TINT_NUM,
        enums::TPARENTHESIS_CLOSE, enums::TSEMICOLON, enums::TBRACES_CLOSE,
        enums::TKEYWORD_ELSE, enums::TBRACES_OPEN,
        enums::TDOUBLE_NUM, enums::TSEMICOLON, enums::TBRACES_CLOSE
    }));
    EXPECT_EQ(tokens[2].value, "x");
    EXPECT_EQ(tokens[4].value, "100");
    EXPECT_EQ(tokens[7].value, "my_function");
    EXPECT_EQ(tokens[9].value, "x");
    EXPECT_EQ(tokens[11].value, "5");
    EXPECT_EQ(tokens[17].value, "20.5");
}

// Zen Lexer Unit Tests

TEST(lexer_unit, basic_integers)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(123 456 0 999)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TINT_NUM, enums::TINT_NUM, enums::TINT_NUM}));
    EXPECT_EQ(tokens[0].value, "123");
    EXPECT_EQ(tokens[1].value, "456");
    EXPECT_EQ(tokens[2].value, "0");
    EXPECT_EQ(tokens[3].value, "999");
}

TEST(lexer_unit, numbers_with_underscores)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(1_000 1_234_567 42_0)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TINT_NUM, enums::TINT_NUM}));
    EXPECT_EQ(tokens[0].value, "1000");
    EXPECT_EQ(tokens[1].value, "1234567");
    EXPECT_EQ(tokens[2].value, "420");
}

TEST(lexer_unit, double_numbers)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(123.456 0.5 999.0)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TDOUBLE_NUM, enums::TDOUBLE_NUM, enums::TDOUBLE_NUM}));
    EXPECT_EQ(tokens[0].value, "123.456");
    EXPECT_EQ(tokens[1].value, "0.5");
    EXPECT_EQ(tokens[2].value, "999.0");
}

TEST(lexer_unit, float_numbers_with_suffix)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(123.45f 0.5f 999f)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TFLOAT_NUM, enums::TFLOAT_NUM, enums::TFLOAT_NUM}));
    EXPECT_EQ(tokens[0].value, "123.45");
    EXPECT_EQ(tokens[1].value, "0.5");
    EXPECT_EQ(tokens[2].value, "999");
}

TEST(lexer_unit, double_numbers_with_suffix)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(123.45d 0.5d 999d)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TDOUBLE_NUM, enums::TDOUBLE_NUM, enums::TDOUBLE_NUM}));
    EXPECT_EQ(tokens[0].value, "123.45");
    EXPECT_EQ(tokens[1].value, "0.5");
    EXPECT_EQ(tokens[2].value, "999");
}

TEST(lexer_unit, integer_type_suffixes)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(123b 456s 789i 999l)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TBYTE_NUM, enums::TSHORT_NUM, enums::TINT_NUM, enums::TLONG_NUM}));
    EXPECT_EQ(tokens[0].value, "123");
    EXPECT_EQ(tokens[1].value, "456");
    EXPECT_EQ(tokens[2].value, "789");
    EXPECT_EQ(tokens[3].value, "999");
}

TEST(lexer_unit, numbers_with_underscores_and_suffixes_2)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(1_000 123.45_67f 99b 1000s 123456789i 9876543210l)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TFLOAT_NUM, enums::TBYTE_NUM, enums::TSHORT_NUM, enums::TINT_NUM, enums::TLONG_NUM}));
    EXPECT_EQ(tokens[0].value, "1000");
    EXPECT_EQ(tokens[1].value, "123.4567");
    EXPECT_EQ(tokens[2].value, "99");
    EXPECT_EQ(tokens[3].value, "1000");
    EXPECT_EQ(tokens[4].value, "123456789");
    EXPECT_EQ(tokens[5].value, "9876543210");
}

TEST(lexer_unit, basic_strings)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"("hello" "world" "test")");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TCHAR_ARRAY, enums::TCHAR_ARRAY, enums::TCHAR_ARRAY}));
    EXPECT_EQ(tokens[0].value, "hello");
    EXPECT_EQ(tokens[1].value, "world");
    EXPECT_EQ(tokens[2].value, "test");
}

TEST(lexer_unit, strings_with_escape_sequences)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"("hello\nworld" "tab\there" "quote\"test" "backslash\\")");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TCHAR_ARRAY, enums::TCHAR_ARRAY, enums::TCHAR_ARRAY, enums::TCHAR_ARRAY}));
    EXPECT_EQ(tokens[0].value, "hello\nworld");
    EXPECT_EQ(tokens[1].value, "tab\there");
    EXPECT_EQ(tokens[2].value, "quote\"test");
    EXPECT_EQ(tokens[3].value, "backslash\\");
}

TEST(lexer_unit, empty_string)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"("")");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TCHAR_ARRAY}));
    EXPECT_EQ(tokens[0].value, "");
}

TEST(lexer_unit, keywords)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(for using while if else true false break continue class static)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TKEYWORD_FOR, enums::TKEYWORD_USING, enums::TKEYWORD_WHILE, enums::TKEYWORD_IF,
        enums::TKEYWORD_ELSE, enums::TKEYWORD_TRUE, enums::TKEYWORD_FALSE, enums::TKEYWORD_BREAK,
        enums::TKEYWORD_CONTINUE, enums::TKEYWORD_CLASS, enums::TKEYWORD_STATIC
    }));
}

TEST(lexer_unit, identifiers)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(myVar test123 _private variable_name CamelCase)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TID, enums::TID, enums::TID, enums::TID, enums::TID}));
    EXPECT_EQ(tokens[0].value, "myVar");
    EXPECT_EQ(tokens[1].value, "test123");
    EXPECT_EQ(tokens[2].value, "_private");
    EXPECT_EQ(tokens[3].value, "variable_name");
    EXPECT_EQ(tokens[4].value, "CamelCase");
}

TEST(lexer_unit, punctuation_tokens)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"({ } [ ] ( ) . , ? : ;)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TBRACES_OPEN, enums::TBRACES_CLOSE, enums::TBRACKETS_OPEN, enums::TBRACKETS_CLOSE,
        enums::TPARENTHESIS_OPEN, enums::TPARENTHESIS_CLOSE, enums::TDOT, enums::TCOMMA,
        enums::TQUESTION, enums::TCOLON, enums::TSEMICOLON
    }));
}

TEST(lexer_unit, assignment_operators)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(= == += -= *= /= %=)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TEQU, enums::TEQUAL, enums::TPLUS_EQU, enums::TMINUS_EQU,
        enums::TTIMES_EQU, enums::TSLASH_EQU, enums::TMODULO_EQU
    }));
}

TEST(lexer_unit, arithmetic_operators)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(+ - * / % ++ --)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TPLUS, enums::TMINUS, enums::TTIMES, enums::TSLASH, enums::TMODULO,
        enums::TPLUS_PLUS, enums::TMINUS_MINUS
    }));
}

TEST(lexer_unit, comparison_operators)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(> >= < <= != !)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TGREATER, enums::TGREATER_OR_EQUAL, enums::TLOWER, enums::TLOWER_OR_EQUAL,
        enums::TNOT_EQUAL, enums::TNOT
    }));
}

TEST(lexer_unit, logical_operators)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(&& ||)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TAND, enums::TOR}));
}

TEST(lexer_unit, single_line_comments)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str("123 // this is a comment\n456");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TINT_NUM}));
    EXPECT_EQ(tokens[0].value, "123");
    EXPECT_EQ(tokens[1].value, "456");
}

TEST(lexer_unit, multi_line_comments)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str("123 /* this is a \n multi-line comment */ 456");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TINT_NUM}));
    EXPECT_EQ(tokens[0].value, "123");
    EXPECT_EQ(tokens[1].value, "456");
}

TEST(lexer_unit, nested_multi_line_comments)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str("123 /* outer /* inner */ outer */ 456");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TINT_NUM}));
    EXPECT_EQ(tokens[0].value, "123");
    EXPECT_EQ(tokens[1].value, "456");
}

TEST(lexer_unit, whitespace_handling)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str("  123   \t\n  456  \r\n  789  ");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TINT_NUM, enums::TINT_NUM}));
    EXPECT_EQ(tokens[0].value, "123");
    EXPECT_EQ(tokens[1].value, "456");
    EXPECT_EQ(tokens[2].value, "789");
}

TEST(lexer_unit, mixed_tokens)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(if (x >= 10.5f) { return "hello"; })");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{
        enums::TKEYWORD_IF, enums::TPARENTHESIS_OPEN, enums::TID, enums::TGREATER_OR_EQUAL,
        enums::TFLOAT_NUM, enums::TPARENTHESIS_CLOSE, enums::TBRACES_OPEN, enums::TID,
        enums::TCHAR_ARRAY, enums::TSEMICOLON, enums::TBRACES_CLOSE
    }));
    EXPECT_EQ(tokens[2].value, "x");
    EXPECT_EQ(tokens[4].value, "10.5");
    EXPECT_EQ(tokens[7].value, "return");
    EXPECT_EQ(tokens[8].value, "hello");
}

TEST(lexer_unit, error_tokens)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str(R"(123 @ 456 # 789)");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain, (std::vector<enums::token_type>{enums::TINT_NUM, enums::TERROR, enums::TINT_NUM, enums::TERROR, enums::TINT_NUM}));
    EXPECT_EQ(tokens[0].value, "123");
    EXPECT_EQ(tokens[1].value, "@");
    EXPECT_EQ(tokens[2].value, "456");
    EXPECT_EQ(tokens[3].value, "#");
    EXPECT_EQ(tokens[4].value, "789");
}

TEST(lexer_unit, empty_input)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream1;
    stream1.str("");
    zen::lexer lexer(stream1);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    EXPECT_EQ(ILC::chain.size(), 0);
    EXPECT_EQ(tokens.size(), 0);
}