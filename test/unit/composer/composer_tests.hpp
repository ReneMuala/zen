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
    composer->begin("write_string");
    composer->set_parameter("fd", "long");
    composer->set_parameter("str", "string");
    composer->push("str.data");
    composer->push("str.len");
    composer->push("fd");
    composer->call(std::to_string(write_str), 3);
    composer->end();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code, (std::vector<i64>{hlt,walk, -8, 8, write_str, -8, -8, -16, most, 16, ret}));
}

TEST(composer_unit, print_string)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("write_string");
    composer->set_parameter("fd", "long");
    composer->set_parameter("str", "string");
    composer->push("str.data");
    composer->push("str.len");
    composer->push("fd");
    composer->call(std::to_string(write_str), 3);
    composer->end();

    composer->begin("print_string");
    composer->set_parameter("string", "string");
    composer->push("write_string");
    composer->push<i64>(reinterpret_cast<i64>(stdout), "long");
    composer->push("string");
    composer->call("write_string", 2);
    composer->end();
    auto _p = (i64)composer->_pool.get<i64>(reinterpret_cast<i64>(stdout)).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code, (std::vector<i64>{hlt,walk, -8, 8, write_str, -8, -8, -16, most, 16, ret, push_i64, _p, push_i64, -8, call, 1, most, 8, ret}));
}

TEST(composer_unit, print_string_2)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("write_string");
    composer->set_parameter("fd", "long");
    composer->set_parameter("str", "string");
    composer->push("str.data");
    composer->push("str.len");
    composer->push("fd");
    composer->call(std::to_string(write_str), 3);
    composer->end();

    composer->begin("print_string");
    composer->set_parameter("string", "string");
    composer->push("write_string");
    composer->push<i64>(reinterpret_cast<i64>(stdout), "long");
    composer->push("string");
    composer->call("write_string", 2);
    composer->end();

    composer->begin("print_string_2");
    composer->push("print_string");
    composer->push<zen::types::heap::string*>(zen::types::heap::string::from_string("hello world"), "string");
    composer->call("print_string", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<i64>(reinterpret_cast<i64>(stdout)).get();
    auto _p2 = (i64)composer->_pool.get<zen::types::heap::string*>(zen::types::heap::string::from_string("hello world")).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code, (std::vector<i64>{hlt,walk, -8, 8, write_str, -8, -8, -16, most, 16, ret, push_i64, _p, push_i64, -8, call, 1, most, 8, ret, push_i64, _p2, call, 11, ret,}));
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
    composer->push("<double>");
    composer->push("x");
    composer->call("double", -1);
    composer->push("<double>");
    composer->push("y");
    composer->call("double", -1);
    composer->plus();
    composer->return_value();
    composer->end();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code, (std::vector<i64>{hlt,most, -8, i32_to_f64, -8, -16, most, -8, i32_to_f64, -8, -20, most, -8, add_f64, -8, -24, -16, f64_to_f64, -40, -8, most, 32, ret,}));
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
    composer->end();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code, (std::vector<i64>{hlt,most, 8, ret,}));
}

TEST(composer_unit, _0_param_test)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("_0_param_test");
    composer->set_return_type("double");
    composer->push<double>(0.0, "double");
    composer->end();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code, (std::vector<i64>{hlt, ret,}));
}

TEST(composer_unit, cast_int_to_double)
{
    using namespace zen;
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("internal::cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push<i32>(20, "int");
    composer->call("double", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<i32>(20).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code, (std::vector<i64>{hlt, most, -8, i32_to_f64, -8, _p, most, 8, ret,}));
}
using namespace zen;

// BYTE CASTING TESTS
TEST(composer_cast, cast_byte_to_byte) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("byte", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i8_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_byte_to_bool) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push<zen::i8>(1, "byte");
    composer->call("bool", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i8_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_byte_to_short) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("short", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -2, i8_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_byte_to_int) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("int", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i8_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_byte_to_long) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("long", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i8_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_byte_to_float) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("float", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i8_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_byte_to_double) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push<zen::i8>(42, "byte");
    composer->call("double", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i8_to_f64, -8, _p, most, 8, ret}));
}

// BOOL CASTING TESTS
TEST(composer_cast, cast_bool_to_byte) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("byte", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i8_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_bool_to_bool) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("bool", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i8_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_bool_to_short) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("short", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -2, i8_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_bool_to_int) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("int", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i8_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_bool_to_long) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("long", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i8_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_bool_to_float) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("float", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i8_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_bool_to_double) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push<zen::i8>(1, "bool");
    composer->call("double", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i8>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i8_to_f64, -8, _p, most, 8, ret}));
}

