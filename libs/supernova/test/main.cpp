#include <gtest/gtest.h>
#include "jit/function.hpp"
//
// Created by ReneMuala on 11/24/2024.
//

TEST(JIT, jump_equal)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(40);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 1);
    }
}

TEST(JIT, jump_equal_case_different_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(-40);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 0);
    }
}

TEST(JIT, jump_not_equal)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(4);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_not_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 1);
    }
}

TEST(JIT, jump_not_equal_case_equal_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(40);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_not_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 0);
    }
}

TEST(JIT, jump_lower)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(39);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_lower(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 1);
    }
}

TEST(JIT, jump_lower_case_greater_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(91);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_lower(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 0);
    }
}

TEST(JIT, jump_lower_equal)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(40);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_lower_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 1);
    }
}

TEST(JIT, jump_lower_equal_case_lower_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(7);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_lower_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 1);
    }
}

TEST(JIT, jump_lower_equal_case_greater_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(41);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_lower_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 0);
    }
}

TEST(JIT, jump_greater)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(40);
    asmjit::x86::Gp arg1 = builder->i32(4);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_greater(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 1);
    }
}

TEST(JIT, jump_greater_case_lower_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(0);
    asmjit::x86::Gp arg1 = builder->i32(1);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_greater(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 0);
    }
}

TEST(JIT, jump_greater_case_equal_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(40);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_greater(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 0);
    }
}

TEST(JIT, jump_greater_equal)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(40);
    asmjit::x86::Gp arg1 = builder->i32(10);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_greater_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 1);
    }
}

TEST(JIT, jump_greater_equal_case_equal_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(40);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_greater_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 1);
    }
}

TEST(JIT, jump_greater_equal_case_lower_value)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    asmjit::x86::Gp arg0 = builder->i32(4);
    asmjit::x86::Gp arg1 = builder->i32(40);
    asmjit::x86::Gp result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_greater_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 0);
    }
}

TEST(JIT, add_f32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<float, float, float>());
    auto arg0 = builder->f32();
    auto arg1 = builder->f32();
    auto result = builder->f32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->add(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<float(float, float)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        float a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a+b);
    }
}

TEST(JIT, add_f64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<double, double, double>());
    auto arg0 = builder->f64();
    auto arg1 = builder->f64();
    auto result = builder->f64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->add(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<double(double, double)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        double a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a+b);
    }
}

TEST(JIT, sub_f64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<double, double, double>());
    auto arg0 = builder->f64();
    auto arg1 = builder->f64();
    auto result = builder->f64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->sub(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<double(double, double)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        double a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a-b);
    }
}

TEST(JIT, mul_f64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<double, double, double>());
    auto arg0 = builder->f64();
    auto arg1 = builder->f64();
    auto result = builder->f64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mul(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<double(double, double)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        double a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a*b);
    }
}

TEST(JIT, div_f64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<double, double, double>());
    auto arg0 = builder->f64();
    auto arg1 = builder->f64();
    auto result = builder->f64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->div(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<double(double, double)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        double a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a/b);
    }
}

TEST(JIT, sub_f32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<float, float, float>());
    auto arg0 = builder->f32();
    auto arg1 = builder->f32();
    auto result = builder->f32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->sub(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<float(float, float)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        float a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a-b);
    }
}

TEST(JIT, mul_f32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<float, float, float>());
    auto arg0 = builder->f32();
    auto arg1 = builder->f32();
    auto result = builder->f32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mul(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<float(float, float)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        float a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a*b);
    }
}

TEST(JIT, div_f32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<float, float, float>());
    auto arg0 = builder->f32();
    auto arg1 = builder->f32();
    auto result = builder->f32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->div(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<float(float, float)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        float a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a/b);
    }
}

TEST(JIT, add_i8)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<char, char, char>(), false);
    auto arg0 = builder->i8();
    auto arg1 = builder->i8();
    auto result = builder->i8();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->add(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<char(char, char)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        char a = 120, b = 50, r = func(a, b);
        ASSERT_EQ(r, static_cast<char>(a + b));
    }
}

