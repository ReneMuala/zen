//
// Created by dte on 12/16/2025.
//

#include "string.hpp"

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

    std::shared_ptr<zen::builder::library> create(utils::constant_pool & pool)
    {
        std::shared_ptr<zen::builder::library> library = zen::builder::library::create("io");
        const auto print_str = create_print_string(pool);
        const auto println_str = create_println_string(pool, print_str);
        library->add(print_str);
        library->add(println_str);
        return library;
    }
}
