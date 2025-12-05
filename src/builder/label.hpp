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
struct label {
   std::list<types::stack::i64> indexes;
   std::optional<types::stack::i64> bind_address;
   void use(std::vector<types::stack::i64> & code)
   {
      if (code.empty())
         throw std::invalid_argument("cannot bind label in empty code");
      if (bind_address)
         code[code.size() - 1] = bind_address.value() - code.size() + 1;
      else
         indexes.emplace_back(code.size() - 1);
   }

   void bind(std::vector<types::stack::i64> & code)
   {
      for (const auto & index : indexes)
      {
         code[index] = static_cast<types::stack::i64>(code.size()) - index - 1;
      }
      bind_address = code.size();
   }
};

} // _private