TEST(JIT, sub_i8)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<char, char, char>());
    auto arg0 = builder->i8();
    auto arg1 = builder->i8();
    auto result = builder->i8();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->sub(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<char(char, char)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        char a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, static_cast<char>(a - b));
    }
}

TEST(JIT, mul_i8)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<char, char, char>());
    auto arg0 = builder->i8();
    auto arg1 = builder->i8();
    auto result = builder->i8();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mul(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<char(char, char)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        char a = 5, b = 5, r = func(a, b);
        ASSERT_EQ(r, static_cast<char>(a * b));
    }
}

TEST(JIT, div_i8)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<char, char, char>());
    auto arg0 = builder->i8();
    auto arg1 = builder->i8();
    auto result = builder->i8();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->div(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<char(char, char)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        char a = 5, b = 5, r = func(a, b);
        ASSERT_EQ(r, static_cast<char>(a / b));
    }
}

TEST(JIT, mod_i8)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<char, char, char>());
    auto arg0 = builder->i8();
    auto arg1 = builder->i8();
    auto result = builder->i8();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mod(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<char(char, char)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        char a = 50, b = 40, r = func(a, b);
        ASSERT_EQ(r, static_cast<char>(a % b));
    }
}

TEST(JIT, add_i16)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<short, short, short>());
    auto arg0 = builder->i16();
    auto arg1 = builder->i16();
    auto result = builder->i16();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->add(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<short(short, short)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        short a = 50, b = 50, r = func(a, b);
        ASSERT_EQ(r, static_cast<short>(a + b));
    }
}

TEST(JIT, sub_i16)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<short, short, short>());
    auto arg0 = builder->i16();
    auto arg1 = builder->i16();
    auto result = builder->i16();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->sub(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<short(short, short)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        short a = 50, b = 50, r = func(a, b);
        ASSERT_EQ(r, static_cast<short>(a - b));
    }
}

TEST(JIT, mul_i16)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<short, short, short>());
    auto arg0 = builder->i16();
    auto arg1 = builder->i16();
    auto result = builder->i16();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mul(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<short(short, short)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        short a = 50, b = 50, r = func(a, b);
        ASSERT_EQ(r, static_cast<short>(a * b));
    }
}

TEST(JIT, div_i16)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<short, short, short>());
    auto arg0 = builder->i16();
    auto arg1 = builder->i16();
    auto result = builder->i16();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->div(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<short(short, short)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        short a = 50, b = 50, r = func(a, b);
        ASSERT_EQ(r, static_cast<short>(a / b));
    }
}

TEST(JIT, mod_i16)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<short, short, short>());
    auto arg0 = builder->i16();
    auto arg1 = builder->i16();
    auto result = builder->i16();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mod(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<short(short, short)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        short a = 50, b = 50, r = func(a, b);
        ASSERT_EQ(r, static_cast<short>(a % b));
    }
}

TEST(JIT, add_i32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int, int, int>());
    auto arg0 = builder->i32();
    auto arg1 = builder->i32();
    auto result = builder->i32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->add(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<int(int, int)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a+b);
    }
}

TEST(JIT, sub_i32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int, int, int>());
    auto arg0 = builder->i32();
    auto arg1 = builder->i32();
    auto result = builder->i32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->sub(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<int(int, int)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a-b);
    }
}

TEST(JIT, mul_i32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int, int, int>());
    auto arg0 = builder->i32();
    auto arg1 = builder->i32();
    auto result = builder->i32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mul(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<int(int, int)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a*b);
    }
}

TEST(JIT, div_i32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int, int, int>());
    auto arg0 = builder->i32();
    auto arg1 = builder->i32();
    auto result = builder->i32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->div(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<int(int, int)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a/b);
    }
}

TEST(JIT, mod_i32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int, int, int>());
    auto arg0 = builder->i32();
    auto arg1 = builder->i32();
    auto result = builder->i32();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mod(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<int(int, int)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a%b);
    }
}

TEST(JIT, add_i64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<long long, long long, long long>());
    auto arg0 = builder->i64();
    auto arg1 = builder->i64();
    auto result = builder->i64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->add(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<long long(long long, long long)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        long long a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a+b);
    }
}

