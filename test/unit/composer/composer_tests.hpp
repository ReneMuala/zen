//
// Created by dte on 9/16/2025.
//

#pragma once
#include <gtest/gtest.h>
#include "composer/composer.hpp"
#include "composer/vm/composer.hpp"
#include "types/stack.hpp"

TEST(composer_unit, write_string)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("write");
    composer->set_parameter("fd", "long");
    composer->set_parameter("str", "string");
    composer->push("str.data");
    composer->push("str.len");
    composer->push("fd");
    composer->call(std::to_string(write_str), 3);;
    composer->end();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt,most, -8, refer, -8, -24, 8, write_str, -8, -24, -32, most, 8, ret,}));
}

TEST(composer_unit, print_string)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("write");
    composer->set_parameter("fd", "long");
    composer->set_parameter("str", "string");
    composer->push("str.data");
    composer->push("str.len");
    composer->push("fd");
    composer->call(std::to_string(write_str), 3);
    composer->end();

    composer->begin("print");
    composer->set_parameter("string", "string");
    composer->push("write");
    composer->push<i64>(reinterpret_cast<i64>(stdout), "long");
    composer->push("string");
    composer->call("write", 2);
    composer->end();
    auto _p = (i64)composer->_pool.get<i64>(reinterpret_cast<i64>(stdout)).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt,most, -8, refer, -8, -24, 8, write_str, -8, -24, -32, most, 8, ret, push_i64, _p,
                  push_i64, -16, call, 1, most, 16, ret,}));
}

TEST(composer_unit, print_string_2)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("write");
    composer->set_parameter("fd", "long");
    composer->set_parameter("str", "string");
    composer->push("str.data");
    composer->push("str.len");
    composer->push("fd");
    composer->call(std::to_string(write_str), 3);
    composer->end();

    composer->begin("print");
    composer->set_parameter("string", "string");
    composer->push("write");
    composer->push<i64>(reinterpret_cast<i64>(stdout), "long");
    composer->push("string");
    composer->call("write", 2);
    composer->end();

    composer->begin("print_string_2");
    composer->push("print");
    composer->push<zen::types::heap::string*>(zen::types::heap::string::from_string("hello world"), "string");
    composer->call("print", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<i64>(reinterpret_cast<i64>(stdout)).get();
    auto _p2 = (i64)composer->_pool.get<zen::types::heap::string*>(zen::types::heap::string::from_string("hello world"))
                            .get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt,most, -8, refer, -8, -24, 8, write_str, -8, -24, -32, most, 8, ret, push_i64, _p,
                  push_i64, -16, call, 1, most, 16, ret, most, -8, call, 0, push_i64, -8, push_i64, _p2, call, 0, most,
                  16, push_i64, -8, call, 14, most, 8, push_i64, -8, call, 0, most, 8, most, 8, ret,}));
}

TEST(composer_unit, sum_ints_as_doubles)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("sum_ints_as_doubles");
    composer->set_return_type("double");
    composer->set_parameter("x", "int");
    composer->set_parameter("y", "int");
    composer->push("x");
    composer->call("double", 1);
    composer->push("y");
    composer->call("double", 1);
    composer->plus();
    composer->return_value();
    composer->end();
    // composer->bake();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt,most, -8, i32_to_f64, -8, -24, most, -8, i32_to_f64, -8, -28, most, -8, add_f64, -8,
                  -24, -16, f64_to_f64, -48, -8, most, 24, ret,}));
}

TEST(composer_unit, _1_param_test)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("_1_param_test");
    composer->set_return_type("double");
    composer->set_parameter("x", "double");
    composer->push("x");
    composer->return_value();
    composer->end();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, f64_to_f64, -24, -16, ret,}));
}

TEST(composer_unit, _0_param_test)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("_0_param_test");
    composer->set_return_type("double");
    composer->push<double>(0.0, "double");
    composer->return_value();
    composer->end();
    auto _p = (i64)composer->_pool.get<f64>(0.0).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, f64_to_f64, -16, _p, ret,}));
}

TEST(composer_unit, cast_int_to_double)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("internal::cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push("x");
    composer->push<i32>(20, "int");
    composer->call("double", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<i32>(20).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i32_to_f64, -8, _p, most, 8, ret,}));
}

using namespace zen;

