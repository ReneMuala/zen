//
// Created by dte on 3/21/2025.
//

#include "vm.hpp"

#include <cfloat>
#include <charconv>
#include <iostream>
#include <ostream>
#include <stack>
#include <fmt/core.h>

#define KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(T) \
case add_ ## T: \
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) + *address<T>(this->code[i + 3], stack); \
i+=3;\
break; \
case sub_ ## T:\
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) - *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case mul_ ## T:\
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) * *address<T>(this->code[i + 3], stack);\
            i+=3;\
break;\
case div_ ## T:\
    *address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 3], stack) == 0 ? 0 : *address<T>(this->code[i + 2], stack) / *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case mod_ ## T:\
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) % *address<T>(this->code[i + 3], stack);\
i+=3;\
break;

#define KAIZEN_ARITHMETICS_FOR_FLOAT_TYPE(T) \
case add_ ## T: \
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) + *address<T>(this->code[i + 3], stack); \
i+=3;\
break; \
case sub_ ## T:\
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) - *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case mul_ ## T:\
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) * *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case div_ ## T:\
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 3], stack) == 0 ? 0 : *address<T>(this->code[i + 2], stack) / *address<T>(this->code[i + 3], stack);\
i+=3;\
break;

#define KAIZEN_RELATIONAL_FOR_TYPE(T) \
case gt_ ## T:\
*address<boolean>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) > *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case lt_ ## T:\
*address<boolean>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) < *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case gte_ ## T:\
*address<boolean>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) >= *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case lte_ ## T:\
*address<boolean>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) <= *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case eq_ ## T:\
*address<boolean>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) == *address<T>(this->code[i + 3], stack);\
i+=3;\
break;\
case neq_ ## T:\
*address<boolean>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) != *address<T>(this->code[i + 3], stack);\
i+=3;\
break;

