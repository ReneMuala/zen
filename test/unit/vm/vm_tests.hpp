//
// Created by dte on 9/17/2025.
//

#pragma once
#include "vm/vm.hpp"

using namespace zen;

TEST(vm_unit, i8_to_i8_test)
{
    i8 a = 0, b = 10;
    std::vector<i64> code {i8_to_i8, vm::ref(a), vm::ref(b), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(a, b);
}

TEST(vm_unit, i8_to_i64_test)
{
    i8 a = 127;
    i64 b = 0;
    std::vector<i64> code {i8_to_i64, vm::ref(b), vm::ref(a), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(b, 127);
}

TEST(vm_unit, i32_to_f64_test)
{
    i32 a = 42;
    f64 b = 0.0;
    std::vector<i64> code {i32_to_f64, vm::ref(b), vm::ref(a), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_DOUBLE_EQ(b, 42.0);
}

TEST(vm_unit, f32_to_i16_test)
{
    f32 a = 123.7f;
    i16 b = 0;
    std::vector<i64> code {f32_to_i16, vm::ref(b), vm::ref(a), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(b, 123);
}

TEST(vm_unit, add_i64_test)
{
    i64 a = 0, b = 15, c = 25;
    std::vector<i64> code {add_i64, vm::ref(a), vm::ref(b), vm::ref(c), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(a, 40);
}

TEST(vm_unit, sub_f64_test)
{
    f64 a = 0.0, b = 10.5, c = 3.2;
    std::vector<i64> code {sub_f64, vm::ref(a), vm::ref(b), vm::ref(c), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_DOUBLE_EQ(a, 7.3);
}

TEST(vm_unit, mul_i32_test)
{
    i32 a = 0, b = 6, c = 7;
    std::vector<i64> code {mul_i32, vm::ref(a), vm::ref(b), vm::ref(c), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(a, 42);
}

TEST(vm_unit, div_i16_test)
{
    i16 a = 0, b = 20, c = 4;
    std::vector<i64> code {div_i16, vm::ref(a), vm::ref(b), vm::ref(c), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(a, 5);
}

TEST(vm_unit, mod_i64_test)
{
    i64 a = 0, b = 17, c = 5;
    std::vector<i64> code {mod_i64, vm::ref(a), vm::ref(b), vm::ref(c), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(a, 2);
}

TEST(vm_unit, gt_i32_test)
{
    boolean result = false;
    i32 a = 10, b = 5;
    std::vector<i64> code {gt_i32, vm::ref(result), vm::ref(a), vm::ref(b), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_TRUE(result);
}

TEST(vm_unit, lt_f64_test)
{
    boolean result = true;
    f64 a = 3.14, b = 2.71;
    std::vector<i64> code {lt_f64, vm::ref(result), vm::ref(a), vm::ref(b), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_FALSE(result);
}

TEST(vm_unit, eq_i8_test)
{
    boolean result = false;
    i8 a = 42, b = 42;
    std::vector<i64> code {eq_i8, vm::ref(result), vm::ref(a), vm::ref(b), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_TRUE(result);
}

TEST(vm_unit, neq_i64_test)
{
    boolean result = false;
    i64 a = 100, b = 200;
    std::vector<i64> code {neq_i64, vm::ref(result), vm::ref(a), vm::ref(b), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_TRUE(result);
}

TEST(vm_unit, boolean_and_test)
{
    boolean result = false, a = true, b = true;
    std::vector<i64> code {boolean_and, vm::ref(result), vm::ref(a), vm::ref(b), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_TRUE(result);
}

TEST(vm_unit, boolean_or_test)
{
    boolean result = false, a = false, b = true;
    std::vector<i64> code {boolean_or, vm::ref(result), vm::ref(a), vm::ref(b), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_TRUE(result);
}

TEST(vm_unit, boolean_not_test)
{
    boolean result = true, input = true;
    std::vector<i64> code {boolean_not, vm::ref(result), vm::ref(input), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_FALSE(result);
}

TEST(vm_unit, push_i64_test)
{
    zen::vm::stack stack;
    i64 value = 123456;
    std::vector<i64> code {push_i64, vm::ref(value), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(stack, 0);
    EXPECT_EQ(stack.size(), sizeof(i64));
}

TEST(vm_unit, push_f32_test)
{
    zen::vm::stack stack;
    f32 value = 3.14f;
    std::vector<i64> code {push_f32, vm::ref(value), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(stack, 0);
    EXPECT_EQ(stack.size(), sizeof(f32));
}

TEST(vm_unit, push_i8_test)
{
    zen::vm::stack stack;
    i8 value = 127;
    std::vector<i64> code {push_i8, vm::ref(value), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(stack, 0);
    EXPECT_EQ(stack.size(), sizeof(i8));
}

TEST(vm_unit, push_i16_test)
{
    zen::vm::stack stack;
    i16 value = 32767;
    std::vector<i64> code {push_i16, vm::ref(value), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(stack, 0);
    EXPECT_EQ(stack.size(), sizeof(i16));
}

TEST(vm_unit, push_i32_test)
{
    zen::vm::stack stack;
    i32 value = 2147483647;
    std::vector<i64> code {push_i32, vm::ref(value), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(stack, 0);
    EXPECT_EQ(stack.size(), sizeof(i32));
}

TEST(vm_unit, push_f64_test)
{
    zen::vm::stack stack;
    f64 value = 123.456;
    std::vector<i64> code {push_f64, vm::ref(value), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(stack, 0);
    EXPECT_EQ(stack.size(), sizeof(f64));
}

// Control flow tests
TEST(vm_unit, go_test)
{
    i64 result = 0, value1 = 1, value2 = 42;
    std::vector<i64> code {
        go, 4,                                    // Jump over next instruction
        i64_to_i64, vm::ref(result), vm::ref(value1),  // This should be skipped
        i64_to_i64, vm::ref(result), vm::ref(value2),  // This should execute
        hlt
    };
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(result, 42);
}

TEST(vm_unit, go_if_true_test)
{
    boolean condition = true;
    i64 result = 0, value1 = 1, value2 = 42;
    std::vector<i64> code {
        go_if, vm::ref(condition), 5,                   // Jump if true
        i64_to_i64, vm::ref(result), vm::ref(value1),        // Skip this
        i64_to_i64, vm::ref(result), vm::ref(value2),        // Execute this
        hlt
    };
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(result, 42);
}

TEST(vm_unit, go_if_false_test)
{
    boolean condition = false;
    i64 result = 0, value1 = 42, value2 = 1;
    std::vector<i64> code {
        go_if, vm::ref(condition), 3,                   // Don't jump
        i64_to_i64, vm::ref(result), vm::ref(value1),        // Execute this
        i64_to_i64, vm::ref(result), vm::ref(value2),        // And this
        hlt
    };
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(result, 1);
}

TEST(vm_unit, allocate_deallocate_test)
{
    i64 ptr = 0;
    i64 size = 1024;
    std::vector<i64> code {
        allocate, vm::ref(ptr), vm::ref(size),
        deallocate, vm::ref(ptr),
        hlt
    };
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_TRUE(true);
}

// Complex test combining multiple operations
TEST(vm_unit, complex_arithmetic_test)
{
    i32 a = 0, b = 10, c = 5, temp = 0;
    std::vector<i64> code {
        add_i32, vm::ref(temp), vm::ref(b), vm::ref(c),  // temp = 10 + 5 = 15
        mul_i32, vm::ref(a), vm::ref(temp), vm::ref(c),  // a = 15 * 5 = 75
        hlt
    };
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(a, 75);
}

TEST(vm_unit, write_str_test)
{
    const auto _tmp = tmpfile();
    std::string message = "hello world";

    {
        zen::utils::constant_pool pool;
        i64 _len = message.length() + 20; // trying something nasty
        const auto _str = (i64)pool.get<i64>((i64)zen::string::from_string(message)).get();
        const auto _stream = (i64)pool.get<i64>((i64)_tmp).get();
        std::vector<i64> code {write_str, _str, vm::ref(_len), vm::ref(_stream), hlt};
        zen::vm vm1;
        vm1.load(code);
        vm1.run(0);
        rewind(_tmp);
        const auto _c_str = new char[_len]{0};
        fread(_c_str, sizeof(char), _len, _tmp);
        EXPECT_EQ(message, std::string(_c_str));
        delete[] _c_str;
    }

    fclose(_tmp);
}

TEST(vm_unit, write_then_read_str_test)
{
    const auto _tmp = tmpfile();
    std::string message = "hello world";

    {
        zen::utils::constant_pool pool;
        i64 _len = message.length() + 20; // trying something nasty
        const auto _str = (i64)pool.get<i64>((i64)zen::string::from_string(message)).get();
        const auto _stream = (i64)pool.get<i64>((i64)_tmp).get();
        std::vector<i64> code {write_str, _str, vm::ref(_len), vm::ref(_stream), hlt};
        zen::vm vm1;
        vm1.load(code);
        vm1.run(0);
        rewind(_tmp);
        const auto _c_str = new char[_len]{0};
        fread(_c_str, sizeof(char), _len, _tmp);
        EXPECT_EQ(message, std::string(_c_str));
        delete[] _c_str;
    }
    rewind(_tmp);

    zen::utils::constant_pool pool;
    const auto _str = (i64)pool.get<i64>((i64)zen::string::empty()).get();
    const auto _stream = (i64)pool.get<i64>((i64)_tmp).get();
    i64 _len = 50;
    std::vector<i64> code {read_str, _str, vm::ref(_len), vm::ref(_stream), hlt};
    zen::vm vm1;
    vm1.load(code);
    vm1.run(0);
    EXPECT_EQ(*(i64*)*(i64*)_str, message.length());
    EXPECT_EQ(std::string((char*)*(i64*)(*(i64*)_str+sizeof(i64))), message);
    fclose(_tmp);
}