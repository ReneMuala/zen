//
// Created by dte on 12/3/2025.
//

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "exceptions/semantic_error.hpp"
#include "types/stack.hpp"

namespace zen::builder
{
    struct type
    {
        enum kind
        {
            stack,
            heap
        } kind;
        operator int() const
        {
            return id;
        }
        const std::string name;
        std::shared_ptr<type> base;
        std::vector<std::pair<std::string, std::shared_ptr<type>>> fields;
        unsigned long long id{};
        [[nodiscard]] std::optional<std::pair<types::stack::i64, std::shared_ptr<type>>> get_field(const std::string & name) const
        {
            types::stack::i64 offset = 0;
            std::string hint;
            for (const auto & [fst, snd] : fields)
            {
                if (fst == name) return {{offset, snd}};
                offset += snd->_size;
            }
            return {};
        }

        bool has_relation(const type & t) const
        {
            if (*this == t)
            {
                return true;
            }

            for (const auto & field : fields)
            {
                if (field.second->has_relation(t))
                {
                    return true;
                }
            }
            return false;
        }


        void add_field(const std::string & name, const std::shared_ptr<type> & type, const int offset)
        {
            if (type->has_relation(*this))
                throw exceptions::semantic_error("infinite-sized type", offset, fmt::format("class '{}' contains itself as a field (directly or indirectly).\n\tuse a wrapper in the cycle to break infinite size", this->name));
            _size += type->get_full_size();
            fields.emplace_back(name, type);
        }
        explicit operator const std::string&() const { return name; }
        explicit type(std::string  name, const types::stack::i64 size, const enum kind kind = stack) : kind(kind), name(std::move(name)), _size(size) {}
        bool operator==(const type& other) const
        {
            return id == other.id;
        }
        [[nodiscard]] types::stack::i64 get_size() const { return kind == stack ? _size : static_cast<types::stack::i64>(sizeof(types::stack::i64)); }
        [[nodiscard]] types::stack::i64 get_full_size() const { return _size; }

        static std::shared_ptr<type> create(const std::string & name,const  types::stack::i64 & size)
        {
            constexpr std::hash<std::string> hasher;
            auto t = std::make_shared<type>(name, size);
            t->id = hasher(name)+size;
            return t;
        }
    private:
        types::stack::i64 _size;
    };
}

