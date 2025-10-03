//
// Created by dte on 10/3/2025.
//

#pragma once
#include "label.hpp"
#include "composer/composer.hpp"
namespace zen::composer::vm
{
    struct function
    {
        signature signature;
        i64 address;
        bool defined = true;
        std::vector<label> labels;
    };
}