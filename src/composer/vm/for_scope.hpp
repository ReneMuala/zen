//
// Created by dte on 9/22/2025.
//

#pragma once
#include <optional>

#include "scope.hpp"
#include "block_scope.hpp"
#include "label.hpp"
#include "composer/composer.hpp"
namespace zen::composer::vm
{
    struct for_scope : public block_scope
    {
        int nested_iterators_count = 0;
    };
}
