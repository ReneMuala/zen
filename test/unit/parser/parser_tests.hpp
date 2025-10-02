//
// Created by dte on 9/16/2025.
//

#pragma once
#include <gtest/gtest.h>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

inline void setup_parser_test(const std::string& code) {
    ILC::chain.clear();
    tokens.clear();
    parser::reset();
    std::stringstream stream(code);
    zen::lexer lexer(stream);
    while (auto token = lexer.next()) {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    ILC::chain_size = ILC::chain.size();
}

TEST(parser_unit, function)
{
    ILC::chain.clear();
    tokens.clear();
    std::stringstream stream0;
    stream0.str(R"(main = {})");
    zen::lexer lexer(stream0);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    ILC::chain_size = ILC::chain.size();
    EXPECT_TRUE(parse());
}

TEST(parser_unit, function_with_single_int_parameter)
{
    setup_parser_test("test(a: int) = {}");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, function_with_multiple_parameters)
{
    setup_parser_test("add(a: int, b: int, c: int) = int { a + b + c }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, function_with_different_parameter_types)
{
    setup_parser_test("mix(a: int, b: float, c: bool) = {}");
    EXPECT_TRUE(parse());
}

// TEST(parser_unit, function_with_generic_parameter)
// {
//     setup_parser_test("identity<T>(value: T) = T { return value; }");
//     EXPECT_TRUE(parse());
// }

TEST(parser_unit, function_with_parameter_list_in_class)
{
    setup_parser_test("class Player { set_age(age: short) = {} }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, empty_parameter_list)
{
    setup_parser_test("no_params() = {}");
    EXPECT_TRUE(parse());
}

// TEST(parser_unit, function_with_complex_parameter_type)
// {
//     setup_parser_test("process(list_of_strings: List<string>) = {}");
//     EXPECT_TRUE(parse());
// }

TEST(parser_unit, function_with_params_and_body)
{
    setup_parser_test("calculate_area(width: int, height: int) = int { width * height }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, function_without_return_and_with_params)
{
    setup_parser_test("print_sum(a: int, b: int) = {}");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, invalid_function_missing_type_for_param)
{
    setup_parser_test("test(x:) = {}");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing parameter type.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, invalid_function_missing_comma_between_params)
{
    setup_parser_test("test(a: int b: int) = {}");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing comma between parameters.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, invalid_function_extra_comma_at_end)
{
    setup_parser_test("test(a: int,) = {}");
    try {
        parse();
        FAIL() << "Expected syntax_error for extra comma in parameter list.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, simple_arithmetic_expression)
{
    setup_parser_test("my_func = int { 1 + 2 * 3 }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, complex_expression_with_parenthesis)
{
    setup_parser_test("my_func = int { (10 + 2) / 3 }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, chained_comparison_operators)
{
    setup_parser_test("my_func(a: int, b: int) = { if (a > 10 && b < 20) {} }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, assignment_expression)
{
    setup_parser_test("my_func = { y: int = 1 x : int = y + 5 }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, combined_assignment_operators)
{
    setup_parser_test("my_func = { x: int = 0 x = 5 }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, function_call_with_arguments)
{
    setup_parser_test("my_other_func(x: int, y: int, z: int) = int(x+y+z) my_func = { my_other_func(1, 2, 3) }");
    EXPECT_TRUE(parse());
}

// Class Declaration Tests

TEST(parser_unit, simple_class_declaration)
{
    setup_parser_test("class MyClass {}");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, class_with_fields)
{
    setup_parser_test("class Person { name: string age: int }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, class_with_static_fields)
{
    setup_parser_test("class Counter { static count: int = 0 }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, class_with_method)
{
    setup_parser_test("class Calculator { add(a: int, b: int) = int { a + b } }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, class_with_generic)
{
    setup_parser_test("class List<T> { item: T }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, invalid_class_missing_name)
{
    setup_parser_test("class {}");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing class name.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, invalid_class_missing_braces)
{
    setup_parser_test("class MyClass");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing class body.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

// Control Flow Tests

TEST(parser_unit, simple_if_statement)
{
    setup_parser_test("test_func = { if (true) {} }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, if_else_statement)
{
    setup_parser_test("test_func = { x: int = 1 if (x > 0) { x = x + 1 } else { x = 0 } }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, if_else_if_chain)
{
    setup_parser_test("test_func = { x: int = 1 if (x > 10) { x = 1 } else if (x > 5) { x = 2 } else { x = 3 } }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, nested_if_statements)
{
    setup_parser_test("test_func = { x : int = 1 a: int = 2 b : int = 3 if (a > 0) { if (b > 0) { x = 1 } } }");
    EXPECT_TRUE(parse());
}

/* disable extract for now

TEST(parser_unit, if_with_pattern_matching)
{
    setup_parser_test("getValue() = int(0) test_func = { x: int = 1 if (x: int = getValue() && x > 0) { x = x * 2 } }");
    EXPECT_TRUE(parse());
}
*/
TEST(parser_unit, invalid_if_missing_condition)
{
    setup_parser_test("test_func = { if () {} }");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing if condition.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, invalid_if_missing_parentheses)
{
    setup_parser_test("test_func = { if true {} }");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing parentheses around condition.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

// Loop Tests

TEST(parser_unit, simple_while_loop)
{
    setup_parser_test("test_func = { x: int = 10 while (x > 0) { x = x - 1 } }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, while_with_complex_condition)
{
    setup_parser_test("test_func = { x: int = 10 y: int = 1 while (x > 0 && y < 10) { x = x - 1 y = y + 1 } }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, simple_for_loop)
{
    setup_parser_test("test_func = { x: int = 10 i: int = 1 for (i: int = 0,10) { x = x + i } }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, for_loop_without_step)
{
    setup_parser_test("test_func = { x: int = 10 i: int = 1 for (i: int = 0,10, 1) { x = x + i } }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, for_loop_with_multiple_variables)
{
    setup_parser_test("test_func = { x: int = 10 i: int = 1 j: int = 1 for (i: int = 0, 10; j: int = 0,5) { x = i + j } }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, invalid_for_missing_initialization)
{
    setup_parser_test("test_func = { for (, i < 10, i = i + 1) {} }");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing for loop initialization.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

// Variable Declaration Tests

TEST(parser_unit, simple_variable_declaration)
{
    setup_parser_test("test_func = { x: int }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, variable_with_initialization)
{
    setup_parser_test("test_func = { x: int = 42 }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, multiple_variables)
{
    setup_parser_test("test_func = { x: int = 1 /*s: string = \"hello\" /* missing constructor implementation :( */ */ y : double = 3.14 z: bool = true }");
    EXPECT_TRUE(parse());
}

// TEST(parser_unit, variable_with_generic_type)
// {
//     setup_parser_test("test_func = { myList: List<int> }");
//     EXPECT_TRUE(parse());
// }

TEST(parser_unit, invalid_variable_missing_type)
{
    setup_parser_test("test_func = { x: }");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing variable type.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, invalid_variable_missing_colon)
{
    setup_parser_test("test_func = { x int = 5 }");
    try {
        parse();
        FAIL() << "Expected semantic_error for missing symbol x.";
    } catch (const zen::exceptions::semantic_error& e) {
        SUCCEED();
    }
}

// Expression Tests

TEST(parser_unit, ternary_operator)
{
    setup_parser_test("test_func(x: int) = int { x > 0 ? 1 : -1 }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, nested_ternary)
{
    setup_parser_test("test_func(x: int, y: int) = int { x > 0 ? (y > 0 ? 1 : 2) : -1 }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, logical_operators)
{
    setup_parser_test("test_func(a: bool, b: bool, c: bool) = bool { a && b || c }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, bitwise_and_logical_not)
{
    setup_parser_test("test_func(a: bool, b: bool) = bool { !a && !b }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, string_concatenation)
{
    setup_parser_test("test_func = string { \"Hello\" + \" \" + \"World\" }");
    try {
        parse();
        FAIL();
    } catch (const zen::exceptions::semantic_error& e) {
        SUCCEED();
    }
}
/*
TEST(parser_unit, array_literal)
{
    setup_parser_test("test_func = { arr: int[] = [1, 2, 3, 4, 5] }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, empty_array)
{
    setup_parser_test("test_func = { arr: int[] = [] }");
    try {
        parse();
        SUCCEED();
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}


TEST(parser_unit, nested_array)
{
    setup_parser_test("test_func = { matrix: int[][] = [[1, 2], [3, 4]] }");
    EXPECT_TRUE(parse());
}
*/
TEST(parser_unit, function_call_no_args)
{
    setup_parser_test("getValue() = int(42) test_func = { x: int = getValue() }");
    EXPECT_TRUE(parse());
}

// TEST(parser_unit, chained_function_calls)
// {
//     setup_parser_test("getObj() = MyClass() test_func = { result: int = getObj().getValue().process() }");
//     EXPECT_TRUE(parse());
// }
//
// TEST(parser_unit, function_with_generic_call)
// {
//     setup_parser_test("create<T>() = T() test_func = { obj: MyClass = create<MyClass>() }");
//     EXPECT_TRUE(parse());
// }

// Increment/Decrement Tests

TEST(parser_unit, prefix_increment)
{
    setup_parser_test("test_func = { x: int = 0 y: int = ++x }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, postfix_increment)
{
    setup_parser_test("test_func = { x: int = 0 y: int = x++ }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, prefix_decrement)
{
    setup_parser_test("test_func = { x: int = 10 y: int = --x }");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, postfix_decrement)
{
    setup_parser_test("test_func = { x: int = 10 y: int = x-- }");
    EXPECT_TRUE(parse());
}

// Using Statement Tests

TEST(parser_unit, using_statement)
{
    setup_parser_test("using std.io");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, using_with_dots)
{
    setup_parser_test("using system.collections.generic");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, multiple_using_statements)
{
    setup_parser_test("using std.io using std.string using math.functions");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, invalid_using_missing_namespace)
{
    setup_parser_test("using");
    try {
        parse();
        FAIL() << "Expected syntax_error for missing namespace in using statement.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

// Complex Combinations

TEST(parser_unit, class_with_constructor_and_methods)
{
    setup_parser_test(R"(
        class Rectangle {
            width: int
            height: int
            /* init(w: int, h: int) = {
                width = w
                height = h
            }
            area() = int { width * height }
            perimeter() = int { 2 * (width + height) }
            */
        }
    )");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, function_with_local_variables_and_logic)
{
    setup_parser_test(R"(
        factorial(n: int) = int {
            result: int = 1
            i: int = 1
            while (i <= n) {
                result = result * i
                i = i + 1
            }
            result
        }
    )");
    EXPECT_TRUE(parse());
}
/*
TEST(parser_unit, nested_control_structures)
{
    setup_parser_test(R"(
        process_matrix(matrix: int[][]) = {
            i: int = 0
            while (i < matrix.length()) {
                j: int = 0
                while (j < matrix[i].length()) {
                    if (matrix[i][j] > 0) {
                        matrix[i][j] = matrix[i][j] * 2
                    } else if (matrix[i][j] < 0) {
                        matrix[i][j] = 0
                    }
                    j = j + 1
                }
                i = i + 1
            }
        }
    )");
    EXPECT_TRUE(parse());
}

TEST(parser_unit, generic_function_with_constraints)
{
        setup_parser_test("compare<T>(a: T, b: T) = bool { a == b }");
        EXPECT_TRUE(parse());
}
*/
// Error Cases for Complex Expressions

TEST(parser_unit, invalid_unmatched_parentheses)
{
    setup_parser_test("test_func = int { (1 + 2 * 3 }");
    try {
        parse();
        FAIL() << "Expected syntax_error for unmatched parentheses.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, invalid_incomplete_ternary)
{
    setup_parser_test("test_func = int { x: int = 1 x > 0 ? 1 }");
    try {
        parse();
        FAIL() << "Expected syntax_error for incomplete ternary operator.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, invalid_trailing_operator)
{
    setup_parser_test("test_func = int { 1 + 2 + }");
    try {
        parse();
        FAIL() << "Expected syntax_error for trailing operator.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}

TEST(parser_unit, invalid_double_operator)
{
    setup_parser_test("test_func = int { 1 ++ 2 }");
    try {
        parse();
        FAIL() << "Expected syntax_error for invalid operator sequence.";
    } catch (const zen::exceptions::syntax_error& e) {
        SUCCEED();
    }
}