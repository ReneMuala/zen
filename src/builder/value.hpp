//
// Created by dte on 12/4/2025.
//

#pragma once
#include <memory>

#include "type.hpp"
#include "types/stack.hpp"


namespace zen::builder
{
    struct value
    {
        enum kind
        {
            temporary,
            constant,
            fake_constant,
            variable
        };

        kind kind;
        types::stack::i64 offset;
        std::shared_ptr<builder::type> type;
        bool no_destructor = false;
        std::string label;
        bool is_reference = false;
        bool is_negated = false;


        [[nodiscard]] bool is(const std::shared_ptr<builder::type>& t) const
        {
            return *type == *t;
        }

        [[nodiscard]] bool has_same_type_as(const value& other) const
        {
            return *type == *other.type;
        }

        [[nodiscard]] types::stack::i64 address(const types::stack::i64& st_point) const
        {
            return kind == kind::constant or kind == kind::fake_constant ? _address : _address - st_point;
        }

        explicit value(const std::shared_ptr<builder::type>& type, const types::stack::i64& address,
                       const enum kind& kind = variable) :
            kind(kind), offset(0), type(type), _address(address)
        {
        }

        explicit value(const std::string& label, const std::shared_ptr<builder::type>& type,
                       const types::stack::i64& address) : value(type, address)
        {
            this->label = label;
        }

        types::stack::i64 _address;
    };
} // builder
// zen
