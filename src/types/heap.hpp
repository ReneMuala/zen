//
// Created by dte on 9/15/2025.
//

#pragma once
#include "stack.hpp"
namespace zen::types::heap
{
    struct string
    {
        stack::i64 len;
        stack::i64 data;

        ~string()
        {
            if (len and data)
            {
                len = 0;
                data = 0;
                free(reinterpret_cast<void*>(data));
            }
        }

        static string* from_string(const std::string & candidate)
        {
            const auto it  = new string;
            it->len = static_cast<stack::i64>(candidate.length());
            it->data = reinterpret_cast<stack::i64>(strdup(candidate.c_str()));
            return it;
        }

        static string* from_len(const stack::i64 len)
        {
            const auto it  = new string;
            it->len = len;
            it->data = reinterpret_cast<stack::i64>(new char[len+1]{0});
            return it;
        }

        static string* empty()
        {
            return from_len(0);
        }

        static void destroy(const string* it)
        {
            delete it;
        }

        explicit operator std::string() const {
            const auto _ptr = reinterpret_cast<const char*>(data);
            return std::string(_ptr);
        }
    };
}