#define KAIZEN_CONVERSION_FOR_TYPE(T) \
case ( T ## _to_i64):\
*address<i64>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack);\
i += 2;\
break;\
case (T ## _to_f64):\
*address<f64>(this->code[i + 1], stack) = static_cast<f64>(*address<T>(this->code[i + 2], stack));\
i += 2;\
break;\
case (T ## _to_i8):\
*address<i8>(this->code[i + 1], stack) = static_cast<i8>(*address<T>(this->code[i + 2], stack));\
i += 2;\
break;\
case (T ## _to_i32):\
*address<i32>(this->code[i + 1], stack) = static_cast<i32>(*address<T>(this->code[i + 2], stack));\
i += 2;\
break;\
case (T ## _to_i16):\
*address<i16>(this->code[i + 1], stack) = static_cast<i16>(*address<T>(this->code[i + 2], stack));\
i += 2;\
break;\
case (T ## _to_f32):\
*address<f32>(this->code[i + 1], stack) = static_cast<f32>(*address<T>(this->code[i + 2], stack));\
i += 2;\
break;\
case (T ## _to_boolean):\
*address<boolean>(this->code[i + 1], stack) = static_cast<boolean>(*address<T>(this->code[i + 2], stack));\
i += 2;\
break;
/*
#define KAIZEN_CONVERSION_FROM_T_to_str(T)\
case T##_to_str:\
    {\
        constexpr size_t size = 3 + DBL_MANT_DIG - DBL_MIN_EXP;\
        char buffer[size]{};\
        auto result = std::to_chars((char*)buffer, (char*)(buffer + size), value, std::chars_format::fixed);\
        auto _str = std::string(buffer);\
        free((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8));\
        *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = reinterpret_cast<i64>(strdup(_str.c_str()));\
        *(i64*)(*address<i64>(this->code[i + 1], stack)) = _str.length();\
    }\
i+=2;\
break;
 */

#define KAIZEN_CONVERSION_FROM_INTEGER_TO_STRING(T)\
case T##_to_str:\
    {\
        auto _str = std::to_string(*address<T>(this->code[i + 2], stack));\
        free((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8));\
        *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = reinterpret_cast<i64>(strdup(_str.c_str()));\
        *(i64*)(*address<i64>(this->code[i + 1], stack)) = _str.length();\
    }\
i+=2;\
break;

#define KAIZEN_STRING_CONVERSION_FOR_INTEGER_TYPE(T,A) \
KAIZEN_CONVERSION_FROM_INTEGER_TO_STRING(T)\
case str_to_##T:\
    *address<i64>(this->code[i + 1], stack) = A((char*)*(i64*)(*address<i64>(this->code[i + 2], stack) + 8), nullptr, 10);\
    i+=2;\
break;

#define KAIZEN_STACK_PUSH_FOR_TYPE(T) \
case push_ ## T:\
    stack -= sizeof(T); \
    *static_cast<T*>(stack - sizeof(T)) = *address<T>(this->code[i + 1] - (this->code[i + 1] < 0 ? sizeof(T) : 0), stack); \
    i += 1; \
break;

#define KAIZEN_IO_WRITE_FOR_SCALAR_TYPE(T) \
case write_ ## T: \
fwrite(address<T>(this->code[i + 1], stack), sizeof(T), 1, reinterpret_cast<FILE*>(*address<i64>(this->code[i + 2], stack))); \
i += 2;\
break;

#define KAIZEN_IO_READ_FOR_SCALAR_TYPE(T) \
case read_ ## T: \
fread(address<T>(this->code[i + 1], stack), sizeof(T), 1, reinterpret_cast<FILE*>(*address<i64>(this->code[i + 2], stack)));\
i += 2;\
break;

void* zen::vm::stack::operator-(const i64& size)
{
    if (size > -this->negative_stack_size)
    {
        throw std::runtime_error(fmt::format("zen::vm::stack address out of range: ({} of {})", size,
                                             -this->negative_stack_size));
    }
    return reinterpret_cast<void*>(reinterpret_cast<i64>(data) + abs(this->negative_stack_size) - size);
}

bool zen::vm::stack::operator-=(const i64& size)
{
    negative_stack_size -= size;
    if (negative_stack_size <= 0)
        return (data = realloc(data, std::abs(negative_stack_size)));
    throw std::runtime_error(fmt::format("zen::vm::stack invalid resize: {}", size));
    negative_stack_size -= std::abs(size);
    return false;
}

bool zen::vm::stack::operator+=(const i64& size)
{
    negative_stack_size += size;
    if (negative_stack_size <= 0)
        return (data = realloc(data, std::abs(negative_stack_size)));
    throw std::runtime_error(fmt::format("zen::vm::stack stack overflow: {}", size));
    negative_stack_size -= std::abs(size);
    return false;
}

zen::vm::stack::~stack()
{
    if (negative_stack_size != 0)
        free(data);
}

bool zen::vm::stack::empty() const
{
    return negative_stack_size == 0;
}

zen::i64 zen::vm::stack::size() const
{
    return negative_stack_size < 0 ? -negative_stack_size : negative_stack_size;
}

void zen::vm::load(std::vector<i64>& code)
{
    this->code = std::move(code);
}

void zen::vm::run(const i64& entry_point)
{
    stack stack;
    run(stack, entry_point);
}

void zen::vm::run(stack& stack, const i64& entry_point)
{
    std::vector<i64> stack_usage_deque;
    i64 i = 0;
    i64 last_stack_usage = 0;
    try
    {
        for (i = entry_point; i < this->code.size(); i++)
        {
            if (false)
            {
                bool display = true;
                i64 stack_usage_difference = -stack.negative_stack_size - last_stack_usage;
                if (stack_usage_difference && display)
                    fmt::println("sud = {}", stack_usage_difference);

                if (stack_usage_difference > 0)
                    stack_usage_deque.push_back(stack_usage_difference);
                else
                {
                    while (stack_usage_difference < 0 and not stack_usage_deque.empty())
                    {
                        stack_usage_difference += stack_usage_deque.back();
                        stack_usage_deque.pop_back();
                    }
                }
                last_stack_usage = -stack.negative_stack_size;
                auto stack_usage = last_stack_usage;
                if (display)
                {
                    for (const auto& sti : stack_usage_deque)
                    {
                        fmt::print("\tstack[{}] ", stack_usage);
                        switch (sti)
                        {
                        case 1:
                            fmt::println("{}", *(i8*)(stack - stack_usage));
                            break;
                        case 2:
                            fmt::println("{}", *(i16*)(stack - stack_usage));
                            break;
                        case 4:
                            fmt::println("{}", *(i32*)(stack - stack_usage));
                            break;
                        case 8:
                            fmt::println("{}", *(i64*)(stack - stack_usage));
                            break;
                        default: break;
                        }
                        stack_usage -= sti;
                    }
                    fmt::print("{} ({})\n", i, code[i]);
                    if (false)
                    {
                        std::string line;
                        std::getline(std::cin, line);
                    }
                }
            }
            switch (this->code[i])
            {
            KAIZEN_STACK_PUSH_FOR_TYPE(i8)
            KAIZEN_STACK_PUSH_FOR_TYPE(i16)
            KAIZEN_STACK_PUSH_FOR_TYPE(i32)
            KAIZEN_STACK_PUSH_FOR_TYPE(i64)
            KAIZEN_STACK_PUSH_FOR_TYPE(f32)
            KAIZEN_STACK_PUSH_FOR_TYPE(f64)
            KAIZEN_STACK_PUSH_FOR_TYPE(boolean)

            KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(i8)
            KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(i16)
            KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(i32)
            KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(i64)
            KAIZEN_ARITHMETICS_FOR_FLOAT_TYPE(f32)
            KAIZEN_ARITHMETICS_FOR_FLOAT_TYPE(f64)

            KAIZEN_RELATIONAL_FOR_TYPE(i8)
            KAIZEN_RELATIONAL_FOR_TYPE(i16)
            KAIZEN_RELATIONAL_FOR_TYPE(i32)
            KAIZEN_RELATIONAL_FOR_TYPE(i64)
            KAIZEN_RELATIONAL_FOR_TYPE(f32)
            KAIZEN_RELATIONAL_FOR_TYPE(f64)

            KAIZEN_CONVERSION_FOR_TYPE(i8)
            KAIZEN_CONVERSION_FOR_TYPE(i16)
            KAIZEN_CONVERSION_FOR_TYPE(i32)
            KAIZEN_CONVERSION_FOR_TYPE(i64)
            KAIZEN_CONVERSION_FOR_TYPE(f32)
            KAIZEN_CONVERSION_FOR_TYPE(f64)
                // KAIZEN_CONVERSION_FOR_TYPE(boolean)
            KAIZEN_IO_WRITE_FOR_SCALAR_TYPE(i8)
            KAIZEN_IO_WRITE_FOR_SCALAR_TYPE(i16)
            KAIZEN_IO_WRITE_FOR_SCALAR_TYPE(i32)
            KAIZEN_IO_WRITE_FOR_SCALAR_TYPE(i64)
            KAIZEN_IO_WRITE_FOR_SCALAR_TYPE(f32)
            KAIZEN_IO_WRITE_FOR_SCALAR_TYPE(f64)
            KAIZEN_IO_WRITE_FOR_SCALAR_TYPE(boolean)
            case add_str:
                {
                    const i64 new_length = *(i64*)(*address<i64>(this->code[i + 2], stack)) + *(i64*)(*address<i64>(this->code[i + 3], stack));
                    char * new_str = (char*)malloc(new_length);
                    free((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8));
                    strncpy(new_str, (char*)*(i64*)(*address<i64>(this->code[i + 2], stack) + 8), *(i64*)(*address<i64>(this->code[i + 2], stack)));
                    strncpy(new_str + *(i64*)(*address<i64>(this->code[i + 2], stack)), (char*)*(i64*)(*address<i64>(this->code[i + 3], stack) + 8), *(i64*)(*address<i64>(this->code[i + 3], stack)));
                    *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = (i64)new_str;
                    *(i64*)(*address<i64>(this->code[i + 1], stack)) = new_length;
                }
                i+=3;
                break;
            case eq_str:
                {
                    const auto size1 =  *(i64*)*address<i64>(this->code[i + 2], stack);
                    const auto size2 = *(i64*)*address<i64>(this->code[i + 3], stack);
                    *address<boolean>(this->code[i + 1], stack) = (size1 == size2) and std::strncmp((char*)*(i64*)(*address<i64>(this->code[i + 2], stack) + 8), (char*)*(i64*)(*address<i64>(this->code[i + 3], stack) + 8), size1) == 0;
                }
               i += 3;
                break;
            case i8_to_str:
                {
                    auto _str = std::string{*address<i8>(this->code[i + 2], stack)};
                    free((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8));
                    *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = reinterpret_cast<i64>(strdup(_str.c_str()));
                    *(i64*)(*address<i64>(this->code[i + 1], stack)) = _str.length();
                }
                i += 2;
                break;
            case str_to_i8: *address<i64>(this->code[i + 1], stack) = strtol(
                    (char*)*(i64*)(*address<i64>(this->code[i + 2], stack) + 8), nullptr, 10);
                i += 2;
                break;
            case i16_to_str: { auto _str = std::to_string(*address<i16>(this->code[i + 2], stack)); free((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8)); *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = reinterpret_cast<i64>(strdup(_str.c_str())); *(i64*)(*address<i64>(this->code[i + 1], stack)) = _str.length(); } i+=2; break; case str_to_i16: *address<i64>(this->code[i + 1], stack) = strtol((char*)*(i64*)(*address<i64>(this->code[i + 2], stack) + 8), nullptr, 10);                i += 2;
                break;
            KAIZEN_STRING_CONVERSION_FOR_INTEGER_TYPE(i32, strtol)
            KAIZEN_STRING_CONVERSION_FOR_INTEGER_TYPE(i64, strtoll)
            case f64_to_str:
                {
                    constexpr size_t size = 3 + DBL_MANT_DIG - DBL_MIN_EXP;
                    char buffer[size]{};
                    auto result = std::to_chars((char*)buffer, (char*)(buffer + size),
                                                *address<f64>(this->code[i + 2], stack), std::chars_format::general);
                    auto _str = std::string(buffer);
                    free((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8));
                    *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = reinterpret_cast<i64>(strdup(_str.c_str()));
                    *(i64*)(*address<i64>(this->code[i + 1], stack)) = _str.length();
                }
                i += 2;
                break;
            case str_to_f64:
                *address<f64>(this->code[i + 1], stack) = std::strtod(
                    (char*)*(i64*)(*address<i64>(this->code[i + 2], stack) + 8), nullptr);
                i += 2;
                break;
            case f32_to_str:
                {
                    constexpr size_t size = 3 + FLT_MANT_DIG - FLT_MIN_EXP;
                    char buffer[size]{};
                    auto result = std::to_chars((char*)buffer, (char*)(buffer + size),
                                                *address<f32>(this->code[i + 2], stack), std::chars_format::general);
                    auto _str = std::string(buffer);
                    free((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8));
                    *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = reinterpret_cast<i64>(strdup(_str.c_str()));
                    *(i64*)(*address<i64>(this->code[i + 1], stack)) = _str.length();
                }
                i += 2;
                break;
            case str_to_f32:
                // std::cout << *address<i64>(this->code[i + 1], stack) << " " << *address<i64>(this->code[i + 2], stack) << std::endl;
                *address<f32>(this->code[i + 1], stack) = std::strtof(
                    (char*)*(i64*)(*address<i64>(this->code[i + 2], stack) + 8), nullptr);
                i += 2;
                break;
            case write_str:
                fwrite((char*)(*address<i64>(this->code[i + 1], stack)), 1,
                       // std::min(*address<i64>(this->code[i + 2], stack), *(i64*)(*address<i64>(this->code[i + 1], stack))),
                       *address<i64>(this->code[i + 2], stack),
                       (FILE*)*address<i64>(this->code[i + 3], stack));
                i += 3;
                break;
            case read_str: // ok, this looks messy, but let me explain because its actually simple and safe ;)
                // before reading, resize the destination to the expected size (if necessary).
                if (*(i64*)(*address<i64>(this->code[i + 1], stack)) != *address<i64>(this->code[i + 2], stack))
                {
                    *(i64*)*address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack);
                    *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = (i64)realloc(
                        (char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8),
                        *(i64*)*address<i64>(this->code[i + 1], stack));
                }
                // read and partially resize destination (updating its size member only)
                *(i64*)*address<i64>(this->code[i + 1], stack) = (i64)fread(
                    (char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8), 1,
                    *(i64*)*address<i64>(this->code[i + 1], stack), (FILE*)*address<i64>(this->code[i + 3], stack));
                // complete destination resize operation leaving one extra byte for retro compatibility with c strings
                *(i64*)(*address<i64>(this->code[i + 1], stack) + 8) = (i64)realloc(
                    (char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8),
                    *(i64*)*address<i64>(this->code[i + 1], stack) + 1);
                // zero that extra byte
                ((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8))[*(i64*)*address<
                    i64>(this->code[i + 1], stack)] = 0;
                i += 3;
                break;
            KAIZEN_IO_READ_FOR_SCALAR_TYPE(i8)
            KAIZEN_IO_READ_FOR_SCALAR_TYPE(i16)
            KAIZEN_IO_READ_FOR_SCALAR_TYPE(i32)
            KAIZEN_IO_READ_FOR_SCALAR_TYPE(i64)
            KAIZEN_IO_READ_FOR_SCALAR_TYPE(f32)
            KAIZEN_IO_READ_FOR_SCALAR_TYPE(f64)
            KAIZEN_IO_READ_FOR_SCALAR_TYPE(boolean)

            case boolean_and:
                *address<boolean>(this->code[i + 1], stack) = *address<boolean>(this->code[i + 2], stack) and *address<
                    boolean>(this->code[i + 3], stack);
                i += 3;
                break;
            case boolean_or:
                *address<boolean>(this->code[i + 1], stack) = *address<boolean>(this->code[i + 2], stack) or *address<
                    boolean>(this->code[i + 3], stack);
                i += 3;
                break;
            case boolean_not:
                *address<boolean>(this->code[i + 1], stack) = not*address<boolean>(this->code[i + 2], stack);
                i += 2;
                break;
            case most:
                stack += this->code[i + 1]; // NOLINT
                i += 1;
                break;
            case go:
                i += this->code[i + 1] >= 0 ? this->code[i + 1] + 1 : this->code[i + 1];
                break;
            case go_if_not:
                if (not*address<boolean>(this->code[i + 1], stack))
                    i += this->code[i + 2] >= 0 ? this->code[i + 2] + 2 : this->code[i + 2];
                else
                    i += 2;
                break;
            case call:
                stack -= sizeof(i64); // NOLINT
                *static_cast<i64*>(stack - sizeof(i64)) = i + 2;
                i = this->code[i + 1] - 1;
                break;
            case ret:
                i = *static_cast<i64*>(stack - sizeof(i64)) - 1;
                stack += sizeof(i64); // NOLINT
                break;
            case allocate:
                *address<i64>(this->code[i + 1], stack) = reinterpret_cast<i64>(malloc(
                    *address<i64>(this->code[i + 2], stack)));
                if (not*address<i64>(this->code[i + 1], stack))
                {
                    throw std::runtime_error(fmt::format("fatal error: out of heap memory (zen vm halted at {}) should allocate {} byte(s)", i, *address<i64>(this->code[i + 2], stack)));
                }
                std::memset((void*)*address<i64>(this->code[i + 1], stack), 0, *address<i64>(this->code[i + 2], stack));
                i += 2;
                break;
            case deallocate:
                free(reinterpret_cast<void*>(*address<i64>(this->code[i + 1], stack)));
                i += 1;
                break;
            case reallocate:
                *address<i64>(this->code[i + 1], stack) = reinterpret_cast<i64>(realloc(
                    reinterpret_cast<void*>(*address<i64>(this->code[i + 1], stack)),
                    *address<i64>(this->code[i + 2], stack)));
                if (not*address<i64>(this->code[i + 1], stack))
                {
                    throw std::runtime_error(fmt::format("fatal error: out of heap memory (zen vm halted at {})", i));
                }
                i += 2;
                break;
            case copy:
                // fmt::println("*({}){} = {}", *address<i64>(this->code[i + 3], stack), *address<i64>(this->code[i + 1], stack), *address<i64>(this->code[i + 2], stack));
                // fmt::print("copy({},{},{}) ", *address<i64>(this->code[i + 1], stack), *address<i64>(this->code[i + 2], stack),*address<i64>(this->code[i + 3], stack));
                // fmt::println("[i]: {}", i);
                memcpy(reinterpret_cast<void*>(*address<i64>(this->code[i + 1], stack)),
                       reinterpret_cast<void*>(*address<i64>(this->code[i + 2], stack)),
                       *address<i64>(this->code[i + 3], stack));
                i += 3;
                break;
            case refer:
                *address<i64>(this->code[i + 1], stack) = (i64)address<i64>(this->code[i + 2], stack);
                i += 2;
                break;
            case placeholder:
                break;
            default:
                fmt::println(stderr, "fatal error: unsupported operation {} (zen vm halted at {})", this->code[i], i);
            case hlt:
                return;
            }
        }
    }
    catch (std::exception& e)
    {
        fmt::print(stderr, "fatal error: {}\nvm halted at {}", e.what(), i);
    }
}


