//
// Created by dte on 12/15/2025.
//

#include "library.hpp"

namespace zen::builder
{
    int library::hash() const
    {
        static constexpr std::hash<std::string> hasher;
        return hasher(name);
    }

    std::shared_ptr<builder::function> library::get(const i32 id)
    {
        return functions[id];
    }

    void library::add(const std::shared_ptr<builder::function>& fn)
    {
        functions[fn->hash()] = fn;
    }

    std::shared_ptr<library> library::create(const std::string& name)
    {
        auto lib = std::make_shared<library>(name);
        return lib;
    }
}
