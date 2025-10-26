//
// Created by dte on 9/20/2025.
//

#pragma once
#include <gtest/gtest.h>
#include "parser/parser.hpp"
#include <chrono>

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
    {
        composer->push("string.data");
        const auto deref = composer->dereference(composer->top());
        composer->call(std::to_string(placeholder), 0, composer::call_result::pushed);
        composer->pop();
        composer->push(deref);
    }
    {
        composer->push("string.len");
        const auto deref = composer->dereference(composer->top());
        composer->pop();
        composer->push(deref);
    }
    composer->zen::composer::composer::push<i64>(reinterpret_cast<i64>(stdout), "long");
    composer->call(std::to_string(write_str), 3, composer::call_result::pushed);
    composer->end();
    composer->link();

    setup_parser_test(R"(
        /*sum(x: int, y: int) = int(x+y)
        div(y: float, z: float) = float {
            if(z != 0f){
                y/z
            } else {
                0f
            }
        }*/
        main = {/*
            i: int = 1
            _: unit = print("hello world\n")
            while(i < 10){
                _: unit = print("A\n")
                i = i + 1
                j: int = 1
                while(j < 5){
                    _: unit = print("\tB\n")
                    j = j + 1
                    k: int = 1
                    while(k < 5){
                        _: unit = print("\t\tC\n")
                        k = k + 1
                    }
                }
            }
*/
            for(i: int = 1, 5){
            }
        })");
    auto t0 = std::chrono::system_clock::now();
    EXPECT_TRUE(parse());
    auto t01 = std::chrono::system_clock::now();
    composer->bake();
    const std::list<composer::vm::function> main_functions = composer->functions["main"];
    EXPECT_EQ(main_functions.size(), 1);
    const composer::vm::function main = main_functions.front();
    zen::vm::stack stack;
    stack.push<i64>(0); // returning address
    zen::vm vm1;
    vm1.load(composer->code);
    auto t1 = std::chrono::system_clock::now();
    vm1.run(stack, main.address);
    auto t2 = std::chrono::system_clock::now();
    fmt::println("----------------\n1.compiled in {} ms \n2.ran in {} ms\n----------------", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(t01 - t0).count(),
        (long long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
    EXPECT_TRUE(true);
}
