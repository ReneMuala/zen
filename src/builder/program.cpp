//
// Created by dte on 12/15/2025.
//

#include "program.hpp"

#include "exceptions/link_error.hpp"

namespace zen::builder
{
    std::shared_ptr<builder::program> program::create()
    {
        return std::make_shared<builder::program>();
    }

    void program::add(const std::shared_ptr<builder::library>& lib)
    {
        libraries[lib->hash()] = lib;
    }

    void program::remove(const std::shared_ptr<builder::library>& lib)
    {
        libraries.erase(lib->hash());
    }

    i64 program::link(const std::shared_ptr<builder::function>& fn)
    {
        if (fn->scope)
        {
            throw exceptions::link_error(fmt::format("cannot link function {} before building it", fn->get_canonical_name()));
        }
        const auto hash = fn->hash();
        if (links.contains(hash)) return links[hash];
        const auto base = code.size();
        links[hash] = base;
        code.insert(code.end(), fn->code.begin(), fn->code.end());
        for (const auto & [fst, snd] : fn -> dependencies)
        {
            if (const auto & dep_fb = get(fst))
            {
                const auto dep_base = link(dep_fb);
                snd->bind(code, base, dep_base);
            } else
            {
                throw exceptions::link_error(fmt::format("no such function {} needed by {}", snd->name, fn->get_canonical_name()));
            }
        }
        return base;
    }

    void program::run()
    {
    }

    std::shared_ptr<builder::function> program::get(i32 id) const
    {
        for (auto & lib : libraries)
        {
            if (auto func = lib.second->get(id))
            {
                return func;
            }
        }
        return nullptr;
    }
}
