//
// Created by dte on 12/3/2025.
//

#include "generic_context.hpp"

namespace zen
{
    namespace builder
    {
        types::stack::i64 generic_context::hash() const
        {
            static constexpr std::hash<std::string> hasher;
            return hasher(name+std::to_string(params.size()));
        }
    } // builder
} // zen