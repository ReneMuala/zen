//
// Created by dte on 12/16/2025.
//

#include "string.hpp"

#include "builder/table.hpp"

namespace zen::library::string
{
    inline std::shared_ptr<builder::function> create_allocator(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "string::allocate");
        const auto type = builder::function::_string();
        const auto tab = zen::builder::table::create(fn);
        fn->set_return(type);
        fn->gen<zen::allocate>(fn->ret, fn->constant<i64>(type->get_full_size()));
        const auto len_field = tab->get_field_or_throw(fn->ret, "string::len");
        fn->move(len_field, fn->constant<i64>(0));
        const auto data_field = tab->get_field_or_throw(fn->ret, "string::data");
        fn->move(data_field, fn->constant<i64>(0));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_operator_move(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "operator=");
        const auto tab = zen::builder::table::create(fn);
        const auto to = fn->set_parameter(zen::builder::function::_string(), "to");
        const auto from = fn->set_parameter(zen::builder::function::_string(), "from");

        const auto to_data_field = tab->get_field_or_throw(to, "string::data");
        const auto to_len_field = tab->get_field_or_throw(to, "string::len");
        const auto from_data_field = tab->get_field_or_throw(from, "string::data");
        const auto from_len_field = tab->get_field_or_throw(from, "string::len");
        const auto need_to_free = fn->set_local(zen::builder::function::_bool(), "need_to_free");

        fn->not_equal(need_to_free, to_data_field, fn->constant<i64>(0));
        fn->branch(builder::scope::in_if, need_to_free, [&](const std::shared_ptr<builder::function>& _, auto _1, auto _2)
        {
            fn->gen<zen::deallocate>(fn->dereference(to_data_field));
        });
        const auto from_data = fn->dereference(from_data_field);
        const auto from_len = fn->dereference(from_len_field);
        const auto data = fn->set_local(zen::builder::function::_long(), "data");
        fn->gen<zen::allocate>(data, from_len);
        fn->gen<zen::copy>(data, from_data, from_len);
        fmt::println("data off {}", to_data_field->offset);
        fn->move(to_data_field, data);
        fn->move(to_len_field, from_len_field);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_deallocator(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "string::deallocate");
        const auto tab = zen::builder::table::create(fn);
        const auto type = builder::function::_string();
        const auto it = fn->set_parameter(type,"it");
        const auto has_data = fn->set_local(builder::function::_bool(),"has_data");
        const auto it_data = tab->get_field_or_throw(it, "string::data");
        fn->gen<zen::i64_to_boolean>(has_data, it_data);
        fn->branch(builder::scope::in_if, has_data, [&](auto & fb, auto&, auto&)
        {
            fn->gen<deallocate>(fn->dereference(it_data));
        });
        fn->gen<deallocate>(it);
        fn->build();
        return fn;
    }

    std::shared_ptr<zen::builder::library> create(utils::constant_pool & pool)
    {
        std::shared_ptr<zen::builder::library> library = zen::builder::library::create("string");
        library->add(create_allocator(pool));
        library->add(create_operator_move(pool));
        library->add(create_deallocator(pool));
        return library;
    }
}