// SHORT CASTING TESTS
TEST(composer_cast, cast_short_to_byte) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("byte", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i16_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_short_to_bool) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push<zen::i16>(1, "short");
    composer->call("bool", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i16_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_short_to_short) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("short", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -2, i16_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_short_to_int) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("int", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i16_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_short_to_long) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("long", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i16_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_short_to_float) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("float", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i16_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_short_to_double) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push<zen::i16>(42, "short");
    composer->call("double", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i16>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i16_to_f64, -8, _p, most, 8, ret}));
}

// INT CASTING TESTS
TEST(composer_cast, cast_int_to_byte) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("byte", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i32_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_int_to_bool) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push<zen::i32>(1, "int");
    composer->call("bool", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i32_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_int_to_short) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("short", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -2, i32_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_int_to_int) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("int", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i32_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_int_to_long) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("long", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i32_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_int_to_float) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push<zen::i32>(42, "int");
    composer->call("float", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i32_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_int_to_double) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push<zen::i32>(20, "int");
    composer->call("double", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i32>(20).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i32_to_f64, -8, _p, most, 8, ret}));
}

// LONG CASTING TESTS
TEST(composer_cast, cast_long_to_byte) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("byte", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i64_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_long_to_bool) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push<zen::i64>(1, "long");
    composer->call("bool", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(1).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, i64_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_long_to_short) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("short", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -2, i64_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_long_to_int) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("int", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i64_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_long_to_long) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("long", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i64_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_long_to_float) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("float", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, i64_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_long_to_double) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push<zen::i64>(42, "long");
    composer->call("double", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<zen::i64>(42).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, i64_to_f64, -8, _p, most, 8, ret}));
}

// FLOAT CASTING TESTS
TEST(composer_cast, cast_float_to_byte) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("byte", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, f32_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_float_to_bool) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push<float>(1.0f, "float");
    composer->call("bool", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(1.0f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, f32_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_float_to_short) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("short", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -2, f32_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_float_to_int) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("int", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, f32_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_float_to_long) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("long", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, f32_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_float_to_float) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("float", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, f32_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_float_to_double) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push<float>(42.5f, "float");
    composer->call("double", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<float>(42.5f).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, f32_to_f64, -8, _p, most, 8, ret}));
}

// DOUBLE CASTING TESTS
TEST(composer_cast, cast_double_to_byte) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "byte");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("byte", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, f64_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_double_to_bool) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "bool");
    composer->push("x");
    composer->push<double>(1.0, "double");
    composer->call("bool", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(1.0).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -1, f64_to_i8, -1, _p, most, 1, ret}));
}

TEST(composer_cast, cast_double_to_short) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "short");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("short", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -2, f64_to_i16, -2, _p, most, 2, ret}));
}

TEST(composer_cast, cast_double_to_int) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("int", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, f64_to_i32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_double_to_long) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "long");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("long", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, f64_to_i64, -8, _p, most, 8, ret}));
}

TEST(composer_cast, cast_double_to_float) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "float");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("float", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -4, f64_to_f32, -4, _p, most, 4, ret}));
}

TEST(composer_cast, cast_double_to_double) {
    int ilc_offset = 0;
    const std::unique_ptr<zen::composer::composer> composer = std::make_unique<zen::composer::vm::composer>(ilc_offset);
    composer->begin("cast_test");
    composer->set_local("x", "double");
    composer->push("x");
    composer->push<double>(42.5, "double");
    composer->call("double", 1);
    composer->end();
    auto _p = (i64)composer->_pool.get<double>(42.5).get();
    EXPECT_EQ(dynamic_cast<const zen::composer::vm::composer*>(composer.get())->code,
        (std::vector<i64>{hlt, most, -8, f64_to_f64, -8, _p, most, 8, ret}));
}

TEST(composer_unit, sum_long_function) {
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
        (std::vector<i64>{hlt, most, -8, add_i64, -8, -24, -16, i64_to_i64, -32, -8, most, 24, ret}));
}

TEST(composer_unit, times_two_double_function) {
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
        (std::vector<i64>{hlt, most, -8, mul_f64, -8, _p, -16, f64_to_f64, -24, -8, most, 16, ret}));
}

TEST(composer_unit, times_three_long_function) {
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
        (std::vector<i64>{hlt, most, -8, most, -8, mul_i64, -8, _p, -24, i64_to_i64, -16, -8, i64_to_i64, -32, -16, most, 24, ret}));
}