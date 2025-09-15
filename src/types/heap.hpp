//
// Created by dte on 9/15/2025.
//

#pragma once
#include "stack.hpp"
namespace zen::types::heap
{
    using string = struct
    {
        stack::i64 len;
        stack::i64 data;
    };
}
