//
// Created by dte on 9/20/2025.
//

#pragma once
#include <gtest/gtest.h>
#include "parser/parser.hpp"

inline void setup_integration_test(const std::string& code, zen::composer::composer * composer)
{
    assert(composer);
    composer->reset();
    setup_parser_test(code);
}

TEST(integration, empty_main)
{
    const auto composer = dynamic_cast<zen::composer::vm::composer*>(get_composer());
    setup_integration_test(R"(main = {})", composer);
    EXPECT_TRUE(parse());
    EXPECT_EQ(composer->code, (std::vector<i64>{hlt, ret}));
    zen::vm::stack stack;
    stack.push<i64>(0);
    zen::vm vm1;
    vm1.load(composer->code);
    vm1.run(stack, 1);
    EXPECT_TRUE(true);
}

TEST(integration, add_numbers)
{
    const auto composer = dynamic_cast<zen::composer::vm::composer*>(get_composer());
    setup_integration_test(R"(add(a: int, b: int, c: int) = int { a + b + c })", composer);
    EXPECT_TRUE(parse());
    EXPECT_EQ(composer->code, (std::vector<i64>{hlt, most, -4, add_i32, -4, -20, -16, most, -4, add_i32, -4, -28, -8, i32_to_i32, -32, -4, most, 8, ret,}));
    zen::vm::stack stack;
    stack.push<i32>(0); // return value
    i32 a = 100, b = 200, c = 300;
    stack.push<i32>(a); // a
    stack.push<i32>(b); // b
    stack.push<i32>(c); // c
    stack.push<i64>(0); // address to return to
    zen::vm vm1;
    vm1.load(composer->code);
    vm1.run(stack, 1);
    stack += sizeof(i32) * 3;
    EXPECT_EQ(stack.top<i32>(), a + b + c);
}

TEST(integration, hello_world)
{
    const auto composer = dynamic_cast<zen::composer::vm::composer*>(get_composer());
    composer->reset();

    composer->begin("print");
    composer->set_parameter("string", "string");
    composer->push("string.data");
    composer->push("string.len");
    composer->zen::composer::composer::push<i64>(reinterpret_cast<i64>(stdout), "long");
    composer->call(std::to_string(write_str), 3, composer::call_result::pushed);
    composer->end();

    setup_parser_test(R"(main = { s : string })");
    EXPECT_TRUE(parse());
    composer->link();
    composer->bake();
    const std::list<composer::vm::function> main_functions = composer->functions["main"];
    EXPECT_EQ(main_functions.size(), 1);
    const composer::vm::function main = main_functions.front();
    zen::vm::stack stack;
    stack.push<i64>(0); // returning address
    zen::vm vm1;
    vm1.load(composer->code);
    vm1.run(stack, main.address);
    EXPECT_TRUE(true);
}
