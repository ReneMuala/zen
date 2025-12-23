//
// Created by dte on 12/16/2025.
//

#include "string.hpp"

#include "builder/table.hpp"

namespace zen::library::string
{
    inline std::shared_ptr<builder::function> create_allocator(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "zen::allocate");
        const auto type = builder::function::_string();
        fn->set_return(type);
        fn->gen<zen::allocate>(fn->ret, type->get_full_size(), fmt::format("{}::size:{}", type->name, type->get_full_size()));

        auto pointer_creator = [&](std::shared_ptr<zen::builder::value> & ptr, const std::shared_ptr<zen::builder::value>& original)
        {
            if (not ptr)
            {
                ptr = fn->set_local(zen::builder::function::_long(), "temp::field");
            } else
            {
                fn->gen<zen::add_i64>(ptr, original, ptr->offset, fmt::format("@offset:{}", ptr->offset));
            }
        };

        if (auto r = builder::table::get_field(fn->ret, {"string::len"}, pointer_creator); r.has_value())
        {
            fn->move(r.value(), fn->constant<i64>(0));
        } else
        {
            throw exceptions::semantic_error(r.error(),0);
        }
        if (auto r = builder::table::get_field(fn->ret, {"string::data"}, pointer_creator); r.has_value())
        {
            fn->move(r.value(), fn->constant<i64>(0));
        } else
        {
            throw exceptions::semantic_error(r.error(),0);
        }
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_deallocator(utils::constant_pool & pool)
    {
        const auto fn = zen::builder::function::create(pool, 0,false, "zen::deallocate");
        auto pointer_creator = [&](std::shared_ptr<zen::builder::value> & ptr, const std::shared_ptr<zen::builder::value>& original)
        {
            if (not ptr)
            {
                ptr = fn->set_local(zen::builder::function::_long(), "temp::field");
            } else
            {
                fn->gen<zen::add_i64>(ptr, original, ptr->offset, fmt::format("@offset:{}", ptr->offset));
            }
        };
        const auto type = builder::function::_string();
        const auto it = fn->set_parameter(type,"it");
        const auto has_data = fn->set_local(builder::function::_bool(),"has_data");
        std::shared_ptr<zen::builder::value> it_data;
        if (auto r = builder::table::get_field(it, {"string::data"}, pointer_creator); r.has_value())
        {
            it_data = r.value();
            fn->gen<zen::i64_to_boolean>(has_data, it_data);
        } else
            throw exceptions::semantic_error(r.error(), 0);
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
        library->add(create_deallocator(pool));
        return library;
    }
}