// BYTE CASTING TESTS
TEST(composer_cast, cast_byte_to_byte)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("byte", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i8_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_byte_to_bool)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(1, "byte");
    composer->call("bool", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i8_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_byte_to_short)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("short", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -2, i8_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_byte_to_int)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("int", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i8_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_byte_to_long)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("long", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i8_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_byte_to_float)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("float", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i8_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_byte_to_double)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("double", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i8_to_f64, -8, _p, most, 8, ret}));
}

// BOOL CASTING TESTS
TEST(composer_cast, cast_bool_to_byte)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("byte", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i8_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_bool_to_bool)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("bool", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i8_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_bool_to_short)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("short", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -2, i8_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_bool_to_int)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("int", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i8_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_bool_to_long)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("long", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i8_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_bool_to_float)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("float", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i8_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_bool_to_double)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("double", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i8_to_f64, -8, _p, most, 8, ret}));
}

// SHORT CASTING TESTS
TEST(composer_cast, cast_short_to_byte)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("byte", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i16_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_short_to_bool)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i16>(1, "short");
    composer->call("bool", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i16_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_short_to_short)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("short", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -2, i16_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_short_to_int)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("int", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i16_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_short_to_long)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("long", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i16_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_short_to_float)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("float", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i16_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_short_to_double)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("double", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i16_to_f64, -8, _p, most, 8, ret}));
}

// INT CASTING TESTS
TEST(composer_cast, cast_int_to_byte)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("byte", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i32_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_int_to_bool)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i32>(1, "int");
    composer->call("bool", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i32_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_int_to_short)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("short", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -2, i32_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_int_to_int)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("int", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i32_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_int_to_long)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("long", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i32_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_int_to_float)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("float", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i32_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_int_to_double)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i32>(20, "int");
    composer->call("double", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(20).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i32_to_f64, -8, _p, most, 8, ret}));
}

// LONG CASTING TESTS
TEST(composer_cast, cast_long_to_byte)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("byte", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i64_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_long_to_bool)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i64>(1, "long");
    composer->call("bool", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, i64_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_long_to_short)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("short", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -2, i64_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_long_to_int)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("int", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i64_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_long_to_long)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("long", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i64_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_long_to_float)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("float", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i64_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_long_to_double)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("double", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, i64_to_f64, -8, _p, most, 8, ret}));
}

// FLOAT CASTING TESTS
TEST(composer_cast, cast_float_to_byte)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("byte", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, f32_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_float_to_bool)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push("x");
    composer->push<float>(1.0f, "float");
    composer->call("bool", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(1.0f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, f32_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_float_to_short)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("short", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -2, f32_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_float_to_int)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("int", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, f32_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_float_to_long)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("long", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, f32_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_float_to_float)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("float", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, f32_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_float_to_double)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("double", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, f32_to_f64, -8, _p, most, 8, ret}));
}

// DOUBLE CASTING TESTS
TEST(composer_cast, cast_double_to_byte)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("byte", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, f64_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_double_to_bool)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push("x");
    composer->push<double>(1.0, "double");
    composer->call("bool", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(1.0).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -1, f64_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_double_to_short)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("short", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -2, f64_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_double_to_int)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("int", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, f64_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_double_to_long)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("long", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, f64_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_double_to_float)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("float", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, f64_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_double_to_double)
{
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("double", 1);
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, f64_to_f64, -8, _p, most, 8, ret}));
}

TEST(composer_unit, sum_long_function)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("internal::sum");
    composer->set_return_type("long");
    composer->set_parameter("x", "long");
    composer->set_parameter("y", "long");
    composer->push("x");
    composer->push("y");
    composer->plus();
    composer->return_value();
    composer->end();

    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, add_i64, -8, -32, -24, i64_to_i64, -40, -8, most, 8, ret}));
}

TEST(composer_unit, times_two_double_function)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("internal::timesTwo");
    composer->set_return_type("double");
    composer->set_parameter("x", "double");
    composer->push<f64>(2.0, "double");
    composer->push("x");
    composer->times();
    composer->return_value();
    composer->end();

    auto _p = (i64)composer->_pool.get<f64>(2.0).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, mul_f64, -8, _p, -24, f64_to_f64, -32, -8, most, 8, ret}));
}

