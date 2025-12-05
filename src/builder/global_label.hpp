//
// Created by dte on 9/22/2025.
//
#pragma once

#include <list>
#include <optional>
#include <stdexcept>
#include <vector>
#include "types/stack.hpp"

namespace zen::builder {
class global_label {
   std::list<types::stack::i64> indexes;
   std::optional<types::stack::i64> bind_address;
   public:
   void use(std::vector<types::stack::i64> & code)
   {
      if (code.empty())
         throw std::invalid_argument("cannot bind label in empty code");
      if (bind_address)
         code[code.size() - 1] = bind_address.value();
      else
         indexes.emplace_back(code.size() - 1);
   }

   void bind(std::vector<types::stack::i64> & code)
   {
      for (const auto & index : indexes)
      {
         code[index] = static_cast<types::stack::i64>(code.size());
      }
      bind_address = code.size();
   }

   void bind(std::vector<types::stack::i64> & code, const types::stack::i64 & bind_address)
   {
      for (const auto & index : indexes)
      {
         code[index] = bind_address;
      }
      this->bind_address = bind_address;
   }
};

} // _private