TEST(JIT, sub_i64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<long long, long long, long long>());
    auto arg0 = builder->i64();
    auto arg1 = builder->i64();
    auto result = builder->i64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->sub(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<long long(long long, long long)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        long long a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a-b);
    }
}

TEST(JIT, mul_i64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<long long, long long, long long>());
    auto arg0 = builder->i64();
    auto arg1 = builder->i64();
    auto result = builder->i64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mul(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<long long(long long, long long)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        long long a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a*b);
    }
}

TEST(JIT, div_i64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<long long, long long, long long>());
    auto arg0 = builder->i64();
    auto arg1 = builder->i64();
    auto result = builder->i64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->div(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<long long(long long, long long)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        long long a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a/b);
    }
}

TEST(JIT, mod_i64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<long long, long long, long long>());
    auto arg0 = builder->i64();
    auto arg1 = builder->i64();
    auto result = builder->i64();
    builder->fetch_argument(0, arg0);
    builder->fetch_argument(1, arg1);
    builder->mod(result, arg0, arg1);
    builder->return_value(result);
    const auto func = builder->build<long long(long long, long long)>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        long long a = 90, b = 90, r = func(a, b);
        ASSERT_EQ(r, a%b);
    }
}


bool native_function_called = false;
void native_function()
{
    native_function_called = true;
}

TEST(JIT, native_call)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<void>());
    builder->call(native_function, asmjit::FuncSignature::build<void>(),{},{});
    builder->return_void();
    const auto func = builder->build<void()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        func();
        ASSERT_TRUE(native_function_called);
    }
}

bool native_function_args_called = false;
int native_function_args_a, native_function_args_b, native_function_args_c;
float native_function_args_d, native_function_args_e, native_function_args_f;
long long native_function_args_g, native_function_args_h, native_function_args_i;
void native_function_args(int a, int b, int c, float d, float e, float f, long long g, long long h, long long i)
{
    native_function_args_called = true;
    native_function_args_a = a;
    native_function_args_b = b;
    native_function_args_c = c;
    native_function_args_d = d;
    native_function_args_e = e;
    native_function_args_f = f;
    native_function_args_g = g;
    native_function_args_h = h;
    native_function_args_i = i;
}

TEST(JIT, native_call_args)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<void>());
    int _a = 1, _b = 2, _c = 3;
    float _d = 4.5, _e = 5.5, _f = 6.5;
    long long _g = 7, _h = 8, _i = 9;

    auto a = builder->i32(_a);
    auto b = builder->i32(_b);
    auto c = builder->i32(_c);

    auto d = builder->f32(_d);
    auto e = builder->f32(_e);
    auto f = builder->f32(_f);

    auto g = builder->i64(_g);
    auto h = builder->i64(_h);
    auto i = builder->i64(_i);

    builder->call(native_function_args, asmjit::FuncSignature::build<void, int, int, int, float, float, float, long long, long long, long long>(),{a, b, c, d, e, f, g, h, i},{});
    builder->return_void();
    const auto func = builder->build<void()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        func();
        ASSERT_TRUE(native_function_args_called);
        ASSERT_EQ(native_function_args_a, _a);
        ASSERT_EQ(native_function_args_b, _b);
        ASSERT_EQ(native_function_args_c, _c);
        ASSERT_EQ(native_function_args_d, _d);
        ASSERT_EQ(native_function_args_e, _e);
        ASSERT_EQ(native_function_args_f, _f);
        ASSERT_EQ(native_function_args_g, _g);
        ASSERT_EQ(native_function_args_h, _h);
        ASSERT_EQ(native_function_args_i, _i);
    }
}

bool native_function_args_return_called = false;
int native_function_args_return_a, native_function_args_return_b, native_function_args_return_c;
float native_function_args_return_d, native_function_args_return_e, native_function_args_return_f;
float native_function_args_return(int a, int b, int c, float d, float e, float f)
{
    native_function_args_return_called = true;
    native_function_args_return_a = a;
    native_function_args_return_b = b;
    native_function_args_return_c = c;
    native_function_args_return_d = d;
    native_function_args_return_e = e;
    native_function_args_return_f = f;
    return static_cast<float>(a + b + c) + d + e + f;
}

