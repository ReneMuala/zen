//
// Created by dte on 12/16/2025.
//

#pragma once
#include <memory>
#include "builder/library.hpp"
#include "builder/program.hpp"

namespace zen::libraries::test
{
    std::shared_ptr<zen::builder::library> create(utils::constant_pool & pool, std::shared_ptr<zen::builder::library> lib, std::shared_ptr<zen::builder::program> program);
}
