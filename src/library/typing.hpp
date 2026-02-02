//
// Created by dte on 12/16/2025.
//

#pragma once
#include <memory>
#include "builder/library.hpp"

namespace zen::libraries::typing
{
    std::shared_ptr<::zen::builder::library> create(utils::constant_pool & pool);
}
