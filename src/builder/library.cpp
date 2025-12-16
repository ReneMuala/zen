//
// Created by dte on 12/15/2025.
//

#include "library.hpp"

namespace zen::builder
{
    i64 library::hash() const
    {
        static constexpr std::hash<std::string> hasher;
        return hasher(name);
    }

    std::shared_ptr<builder::function> library::get(const i64 id)
    {
        const auto target = functions.find(id);
        return target != functions.end() ? target->second : nullptr;
    }

    void library::add(std::shared_ptr<builder::function> fn)
    {
        functions[fn->hash()] = fn;
    }

    std::shared_ptr<library> library::create(const std::string& name)
    {
        auto lib = std::make_shared<library>(name);
        return lib;
    }
}
