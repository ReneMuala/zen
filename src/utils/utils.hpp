//
// Created by dte on 8/31/2025.
//

#ifndef UTILS_HPP
#define UTILS_HPP
#include <algorithm>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <stdlib.h>

namespace zen::utils
{
    template <typename type = void>
    class raii
    {
        type* data = nullptr;

    public:
        size_t type_hash = typeid(void).hash_code();

        explicit raii(auto value): type_hash(typeid(decltype(value)).hash_code())
        {
            this->data = malloc(sizeof(decltype(value)));
            if (!this->data)
                throw std::bad_alloc();
            *reinterpret_cast<decltype(value)*>(this->data) = value;
        }

        raii(raii const& other) = delete;

        raii(raii&& other) noexcept
        {
            this->data = other.data;
            this->type_hash = other.type_hash;
            other.data = nullptr;
        };

        type* get() const
        {
            return this->data;
        }

        template <typename other>
        [[nodiscard]] constexpr bool is() const
        {
            return type_hash == typeid(other).hash_code();
        }

        template <typename other>
        other as() const
        {
            if (not is<other>())
                throw std::bad_cast();
            return *reinterpret_cast<other*>(this->data);
        }



        template <typename other>
        [[nodiscard]] raii clone() const
        {
            return std::move(raii{
                *static_cast<other*>(this->data)
            });
        }

        ~raii()
        {
            free(this->data);
        }
    };

    struct constant_pool
    {
        std::unordered_map<std::string, raii<>> data;

        template <typename type>
        const raii<>& get(type value)
        {
            std::string key = typeid(type).name();
            if constexpr (std::is_same_v<type, std::string> or std::is_same_v<type, char> or std::is_same_v<type, char*>
                or std::is_same_v<type, const char*>)
                key += value;
            else
                key += std::to_string(value);
            if (!data.contains(key))
                data.emplace(key, std::move(raii<>(value)));
            return data.find(key)->second;
        }
    };
}
#endif //UTILS_HPP
