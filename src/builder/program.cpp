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

    void program::link(const std::shared_ptr<builder::function>& fn)
    {
        const auto hash = fn->hash();
        if (links.contains(hash)) return;
        links[hash] = code.size();
        code.insert(code.end(), fn->code.begin(), fn->code.end());
        for (const auto & [fst, snd] : fn -> dependencies)
        {
            if (const auto & dep_fb = get(fst))
            {
                link(dep_fb);
            } else
            {
                throw exceptions::link_error(fmt::format("no such function {} needed by {}", snd, fn->get_canonical_name()));
            }
        }
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
