//
// Created by dte on 9/22/2025.
//

#pragma once
#include "block_scope.hpp"
namespace zen::composer::vm
{
    struct for_scope : public block_scope
    {
        int nested_iterators_count = 0;
    };
}
