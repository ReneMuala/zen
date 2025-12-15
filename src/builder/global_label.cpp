//
// Created by dte on 12/12/2025.
//
#include "global_label.hpp"
#include "function.hpp"

namespace zen::builder
{
    std::shared_ptr<global_label> global_label::create(const std::string& name)
    {
        static types::stack::i64 id = 0;
        return std::make_shared<global_label>(name);
    }

    void global_label::use(const std::vector<types::stack::i64>& code)
    {
        if (code.empty())
            throw std::invalid_argument("cannot bind label in empty code");
        indexes.emplace_back(code.size() - 1);
    }

    void global_label::bind(std::vector<types::stack::i64>& code, const types::stack::i64 offset,
        const types::stack::i64 bind_address) const
    {
        for (const auto & index : indexes)
        {
            code[offset + index] = bind_address;
        }
    }
}