TEST(JIT, native_call_args_return)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<float>());
    int _a = 1, _b = 2, _c = 3;
    float _d = 4.5, _e = 5.5, _f = 6.5;

    auto a = builder->i32(_a);
    auto b = builder->i32(_b);
    auto c = builder->i32(_c);

    auto d = builder->f32(_d);
    auto e = builder->f32(_e);
    auto f = builder->f32(_f);
    auto r = builder->f32();

    builder->call(native_function_args_return, asmjit::FuncSignature::build<float, int, int, int, float, float, float>(),{a, b, c, d, e, f},{r});
    builder->return_value(r);
    const auto func = builder->build<float()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        auto _r = func();
        ASSERT_TRUE(native_function_args_return_called);
        ASSERT_EQ(native_function_args_return_a, _a);
        ASSERT_EQ(native_function_args_return_b, _b);
        ASSERT_EQ(native_function_args_return_c, _c);
        ASSERT_EQ(native_function_args_return_d, _d);
        ASSERT_EQ(native_function_args_return_e, _e);
        ASSERT_EQ(native_function_args_return_f, _f);
        ASSERT_EQ(_a+_b+_c+_d+_e+_f, _r);
    }
}

TEST(JIT, increment_i8)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<char>());
    auto result = builder->i8(40);
    builder->increment(result);
    builder->return_value(result);
    const auto func = builder->build<char()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        char  r = func();
        ASSERT_EQ(r, 41);
    }
}

TEST(JIT, increment_i16)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<short>());
    auto result = builder->i16(40);
    builder->increment(result);
    builder->return_value(result);
    const auto func = builder->build<short()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        short  r = func();
        ASSERT_EQ(r, 41);
    }
}


TEST(JIT, increment_i32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    auto result = builder->i32(40);
    builder->increment(result);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 41);
    }
}

TEST(JIT, increment_i64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<long long>());
    auto result = builder->i64(40);
    builder->increment(result);
    builder->return_value(result);
    const auto func = builder->build<long long()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        long long r = func();
        ASSERT_EQ(r, 41);
    }
}

TEST(JIT, increment_f32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<float>());
    auto result = builder->f32(40);
    builder->increment(result);
    builder->return_value(result);
    const auto func = builder->build<float()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        float  r = func();
        ASSERT_EQ(r, 41);
    }
}


TEST(JIT, increment_f64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<double>());
    auto result = builder->f64(40);
    builder->increment(result);
    builder->return_value(result);
    const auto func = builder->build<double()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        double r = func();
        ASSERT_EQ(r, 41);
    }
}

TEST(JIT, decrement_f64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<double>());
    auto result = builder->f64(40);
    builder->decrement(result);
    builder->return_value(result);
    const auto func = builder->build<double()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        double r = func();
        ASSERT_EQ(r, 39);
    }
}

TEST(JIT, decrement_i8)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<char>());
    auto result = builder->i8(40);
    builder->decrement(result);
    builder->return_value(result);
    const auto func = builder->build<char()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        char  r = func();
        ASSERT_EQ(r, 39);
    }
}

TEST(JIT, decrement_i16)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<short>());
    auto result = builder->i16(40);
    builder->decrement(result);
    builder->return_value(result);
    const auto func = builder->build<short()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        short  r = func();
        ASSERT_EQ(r, 39);
    }
}


TEST(JIT, decrement_i32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    const auto result = builder->i32(40);
    builder->decrement(result);
    builder->return_value(result);
    const auto func = builder->build<int()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        int  r = func();
        ASSERT_EQ(r, 39);
    }
}

TEST(JIT, decrement_i64)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<long long>());
    auto result = builder->i64(40);
    builder->decrement(result);
    builder->return_value(result);
    const auto func = builder->build<long long()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        long long r = func();
        ASSERT_EQ(r, 39);
    }
}

TEST(JIT, decrement_f32)
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<float>());
    auto result = builder->f32(40);
    builder->decrement(result);
    builder->return_value(result);
    const auto func = builder->build<float()>();
    ASSERT_NE(func, nullptr);
    if (func)
    {
        float  r = func();
        ASSERT_EQ(r, 39);
    }
}

int main()
{
    testing::InitGoogleTest();
    RUN_ALL_TESTS();
}