TEST(composer_unit, times_three_long_function)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("internal::timesThree");
    composer->set_return_type("long");
    composer->set_return_name("result");
    composer->set_parameter("x", "long");
    composer->set_local("result", "long");
    composer->push("result");
    composer->push<f64>(3.0, "long");
    composer->push("x");
    composer->times();
    composer->assign();
    composer->end();
    auto _p = (i64)composer->_pool.get<f64>(3.0).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, most, -8, mul_i64, -8, _p, -32, i64_to_i64, -16, -8, i64_to_i64, -40, -16
                  , most, 16, ret}));
}

TEST(composer_unit, conditional)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("conditional");
    composer->set_return_type("int");
    composer->set_parameter("cond", "bool");
    composer->set_parameter("x", "int");
    composer->set_parameter("y", "int");
    composer->push("cond");
    composer->begin_if_then();
    composer->push("x");
    composer->return_value();
    composer->else_then();
    composer->push("y");
    composer->return_value();
    composer->end_if();
    composer->end();
    // auto _p = (i64)composer->_pool.get<f64>(3.0).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, go_if_not, -17, 5, i32_to_i32, -21, -16, go, 3, i32_to_i32, -21, -12, ret,}));
}

// TEST(composer_unit, ternary)
// {
//     using namespace zen;
//     int ilc_offset = 0;
//     const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
//     composer->begin("ternary");
//     composer->set_return_type("int");
//     composer->set_parameter("a", "int");
//     composer->set_parameter("b", "int");
//     composer->set_parameter("c", "bool");
//     composer->push("c");
//     composer->push("a");
//     composer->push("b");
//     composer->ternary();
//     composer->return_value();
//     composer->end();
//     EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
//               (std::vector<i64>{hlt, most, -4, go_if_not, -13, 5, i32_to_i32, -4, -21, go, 3, i32_to_i32, -4, -17,
//                   i32_to_i32, -25, -4, most, 4, ret,}));
// }

TEST(composer_unit, while_)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("while");
    composer->set_return_type("int");
    composer->set_return_name("result");
    composer->set_parameter("begin", "int");
    composer->set_parameter("end", "int");
    composer->set_local("result", "int");
    composer->push("result");
    composer->push<zen::i32>(0, "int");
    composer->assign();
    composer->begin_while();
    composer->push("begin");
    composer->push("end");
    composer->lower_or_equal();
    composer->set_while_condition();
    composer->push("result");
    composer->push("result");
    composer->push("begin");
    composer->post_increment();
    composer->plus();
    composer->end_while();
    composer->end();
    composer->bake();
    auto _p2 = (i64)composer->_pool.get<i32>(1).get(); // increment
    auto _p = (i64)composer->_pool.get<i32>(0).get();
    const auto _c = std::vector<i64>{
        hlt, most, -4, i32_to_i32, -4, _p, most, -1, lte_i32, -1, -21, -17, go_if_not, -1, 21, most, -4, i32_to_i32, -4,
        -25, add_i32, -25, -25, _p2, most, -4, add_i32, -4, -29, -8, most, 8, most, 1, go, -27, i32_to_i32, -24, -4,
        most, 4, ret,
    };
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code, _c);
}

TEST(composer_unit, for1_)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("for1");
    composer->set_return_type("int");
    composer->set_return_name("result");
    composer->set_parameter("begin", "int");
    composer->set_parameter("end", "int");
    composer->set_local("result", "int");
    composer->push("result");
    composer->push<zen::i32>(0, "int");
    composer->assign();
    composer->begin_for();
    composer->set_local("i", "int");
    composer->push("i");
    composer->push("begin");
    composer->push("end");
    composer->set_for_begin_end();
    composer->push("result");
    composer->push("result");
    composer->push("i");
    composer->plus();
    composer->assign();
    composer->end_for();
    composer->end();
    auto _p2 = (i64)composer->_pool.get<i32>(1).get(); // increment
    auto _p = (i64)composer->_pool.get<i32>(0).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i32_to_i32, -4, _p, most, -4, i32_to_i32, -4, -24, most, -4, i32_to_i32,
                  -4, -8, add_i32, -8, -8, _p2, most, -1, lte_i32, -1, -5, -25, go_if_not, -1, 11, most, -4, add_i32, -4
                  , -17, -13, i32_to_i32, -17, -4, go, -26, i32_to_i32, -37, -17, most, 17, ret,}));
}

