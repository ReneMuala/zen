//
// Created by dte on 9/22/2025.
//
#pragma once

#include <list>
#include <vector>
#include "types/stack.hpp"

namespace zen::composer::vm {
class label {
   std::list<i64> indexes;
   std::optional<i64> bind_address;
   public:
   void use(std::vector<i64> & code)
   {
      if (code.empty())
         throw std::invalid_argument("cannot bind label in empty code");
      if (bind_address)
         code[code.size() - 1] = bind_address.value() - code.size() + 1;
      else
         indexes.emplace_back(code.size() - 1);
   }

   void bind(std::vector<i64> & code)
   {
      for (const auto & index : indexes)
      {
         code[index] = static_cast<i64>(code.size()) - index - 1;
      }
      bind_address = code.size();
   }
};

} // _private

