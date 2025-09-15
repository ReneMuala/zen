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

        static string* make(const std::string & candidate)
        {
            const auto it  = new string;
            it->len = static_cast<stack::i64>(candidate.length());
            it->data = reinterpret_cast<stack::i64>(strdup(candidate.c_str()));
            return it;
        }

        static void destroy(const string* it)
        {
            delete it;
        }

        explicit operator const std::string&() const {
            const auto _ptr = reinterpret_cast<const char*>(data);
            return std::string(_ptr);
        }
    };
}
