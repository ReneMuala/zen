//
// Created by dte on 12/16/2025.
//

#include "io.hpp"

#include "builder/table.hpp"

namespace zen::library::io
{
    inline std::shared_ptr<builder::function> create_print_string(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "print");
        const auto tab = zen::builder::table::create(fn);
        const auto it = fn->set_parameter(zen::builder::function::_string(), "str");
        const auto it_data_field = tab->get_field_or_throw(it, "string::data");
        const auto it_len_field = tab->get_field_or_throw(it, "string::len");
        fn->gen<zen::write_str>(fn->dereference(it_data_field), fn->dereference(it_len_field), fn->constant<i64>(reinterpret_cast<zen::i64>(stdout)));
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println_string(utils::constant_pool & pool, std::shared_ptr<zen::builder::function> print_str)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        const auto it = fn->set_parameter(zen::builder::function::_string(), "str");
        if (auto r = fn->call(print_str, {it}); not r.has_value())
            throw zen::exceptions::semantic_error(r.error(), 0);
        if (auto r = fn->call(print_str, {fn->constant<std::string>("\n")}); not r.has_value())
            throw zen::exceptions::semantic_error(r.error(), 0);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_print_bool(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "print");
        const auto caster_fun = fn->create("string", {zen::builder::function::_bool()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("print", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_bool(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto print_result = fn->call(print_string_fun, {string_value}); not print_result.has_value())
            throw zen::exceptions::semantic_error(print_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println_bool(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        const auto caster_fun = fn->create("string", {zen::builder::function::_bool()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("println", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_bool(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto println_result = fn->call(print_string_fun, {string_value}); not println_result.has_value())
            throw zen::exceptions::semantic_error(println_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_print_byte(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "print");
        const auto caster_fun = fn->create("string", {zen::builder::function::_byte()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("print", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_byte(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto print_result = fn->call(print_string_fun, {string_value}); not print_result.has_value())
            throw zen::exceptions::semantic_error(print_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println_byte(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        const auto caster_fun = fn->create("string", {zen::builder::function::_byte()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("println", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_byte(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto println_result = fn->call(print_string_fun, {string_value}); not println_result.has_value())
            throw zen::exceptions::semantic_error(println_result.error(), fn->offset);
        fn->build();
        return fn;
    }

     inline std::shared_ptr<builder::function> create_print_short(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "print");
        const auto caster_fun = fn->create("string", {zen::builder::function::_short()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("print", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_short(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto print_result = fn->call(print_string_fun, {string_value}); not print_result.has_value())
            throw zen::exceptions::semantic_error(print_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println_short(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        const auto caster_fun = fn->create("string", {zen::builder::function::_short()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("println", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_short(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto println_result = fn->call(print_string_fun, {string_value}); not println_result.has_value())
            throw zen::exceptions::semantic_error(println_result.error(), fn->offset);
        fn->build();
        return fn;
    }

     inline std::shared_ptr<builder::function> create_print_int(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "print");
        const auto caster_fun = fn->create("string", {zen::builder::function::_int()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("print", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_int(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto print_result = fn->call(print_string_fun, {string_value}); not print_result.has_value())
            throw zen::exceptions::semantic_error(print_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println_int(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        const auto caster_fun = fn->create("string", {zen::builder::function::_int()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("println", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_int(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto println_result = fn->call(print_string_fun, {string_value}); not println_result.has_value())
            throw zen::exceptions::semantic_error(println_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_print_long(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "print");
        const auto caster_fun = fn->create("string", {zen::builder::function::_long()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("print", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_long(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto print_result = fn->call(print_string_fun, {string_value}); not print_result.has_value())
            throw zen::exceptions::semantic_error(print_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println_long(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        const auto caster_fun = fn->create("string", {zen::builder::function::_long()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("println", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_long(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto println_result = fn->call(print_string_fun, {string_value}); not println_result.has_value())
            throw zen::exceptions::semantic_error(println_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_print_float(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "print");
        const auto caster_fun = fn->create("string", {zen::builder::function::_float()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("print", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_float(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto print_result = fn->call(print_string_fun, {string_value}); not print_result.has_value())
            throw zen::exceptions::semantic_error(print_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println_float(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        const auto caster_fun = fn->create("string", {zen::builder::function::_float()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("println", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_float(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto println_result = fn->call(print_string_fun, {string_value}); not println_result.has_value())
            throw zen::exceptions::semantic_error(println_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_print_double(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "print");
        const auto caster_fun = fn->create("string", {zen::builder::function::_double()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("print", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_double(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto print_result = fn->call(print_string_fun, {string_value}); not print_result.has_value())
            throw zen::exceptions::semantic_error(print_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println_double(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        const auto caster_fun = fn->create("string", {zen::builder::function::_double()}, zen::builder::function::_string());
        const auto print_string_fun = fn->create("println", {zen::builder::function::_string()}, nullptr);
        const auto it = fn->set_parameter(zen::builder::function::_double(), "it");
        std::shared_ptr<builder::value> string_value;
        if (auto caster_result = fn->call(caster_fun, {it}); not caster_result.has_value())
            throw zen::exceptions::semantic_error(caster_result.error(), fn->offset);
        else
            string_value = caster_result.value();
        if (auto println_result = fn->call(print_string_fun, {string_value}); not println_result.has_value())
            throw zen::exceptions::semantic_error(println_result.error(), fn->offset);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_println(utils::constant_pool & pool, std::shared_ptr<zen::builder::function> print_str)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "println");
        if (auto r = fn->call(print_str, {fn->constant<std::string>("\n")}); not r.has_value())
            throw zen::exceptions::semantic_error(r.error(), 0);
        fn->build();
        return fn;
    }

    std::shared_ptr<zen::builder::library> create(utils::constant_pool & pool)
    {
        std::shared_ptr<zen::builder::library> library = zen::builder::library::create("io");
        const auto print_str = create_print_string(pool);
        const auto println_str = create_println_string(pool, print_str);
        const auto println = create_println(pool, print_str);
        library->add(print_str);
        library->add(println_str);
        library->add(create_print_bool(pool));
        library->add(create_println_bool(pool));
        library->add(create_print_byte(pool));
        library->add(create_println_byte(pool));
        library->add(create_print_short(pool));
        library->add(create_println_short(pool));
        library->add(create_print_int(pool));
        library->add(create_println_int(pool));
        library->add(create_print_long(pool));
        library->add(create_println_long(pool));
        library->add(create_print_float(pool));
        library->add(create_println_float(pool));
        library->add(create_print_double(pool));
        library->add(create_println_double(pool));
        library->add(println);
        return library;
    }
}
