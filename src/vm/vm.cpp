//
// Created by dte on 3/21/2025.
//

#include "vm.hpp"

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
    *address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) / *address<T>(this->code[i + 3], stack);\
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
*address<T>(this->code[i + 1], stack) = *address<T>(this->code[i + 2], stack) / *address<T>(this->code[i + 3], stack);\
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

#define KAIZEN_STACK_PUSH_FOR_TYPE(T) \
case push_ ## T:\
stack -= sizeof(T); \
*static_cast<T*>(stack - sizeof(T)) = *address<T>(this->code[i + 1], stack); \
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
        fmt::println(stderr, "zen::vm::stack address out of range: {}", size);
        return nullptr;
    }
    return reinterpret_cast<void*>(reinterpret_cast<i64>(data) + abs(this->negative_stack_size) - size);
}

bool zen::vm::stack::operator-=(const i64& size)
{
    negative_stack_size -= size;
    if (negative_stack_size <= 0)
        return (data = realloc(data, std::abs(negative_stack_size)));
    fmt::println(stderr, "zen::vm::stack invalid resize: {}", size);
    negative_stack_size -= std::abs(size);
    return false;
}

bool zen::vm::stack::operator+=(const i64& size)
{
    negative_stack_size += size;
    if (negative_stack_size <= 0)
        return (data = realloc(data, std::abs(negative_stack_size)));
    fmt::println(stderr, "zen::vm::stack stack overflow: {}", size);
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
    for (auto i = entry_point; i < this->code.size(); i++)
    {
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
        case write_str:
            fwrite((char*)*(i64*)(*address<i64>(this->code[i + 1], stack) + 8), 1,
                   std::min(*address<i64>(this->code[i + 2], stack), *(i64*)(*address<i64>(this->code[i + 1], stack))),
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
            i += this->code[i + 1] > 0 ? this->code[i + 1] + 1 : this->code[i + 1];
            break;
        case go_if_not:
            if (not *address<boolean>(this->code[i + 1], stack))
                i += this->code[i + 2] > 0 ? this->code[i + 2] + 2 : this->code[i + 2];
            else
                i += 2;
            break;
        case call:
            stack -= sizeof(i64); // NOLINT
            *static_cast<i64*>(stack - sizeof(i64)) = i + 2;
            // i = *address<i64>(this->code[i + 1], stack) - 1;
            i = *static_cast<i64*>(stack - ((this->code[i + 1] + 2) * sizeof(i64))) - 1;
            break;
        case ret:
            i = *static_cast<i64*>(stack - sizeof(i64)) - 1;
            stack += sizeof(i64); // NOLINT
            break;
        case allocate:
            *address<i64>(this->code[i + 1], stack) = reinterpret_cast<i64>(malloc(
                *address<i64>(this->code[i + 2], stack)));
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
            i += 2;
            break;
        case copy:
            memcpy(reinterpret_cast<void*>(*address<i64>(this->code[i + 1], stack)),
                   reinterpret_cast<void*>(*address<i64>(this->code[i + 2], stack)),
                   *address<i64>(this->code[i + 3], stack));
            i += 3;
            break;
        case walk:
            *address<i64>(this->code[i + 1], stack) = reinterpret_cast<i64>(reinterpret_cast<char*>(*address<i64>(
                    this->code[i + 2], stack))) +
                static_cast<i64>(sizeof(this->code[i + 3]));
            i += 3;
            break;
        default:
            fmt::println(stderr, "fatal error: unsupported operation {} (zen vm halted at {})", this->code[i], i);
        case hlt:
            return;
        }
    }
}


