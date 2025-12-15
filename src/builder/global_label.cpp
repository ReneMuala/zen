//
// Created by dte on 12/12/2025.
//
#include "global_label.hpp"
#include "function.hpp"

namespace zen::builder
{
    void global_label::use(std::shared_ptr<builder::function> function)
    {
        auto& code = function->code;
        if (code.empty())
            throw std::invalid_argument("cannot bind label in empty code");
        if (bind_address)
        {
            code[code.size() - 1] = bind_address.value();
        }
        const auto pair = std::pair<
            std::shared_ptr<builder::function>, i64>{
            function, code.size() - 1
        };
        indexes.emplace_back(pair);
    }

    void global_label::bind(const types::stack::i64 & ba)
    {
        for (const auto & [fst, snd] : indexes)
        {
            fst->code[snd] = ba;
        }
        this->bind_address = ba;
    }
}
