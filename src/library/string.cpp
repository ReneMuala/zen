//
// Created by dte on 12/16/2025.
//

#include "string.hpp"

#include "builder/table.hpp"

namespace zen::library::string
{
    inline std::shared_ptr<builder::function> create_allocator(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string::allocate");
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

    inline std::shared_ptr<builder::function> create_operator_move(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "operator=");
        const auto tab = zen::builder::table::create(fn);
        const auto to = fn->set_parameter(zen::builder::function::_string(), "to");
        const auto from = fn->set_parameter(zen::builder::function::_string(), "from");

        const auto to_data_field = tab->get_field_or_throw(to, "string::data");
        const auto to_len_field = tab->get_field_or_throw(to, "string::len");
        const auto from_data_field = tab->get_field_or_throw(from, "string::data");
        const auto from_len_field = tab->get_field_or_throw(from, "string::len");
        const auto need_to_free = fn->set_local(zen::builder::function::_bool(), "need_to_free");

        fn->not_equal(need_to_free, to_data_field, fn->constant<i64>(0));
        fn->branch(builder::scope::in_if, need_to_free,
                   [&](const std::shared_ptr<builder::function>& _, auto _1, auto _2)
                   {
                       fn->gen<zen::deallocate>(fn->dereference(to_data_field));
                   });
        const auto from_data = fn->dereference(from_data_field);
        const auto from_len = fn->dereference(from_len_field);
        const auto data = fn->set_local(zen::builder::function::_long(), "data");
        fn->gen<zen::allocate>(data, from_len);
        fn->gen<zen::copy>(data, from_data, from_len);
        fn->move(to_data_field, data);
        fn->move(to_len_field, from_len_field);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_operator_plus(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "operator+");
        fn->set_return(zen::builder::function::_string());
        const auto lhs = fn->set_parameter(zen::builder::function::_string(), "lhs");
        const auto rhs = fn->set_parameter(zen::builder::function::_string(), "rhs");
        fn->gen<zen::add_str>(fn->ret, lhs, rhs);
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_operator_equals(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "operator==");
        fn->set_return(zen::builder::function::_bool());
        const auto lhs = fn->set_parameter(zen::builder::function::_string(), "lhs");
        const auto rhs = fn->set_parameter(zen::builder::function::_string(), "rhs");
        fn->gen<zen::eq_str>(fn->ret, lhs, rhs);
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_operator_not_equals(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "operator!=");
        fn->set_return(zen::builder::function::_bool());
        const auto lhs = fn->set_parameter(zen::builder::function::_string(), "lhs");
        const auto rhs = fn->set_parameter(zen::builder::function::_string(), "rhs");
        fn->gen<zen::eq_str>(fn->ret, lhs, rhs);
        fn->not_(fn->ret, fn->ret);
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_deallocator(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string::deallocate");
        const auto tab = zen::builder::table::create(fn);
        const auto type = builder::function::_string();
        const auto it = fn->set_parameter(type, "it");
        const auto has_data = fn->set_local(builder::function::_bool(), "has_data");
        const auto it_data = tab->get_field_or_throw(it, "string::data");
        fn->gen<zen::i64_to_boolean>(has_data, it_data);
        fn->branch(builder::scope::in_if, has_data, [&](auto& fb, auto&, auto&)
        {
            fn->gen<deallocate>(fn->dereference(it_data));
        });
        fn->gen<deallocate>(it);
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_method_len(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string.len");
        fn->set_return(zen::builder::function::_long());
        const auto this_ = fn->set_parameter(zen::builder::function::_string(), "this");
        const auto tab = zen::builder::table::create(fn);
        fn->move(fn->ret, tab->get_field_or_throw(this_, "string::len"));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_method_empty(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string.empty");
        fn->set_return(zen::builder::function::_bool());
        const auto this_ = fn->set_parameter(zen::builder::function::_string(), "this");
        const auto tab = zen::builder::table::create(fn);
        fn->equal(fn->ret, tab->get_field_or_throw(this_, "string::len"), fn->constant<i64>(0));
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_method_at(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string.at");
        fn->set_return(zen::builder::function::_byte());
        const auto this_ = fn->set_parameter(zen::builder::function::_string(), "this");
        const auto pos = fn->set_parameter(zen::builder::function::_long(), "pos");
        const auto tab = zen::builder::table::create(fn);
        const auto in_bounds = fn->set_local(zen::builder::function::_bool(), "in_bounds");
        fn->lower(in_bounds, pos, tab->get_field_or_throw(this_, "string::len"));
        fn->branch(zen::builder::scope::in_if, in_bounds, [&](auto& fb, auto& pel, auto& pen)
        {
            const auto ptr = fn->set_local(zen::builder::function::_long(), "ptr");
            fn->add(ptr, tab->get_field_or_throw(this_, "string::data"), pos);
            ptr->is_reference = true;
            ptr->type = zen::builder::function::_byte();
            fn->move(fn->ret, ptr);
            fn->return_implicitly();
            fn->branch(zen::builder::scope::in_else, nullptr, [&](auto& fb, auto&, auto&)
            {
                fn->return_value(fn->constant<i8>(0));
            }, pel, pen);
        });
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_method_slice(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string.slice");
        fn->set_return(zen::builder::function::_string());
        const auto this_ = fn->set_parameter(zen::builder::function::_string(), "this");
        const auto from = fn->set_parameter(zen::builder::function::_long(), "from");
        const auto to = fn->set_parameter(zen::builder::function::_long(), "to");
        const auto tab = zen::builder::table::create(fn);
        const auto from_less_than_zero = fn->set_local(zen::builder::function::_bool(), "from_less_than_zero");
        fn->lower(from_less_than_zero, from, fn->constant<i64>(0));
        fn->branch(zen::builder::scope::in_if, from_less_than_zero, [&](auto&, auto&, auto&)
        {
            fn->move(from, fn->constant<i64>(0));
        });
        const auto to_greater_than_len = fn->set_local(zen::builder::function::_bool(), "to_greater_than_len");
        const auto this_len = fn->dereference(tab->get_field_or_throw(this_, "string::len"));
        fn->greater(to_greater_than_len, to, this_len);
        fn->branch(zen::builder::scope::in_if, to_greater_than_len, [&](auto&, auto&, auto&)
        {
            fn->move(to, this_len);
        });
        const auto len = fn->set_local(zen::builder::function::_long(), "len");
        fn->sub(len, to, from);
        const auto len_greater_than_zero = fn->set_local(zen::builder::function::_bool(), "to_greater_than_len");
        fn->greater(len_greater_than_zero, len, fn->constant<i64>(0));
        fn->branch(zen::builder::scope::in_if, len_greater_than_zero, [&](auto&, auto&, auto&)
        {
            const auto slice = fn->set_local(zen::builder::function::_string(), "slice");
            const auto slice_data = tab->get_field_or_throw(slice, "string::data");
            const auto slice_len = tab->get_field_or_throw(slice, "string::len");
            fn->add(slice_data, tab->get_field_or_throw(this_, "string::data"), from);
            fn->move(slice_len, len);
            fn->move(fn->ret, slice);
            fn->move(slice_data, fn->constant<i64>(0));
            fn->move(slice_len, fn->constant<i64>(0));
        });
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_method_sub(utils::constant_pool& pool)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "string.sub");
        fn->set_return(zen::builder::function::_string());
        const auto this_ = fn->set_parameter(zen::builder::function::_string(), "this");
        const auto from = fn->set_parameter(zen::builder::function::_long(), "from");
        const auto len = fn->set_parameter(zen::builder::function::_long(), "len");
        const auto tab = zen::builder::table::create(fn);
        const auto from_less_than_zero = fn->set_local(zen::builder::function::_bool(), "from_less_than_zero");
        fn->lower(from_less_than_zero, from, fn->constant<i64>(0));
        fn->branch(zen::builder::scope::in_if, from_less_than_zero, [&](auto&, auto&, auto&)
        {
            fn->move(from, fn->constant<i64>(0));
        });
        fn->add(len, len, from);
        const auto len_greater_equal_this_len = fn->set_local(zen::builder::function::_bool(),
                                                              "len_greater_equal_this_len");
        const auto this_len = fn->dereference(tab->get_field_or_throw(this_, "string::len"));
        fn->greater_equal(len_greater_equal_this_len, len, this_len);
        fn->branch(zen::builder::scope::in_if, len_greater_equal_this_len, [&](auto&, auto& pel, auto& pen)
        {
            fn->move(len, this_len);
            fn->sub(len, len, fn->constant<i64>(1));
            fn->branch(zen::builder::scope::in_else, nullptr, [&](auto&, auto&, auto&)
            {
                fn->sub(len, len, from);
            }, pel, pen);
        });
        const auto len_greater_than_zero = fn->set_local(zen::builder::function::_bool(), "to_greater_than_len");
        fn->greater(len_greater_than_zero, len, fn->constant<i64>(0));
        fn->branch(zen::builder::scope::in_if, len_greater_than_zero, [&](auto&, auto&, auto&)
        {
            const auto slice = fn->set_local(zen::builder::function::_string(), "slice");
            const auto slice_data = tab->get_field_or_throw(slice, "string::data");
            const auto slice_len = tab->get_field_or_throw(slice, "string::len");
            fn->add(slice_data, tab->get_field_or_throw(this_, "string::data"), from);
            fn->move(slice_len, len);
            fn->move(fn->ret, slice);
            fn->move(slice_data, fn->constant<i64>(0));
            fn->move(slice_len, fn->constant<i64>(0));
        });
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    std::shared_ptr<zen::builder::library> create(utils::constant_pool& pool)
    {
        std::shared_ptr<zen::builder::library> library = zen::builder::library::create("string");
        library->add(create_allocator(pool));
        library->add(create_operator_move(pool));
        library->add(create_operator_plus(pool));
        library->add(create_operator_equals(pool));
        library->add(create_operator_not_equals(pool));
        library->add(create_method_len(pool));
        library->add(create_method_empty(pool));
        library->add(create_method_at(pool));
        library->add(create_method_slice(pool));
        library->add(create_method_sub(pool));
        library->add(create_deallocator(pool));
        return library;
    }
}
