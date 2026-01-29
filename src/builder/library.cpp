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

    std::shared_ptr<generic_context> library::get_generic_function(i64 id)
    {
        const auto target = generic_functions.find(id);
        return target != generic_functions.end() ? target->second : nullptr;
    }

    std::shared_ptr<generic_context> library::get_generic_type(i64 id)
    {
        const auto target = generic_types.find(id);
        return target != generic_types.end() ? target->second : nullptr;
    }

    std::shared_ptr<type> library::get_type(i64 id)
    {
        const auto target = types.find(id);
        return target != types.end() ? target->second : nullptr;
    }

    void library::add(const std::shared_ptr<function>& fn)
    {
        functions[fn->hash()] = fn;
    }

    void library::add_generic_type(const std::shared_ptr<builder::generic_context> & gc)
    {
        generic_types[gc->hash()] = gc;
    }

    void library::add_generic_function(const std::shared_ptr<builder::generic_context> & gc)
    {
        generic_functions[gc->hash()] = gc;
    }

    void library::add(const std::shared_ptr<type>& ty)
    {
        types[ty->hash()] = ty;
    }

    std::shared_ptr<library> library::create(const std::string& name)
    {
        auto lib = std::make_shared<library>(name);
        return lib;
    }
}
