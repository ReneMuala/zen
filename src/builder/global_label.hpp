//
// Created by dte on 9/22/2025.
//
#pragma once

#include <list>
#include <optional>
#include <stdexcept>
#include <vector>
#include "types/stack.hpp"
#include <memory>

namespace zen::builder
{
   struct function;

   struct global_label
   {
      std::list<std::pair<std::shared_ptr<builder::function>, types::stack::i64>> indexes;
      std::optional<types::stack::i64> bind_address;
   public:
      void use(std::shared_ptr<builder::function> function);
      void bind(const types::stack::i64& ba);
   };
} // _private

