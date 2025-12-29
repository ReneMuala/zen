//
// Created by dte on 12/16/2025.
//

#include "casting.hpp"

#include "builder/table.hpp"
#include "composer/composer.hpp"

namespace zen::library::casting
{
    inline std::shared_ptr<builder::function> create_convert_int_to_byte(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "byte");
        fn->set_return(zen::builder::function::_byte());
        fn->gen<zen::i32_to_i8>(fn->ret, fn->set_parameter(zen::builder::function::_int(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_int_to_bool(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "bool");
        fn->set_return(zen::builder::function::_bool());
        fn->gen<zen::i32_to_boolean>(fn->ret, fn->set_parameter(zen::builder::function::_int(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }


    inline std::shared_ptr<builder::function> create_convert_int_to_long(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "long");
        fn->set_return(zen::builder::function::_long());
        fn->gen<zen::i32_to_i64>(fn->ret, fn->set_parameter(zen::builder::function::_int(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_int_to_short(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "short");
        fn->set_return(zen::builder::function::_short());
        fn->gen<zen::i32_to_i16>(fn->ret, fn->set_parameter(zen::builder::function::_int(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_int_to_float(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "float");
        fn->set_return(zen::builder::function::_float());
        fn->gen<zen::i32_to_f32>(fn->ret, fn->set_parameter(zen::builder::function::_int(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_int_to_double(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "double");
        fn->set_return(zen::builder::function::_double());
        fn->gen<zen::i32_to_f64>(fn->ret, fn->set_parameter(zen::builder::function::_int(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    // ========== BYTE CONVERSIONS ==========

    inline std::shared_ptr<builder::function> create_convert_byte_to_int(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "int");
        fn->set_return(zen::builder::function::_int());
        fn->gen<zen::i8_to_i32>(fn->ret, fn->set_parameter(zen::builder::function::_byte(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_byte_to_bool(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "bool");
        fn->set_return(zen::builder::function::_bool());
        fn->gen<zen::i8_to_boolean>(fn->ret, fn->set_parameter(zen::builder::function::_byte(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_byte_to_long(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "long");
        fn->set_return(zen::builder::function::_long());
        fn->gen<zen::i8_to_i64>(fn->ret, fn->set_parameter(zen::builder::function::_byte(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_byte_to_short(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "short");
        fn->set_return(zen::builder::function::_short());
        fn->gen<zen::i8_to_i16>(fn->ret, fn->set_parameter(zen::builder::function::_byte(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_byte_to_float(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "float");
        fn->set_return(zen::builder::function::_float());
        fn->gen<zen::i8_to_f32>(fn->ret, fn->set_parameter(zen::builder::function::_byte(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_byte_to_double(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "double");
        fn->set_return(zen::builder::function::_double());
        fn->gen<zen::i8_to_f64>(fn->ret, fn->set_parameter(zen::builder::function::_byte(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    // ========== BOOL CONVERSIONS ==========

    inline std::shared_ptr<builder::function> create_convert_bool_to_int(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "int");
        fn->set_return(zen::builder::function::_int());
        fn->gen<zen::i8_to_i32>(fn->ret, fn->set_parameter(zen::builder::function::_bool(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_bool_to_byte(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "byte");
        fn->set_return(zen::builder::function::_byte());
        fn->gen<zen::i8_to_i8>(fn->ret, fn->set_parameter(zen::builder::function::_bool(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_bool_to_long(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "long");
        fn->set_return(zen::builder::function::_long());
        fn->gen<zen::i8_to_i64>(fn->ret, fn->set_parameter(zen::builder::function::_bool(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_bool_to_short(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "short");
        fn->set_return(zen::builder::function::_short());
        fn->gen<zen::i8_to_i16>(fn->ret, fn->set_parameter(zen::builder::function::_bool(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_bool_to_float(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "float");
        fn->set_return(zen::builder::function::_float());
        fn->gen<zen::i8_to_f32>(fn->ret, fn->set_parameter(zen::builder::function::_bool(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_bool_to_double(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "double");
        fn->set_return(zen::builder::function::_double());
        fn->gen<zen::i8_to_f64>(fn->ret, fn->set_parameter(zen::builder::function::_bool(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    // ========== LONG CONVERSIONS ==========

    inline std::shared_ptr<builder::function> create_convert_long_to_int(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "int");
        fn->set_return(zen::builder::function::_int());
        fn->gen<zen::i64_to_i32>(fn->ret, fn->set_parameter(zen::builder::function::_long(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_long_to_byte(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "byte");
        fn->set_return(zen::builder::function::_byte());
        fn->gen<zen::i64_to_i8>(fn->ret, fn->set_parameter(zen::builder::function::_long(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_long_to_bool(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "bool");
        fn->set_return(zen::builder::function::_bool());
        fn->gen<zen::i64_to_boolean>(fn->ret, fn->set_parameter(zen::builder::function::_long(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_long_to_short(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "short");
        fn->set_return(zen::builder::function::_short());
        fn->gen<zen::i64_to_i16>(fn->ret, fn->set_parameter(zen::builder::function::_long(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_long_to_float(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "float");
        fn->set_return(zen::builder::function::_float());
        fn->gen<zen::i64_to_f32>(fn->ret, fn->set_parameter(zen::builder::function::_long(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_long_to_double(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "double");
        fn->set_return(zen::builder::function::_double());
        fn->gen<zen::i64_to_f64>(fn->ret, fn->set_parameter(zen::builder::function::_long(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    // ========== SHORT CONVERSIONS ==========

    inline std::shared_ptr<builder::function> create_convert_short_to_int(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "int");
        fn->set_return(zen::builder::function::_int());
        fn->gen<zen::i16_to_i32>(fn->ret, fn->set_parameter(zen::builder::function::_short(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_short_to_byte(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "byte");
        fn->set_return(zen::builder::function::_byte());
        fn->gen<zen::i16_to_i8>(fn->ret, fn->set_parameter(zen::builder::function::_short(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_short_to_bool(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "bool");
        fn->set_return(zen::builder::function::_bool());
        fn->gen<zen::i16_to_boolean>(fn->ret, fn->set_parameter(zen::builder::function::_short(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_short_to_long(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "long");
        fn->set_return(zen::builder::function::_long());
        fn->gen<zen::i16_to_i64>(fn->ret, fn->set_parameter(zen::builder::function::_short(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_short_to_float(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "float");
        fn->set_return(zen::builder::function::_float());
        fn->gen<zen::i16_to_f32>(fn->ret, fn->set_parameter(zen::builder::function::_short(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_short_to_double(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "double");
        fn->set_return(zen::builder::function::_double());
        fn->gen<zen::i16_to_f64>(fn->ret, fn->set_parameter(zen::builder::function::_short(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    // ========== FLOAT CONVERSIONS ==========

    inline std::shared_ptr<builder::function> create_convert_float_to_int(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "int");
        fn->set_return(zen::builder::function::_int());
        fn->gen<zen::f32_to_i32>(fn->ret, fn->set_parameter(zen::builder::function::_float(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_float_to_byte(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "byte");
        fn->set_return(zen::builder::function::_byte());
        fn->gen<zen::f32_to_i8>(fn->ret, fn->set_parameter(zen::builder::function::_float(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_float_to_bool(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "bool");
        fn->set_return(zen::builder::function::_bool());
        fn->gen<zen::f32_to_boolean>(fn->ret, fn->set_parameter(zen::builder::function::_float(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_float_to_long(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "long");
        fn->set_return(zen::builder::function::_long());
        fn->gen<zen::f32_to_i64>(fn->ret, fn->set_parameter(zen::builder::function::_float(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_float_to_short(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "short");
        fn->set_return(zen::builder::function::_short());
        fn->gen<zen::f32_to_i16>(fn->ret, fn->set_parameter(zen::builder::function::_float(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_float_to_double(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "double");
        fn->set_return(zen::builder::function::_double());
        fn->gen<zen::f32_to_f64>(fn->ret, fn->set_parameter(zen::builder::function::_float(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    // ========== DOUBLE CONVERSIONS ==========

    inline std::shared_ptr<builder::function> create_convert_double_to_int(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "int");
        fn->set_return(zen::builder::function::_int());
        fn->gen<zen::f64_to_i32>(fn->ret, fn->set_parameter(zen::builder::function::_double(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_double_to_byte(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "byte");
        fn->set_return(zen::builder::function::_byte());
        fn->gen<zen::f64_to_i8>(fn->ret, fn->set_parameter(zen::builder::function::_double(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_double_to_bool(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "bool");
        fn->set_return(zen::builder::function::_bool());
        fn->gen<zen::f64_to_boolean>(fn->ret, fn->set_parameter(zen::builder::function::_double(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_double_to_long(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "long");
        fn->set_return(zen::builder::function::_long());
        fn->gen<zen::f64_to_i64>(fn->ret, fn->set_parameter(zen::builder::function::_double(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_double_to_short(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "short");
        fn->set_return(zen::builder::function::_short());
        fn->gen<zen::f64_to_i16>(fn->ret, fn->set_parameter(zen::builder::function::_double(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_double_to_float(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "float");
        fn->set_return(zen::builder::function::_float());
        fn->gen<zen::f64_to_f32>(fn->ret, fn->set_parameter(zen::builder::function::_double(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    // ========== STRING CONVERSIONS ==========

    inline std::shared_ptr<builder::function> create_convert_int_to_str(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string");
        fn->set_return(zen::builder::function::_string());
        fn->gen<zen::i32_to_str>(fn->ret, fn->set_parameter(zen::builder::function::_int(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_byte_to_str(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string");
        fn->set_return(zen::builder::function::_string());
        fn->gen<zen::i8_to_str>(fn->ret, fn->set_parameter(zen::builder::function::_byte(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_bool_to_str(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string");
        fn->set_return(zen::builder::function::_string());
        fn->branch(zen::builder::scope::in_if, fn->set_parameter(zen::builder::function::_bool(), "it"), [&](auto &, auto & pel,auto& pen)
        {
            fn->return_value(fn->constant<std::string>("true"));
            fn->branch(zen::builder::scope::in_else, nullptr, [&](auto &, auto &, auto&)
            {
                fn->return_value(fn->constant<std::string>("false"));
            }, pel,pen);
        });
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_long_to_str(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string");
        fn->set_return(zen::builder::function::_string());
        fn->gen<zen::i64_to_str>(fn->ret, fn->set_parameter(zen::builder::function::_long(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_short_to_str(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string");
        fn->set_return(zen::builder::function::_string());
        fn->gen<zen::i16_to_str>(fn->ret, fn->set_parameter(zen::builder::function::_short(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_float_to_str(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string");
        fn->set_return(zen::builder::function::_string());
        fn->gen<zen::f32_to_str>(fn->ret, fn->set_parameter(zen::builder::function::_float(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_convert_double_to_str(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string");
        fn->set_return(zen::builder::function::_string());
        fn->gen<zen::f64_to_str>(fn->ret, fn->set_parameter(zen::builder::function::_double(), "it"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    std::shared_ptr<zen::builder::library> create(utils::constant_pool& pool)
    {
        std::shared_ptr<zen::builder::library> library = zen::builder::library::create("casting");

        // INT conversions
        library->add(create_convert_int_to_byte(pool));
        library->add(create_convert_int_to_bool(pool));
        library->add(create_convert_int_to_short(pool));
        library->add(create_convert_int_to_long(pool));
        library->add(create_convert_int_to_float(pool));
        library->add(create_convert_int_to_double(pool));

        // BYTE conversions
        library->add(create_convert_byte_to_int(pool));
        library->add(create_convert_byte_to_bool(pool));
        library->add(create_convert_byte_to_short(pool));
        library->add(create_convert_byte_to_long(pool));
        library->add(create_convert_byte_to_float(pool));
        library->add(create_convert_byte_to_double(pool));

        // BOOL conversions
        library->add(create_convert_bool_to_int(pool));
        library->add(create_convert_bool_to_byte(pool));
        library->add(create_convert_bool_to_short(pool));
        library->add(create_convert_bool_to_long(pool));
        library->add(create_convert_bool_to_float(pool));
        library->add(create_convert_bool_to_double(pool));

        // LONG conversions
        library->add(create_convert_long_to_int(pool));
        library->add(create_convert_long_to_byte(pool));
        library->add(create_convert_long_to_bool(pool));
        library->add(create_convert_long_to_short(pool));
        library->add(create_convert_long_to_float(pool));
        library->add(create_convert_long_to_double(pool));

        // SHORT conversions
        library->add(create_convert_short_to_int(pool));
        library->add(create_convert_short_to_byte(pool));
        library->add(create_convert_short_to_bool(pool));
        library->add(create_convert_short_to_long(pool));
        library->add(create_convert_short_to_float(pool));
        library->add(create_convert_short_to_double(pool));

        // FLOAT conversions
        library->add(create_convert_float_to_int(pool));
        library->add(create_convert_float_to_byte(pool));
        library->add(create_convert_float_to_bool(pool));
        library->add(create_convert_float_to_long(pool));
        library->add(create_convert_float_to_short(pool));
        library->add(create_convert_float_to_double(pool));

        // DOUBLE conversions
        library->add(create_convert_double_to_int(pool));
        library->add(create_convert_double_to_byte(pool));
        library->add(create_convert_double_to_bool(pool));
        library->add(create_convert_double_to_long(pool));
        library->add(create_convert_double_to_short(pool));
        library->add(create_convert_double_to_float(pool));

        // STRING conversions (to string)
        library->add(create_convert_int_to_str(pool));
        library->add(create_convert_byte_to_str(pool));
        library->add(create_convert_bool_to_str(pool));
        library->add(create_convert_long_to_str(pool));
        library->add(create_convert_short_to_str(pool));
        library->add(create_convert_float_to_str(pool));
        library->add(create_convert_double_to_str(pool));

        return library;
    }
}