TEST(composer_unit, for2_)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("sum_using_reverse");
    composer->set_return_type("int");
    composer->set_return_name("result");
    composer->set_parameter("begin", "int");
    composer->set_parameter("end", "int");
    composer->set_local("result", "int");
    composer->push("result");
    composer->push<zen::i32>(0, "int");
    composer->assign();
    composer->begin_for();
    composer->set_local("i", "int");
    composer->push("i");
    composer->push("end");
    composer->push("begin");
    composer->push<zen::i32>(-1, "int");
    composer->set_for_begin_end_step();
    composer->push("result");
    composer->push("result");
    composer->push("i");
    composer->plus();
    composer->assign();
    composer->end_for();
    composer->end();
    auto _p2 = (i64)composer->_pool.get<i32>(-1).get(); // increment
    auto _p = (i64)composer->_pool.get<i32>(0).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, i32_to_i32, -4, _p, most, -4, i32_to_i32, -4, -20, most, -4, add_i32, -4,
                  -8, _p2, most, -1, lte_i32, -1, -5, -29, go_if_not, -1, 11, most, -4, add_i32, -4, -17, -13,
                  i32_to_i32, -17, -4, go, -23, i32_to_i32, -37, -17, most, 17, ret}));
}

TEST(composer_unit, scope)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("scope_test");
    composer->set_local("result", "int");
    composer->set_local("x", "int");
    composer->set_local("y", "int");
    composer->begin_while();
    composer->push<zen::boolean>(true, "bool");
    composer->set_while_condition();
    composer->set_local("result", "int");
    composer->set_local("x", "int");
    composer->set_local("y", "int");
    composer->push("result");
    composer->push("x");
    composer->push("y");
    composer->plus();
    composer->assign();
    composer->end_while();
    composer->push("result");
    composer->push("x");
    composer->push("y");
    composer->plus();
    composer->assign();
    composer->end();
    composer->bake();
    auto _p = (i64)composer->_pool.get<zen::boolean>(true).get(); // increment
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -4, most, -4, most, -4, go_if_not, _p, 19, most, -4, most, -4, most, -4, most
                  , -4, add_i32, -4, -12, -8, i32_to_i32, -16, -4, most, 16, go, -19, most, -4, add_i32, -4, -12, -8,
                  i32_to_i32, -16, -4, most, 16, ret,}));
}

TEST(composer_unit, overloading)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("sum");
    composer->set_parameter("a", "long");
    composer->set_parameter("b", "long");
    composer->push("a");
    composer->push("b");
    composer->plus();
    composer->end();

    composer->begin("sum");
    composer->set_parameter("a", "int");
    composer->set_parameter("b", "int");
    composer->push("a");
    composer->push("b");
    composer->plus();
    composer->end();

    composer->begin("callee");
    composer->push<i64>(1, "long");
    composer->push<i64>(1, "long");
    composer->push("sum");
    composer->call("sum", 2);
    composer->push<i32>(1, "int");
    composer->push<i32>(1, "int");
    composer->push("sum");
    composer->call("sum", 2);
    composer->end();
    // composer->bake();
    const auto _p = (i64)composer->_pool.get<i64>(1).get();
    auto _p2 = (i64)composer->_pool.get<i32>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
              (std::vector<i64>{hlt, most, -8, add_i64, -8, -32, -24, most, 8, ret, most, -4, add_i32, -4, -20, -16,
                  most, 4, ret, push_i64, _p,push_i64, _p, call, 1, most, 16, push_i32, _p2,push_i32, _p2, call, 10,most
                  , 8, ret,}));
}

/**
 * todo: add scope tests just to ensure that conditional returning is ok
 */
/*
      auto scope = zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_block);
        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_if));
        scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;
        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_between_branches));
        scope->__dncd__peek(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_else));
        scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);

        scope->__dncd__pop(scope->return_status);

        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_if));
        scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_if));
        scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_if));
        // scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_else));
        scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_else));
        scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_if));
        // scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->__dncd__push(zen::composer::vm::block_scope::__unsafely_make(zen::composer::vm::scope::in_else));
        scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        scope->__dncd__pop(scope->return_status);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;

        scope->set_return_status(zen::composer::vm::block_scope::concise_return);
        std::cout << __LINE__ << " " << scope->get_return_status() << std::endl;
        */
