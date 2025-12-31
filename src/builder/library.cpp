//
// Created by dte on 12/15/2025.
//

#include "library.hpp"

namespace zen::builder
{
    void library::add_test(const std::shared_ptr<builder::function>& item)
    {
        for (auto const & test : tests)
        {
            if (test->hash() == item->hash())
                return;
        }
        tests.push_back(item);
    }

    i64 library::hash() const
    {
        static constexpr std::hash<std::string> hasher;
        return hasher(name);
    }

    std::shared_ptr<function> library::get_function(i64 id)
    {
        const auto target = functions.find(id);
        return target != functions.end() ? target->second : nullptr;
    }

    std::shared_ptr<type> library::get_type(i64 id)
    {
        const auto target = types.find(id);
        return target != types.end() ? target->second : nullptr;
    }

    void library::add(std::shared_ptr<builder::function> fn)
    {
        functions[fn->hash()] = fn;
    }

    void library::add(std::shared_ptr<builder::type> ty)
    {
        types[ty->hash()] = ty;
    }

    std::shared_ptr<library> library::create(const std::string& name)
    {
        auto lib = std::make_shared<library>(name);
        return lib;
    }
}
