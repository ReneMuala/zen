//
// Created by dte on 12/16/2025.
//

#include "zen.hpp"

#include "builder/table.hpp"

#ifndef KAIZEN_VERSION
#define KAIZEN_VERSION "unspecified"
#endif

namespace zen::library::zen
{
    inline std::shared_ptr<builder::function> create_get_zen_version(utils::constant_pool & pool)
    {
        const auto fn = ::zen::builder::function::create(pool, 0,false, "getZenVersion");
        fn->set_return(::zen::builder::function::_string());
        fn->return_value(fn->constant<std::string>(std::string(KAIZEN_VERSION)));
        fn->build();
        return fn;
    }

    std::shared_ptr<::zen::builder::library> create(utils::constant_pool & pool)
    {
        std::shared_ptr<::zen::builder::library> library = ::zen::builder::library::create("zen");
        library->add(create_get_zen_version(pool));
        return library;
    }
}
