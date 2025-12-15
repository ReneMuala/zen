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

   struct global_label // FIX THIS TO SET THE BIND ADDRESS TO OFFSET + INDEX IN GLOBAL PROGRAM
   {
      std::string name;
      std::list<types::stack::i64> indexes;

      static std::shared_ptr<global_label> create(const std::string & name);

      void use(const std::vector<types::stack::i64> & code);

      void bind(std::vector<types::stack::i64> & code, const types::stack::i64 offset, const types::stack::i64 bind_address) const;
   };
} // _private

