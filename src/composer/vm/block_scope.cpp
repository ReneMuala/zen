//
// Created by dte on 9/25/2025.
//
#include "block_scope.hpp"
#include "for_scope.hpp"
namespace zen::composer::vm
{
    block_scope* block_scope::__unsafely_make(const enum type type)
{
    const auto it = type == in_for ? new for_scope : new block_scope;
    it->type = type;
    return it;
}
}