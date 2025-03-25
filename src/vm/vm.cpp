//
// Created by dte on 3/21/2025.
//

#include "vm.hpp"

#include <iostream>
#include <ostream>
#include <stack>
#include <fmt/core.h>


void* zen::vm::stack::operator-(const i64& size)
{
    if (size > -this->negative_stack_size)
    {
        fmt::println(stderr, "zen::vm::stack address out of range: {}", size);
        return nullptr;
    }
    return reinterpret_cast<void*>(reinterpret_cast<vm::i64>(data) + abs(this->negative_stack_size) - size);
}

bool zen::vm::stack::operator-=(const i64& size)
{
    negative_stack_size -= size;
    if (negative_stack_size <= 0)
        return (data = realloc(data, abs(negative_stack_size)));
    fmt::println(stderr, "zen::vm::stack invalid resize: {}", size);
    negative_stack_size -= abs(size);
    return false;
}

bool zen::vm::stack::operator+=(const i64& size)
{
    negative_stack_size += size;
    if (negative_stack_size <= 0)
        return (data = realloc(data, abs(negative_stack_size)));
    fmt::println(stderr, "zen::vm::stack stack overflow: {}", size);
    negative_stack_size -= abs(size);
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

zen::vm::i64 zen::vm::stack::size() const
{
    return abs(negative_stack_size);
}

void zen::vm::load(std::vector<i64>& code)
{
    this->code = std::move(code);
}

void zen::vm::run(const i64 & entry_point)
{
    stack stack;
    run(stack, entry_point);
}

void zen::vm::run(stack & stack, const i64 & entry_point)
{
    for (auto i = entry_point; i < this->code.size(); i++)
    {
        switch (this->code[i])
        {
        case push:
            stack -= sizeof(i64); // NOLINT
            *static_cast<i64*>(stack-sizeof(i64)) = *address<i64>(this->code[i + 1], stack);
            i+=1;
            break;
        case pop:
            *address<i64>(this->code[i + 1], stack) = *static_cast<i64*>(stack-sizeof(i64));
            stack += sizeof(i64);  // NOLINT
            i+=1;
            break;
        case add_i64:
            *address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) + *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case sub_i64:
            *address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) - *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case mul_i64:
            *address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) * *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case div_i64:
            *address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) / *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case mod_i64:
            *address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) % *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case gt_i64:
            *address<boolean>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) > *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case lt_i64:
            *address<boolean>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) < *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case gte_i64:
            *address<boolean>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) >= *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case lte_i64:
            *address<boolean>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) <= *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case eq_i64:
            *address<boolean>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) == *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case neq_i64:
            *address<boolean>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) != *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;
        case add_f64:
            *address<f64>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) + *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case sub_f64:
            *address<f64>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) - *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case mul_f64:
            *address<f64>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) * *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case div_f64:
            *address<f64>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) / *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case gt_f64:
            *address<boolean>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) > *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case lt_f64:
            *address<boolean>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) < *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case gte_f64:
            *address<boolean>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) >= *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case lte_f64:
            *address<boolean>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) <= *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case eq_f64:
            *address<boolean>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) == *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case neq_f64:
            *address<boolean>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack) != *address<f64>(this->code[i + 3], stack);
            i+=3;
            break;
        case boolean_and:
            *address<boolean>(this->code[i + 1], stack) = *address<boolean>(this->code[i + 2], stack) and *address<boolean>(this->code[i + 3], stack);
            i+=3;
            break;
        case boolean_or:
            *address<boolean>(this->code[i + 1], stack) = *address<boolean>(this->code[i + 2], stack) or *address<boolean>(this->code[i + 3], stack);
            i+=3;
            break;
        case i64_to_f64:
            *address<f64>(this->code[i + 1], stack) =  static_cast<f64>(*address<i64>(this->code[i + 2], stack));
            i+=2;
            break;
        case i64_to_i64:
            *address<i64>(this->code[i + 1], stack) =  *address<i64>(this->code[i + 2], stack);
            i+=2;
            break;
        case f64_to_i64:
            *address<i64>(this->code[i + 1], stack) = static_cast<i64>(*address<f64>(this->code[i + 2], stack));
            i+=2;
            break;
        case f64_to_f64:
            *address<f64>(this->code[i + 1], stack) = *address<f64>(this->code[i + 2], stack);
            i+=2;
            break;
        case boolean_to_i64:
            *address<i64>(this->code[i + 1], stack) = *address<boolean>(this->code[i + 2], stack);
            i+=2;
            break;
        case i64_to_boolean:
            *address<boolean>(this->code[i + 1], stack) = static_cast<boolean>(*address<i64>(this->code[i + 2], stack));
            i+=2;
            break;
        case boolean_to_f64:
            *address<f64>(this->code[i + 1], stack) = *address<boolean>(this->code[i + 2], stack);
            i+=2;
            break;
        case f64_to_boolean:
            *address<boolean>(this->code[i + 1], stack) = static_cast<boolean>(*address<f64>(this->code[i + 2], stack));
            i+=2;
            break;
        case inc_i64:
            (*(address<i64>(this->code[i + 1], stack)))++;
            i+=1;
            break;
        case dec_i64:
            (*(address<i64>(this->code[i + 1], stack)))--;
            i+=1;
            break;
        case inc_f64:
            (*address<f64>(this->code[i + 1], stack))++;
            i+=1;
            break;
        case dec_f64:
            (*address<f64>(this->code[i + 1], stack))--;
            i+=1;
            break;

        case bit_and:
            *address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) & *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;

        case bit_or:
            *address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) | *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;

        case bit_xor:
            *address<i64>(this->code[i + 1], stack) = *address<i64>(this->code[i + 2], stack) xor *address<i64>(this->code[i + 3], stack);
            i+=3;
            break;

        case bit_not:
            *address<i64>(this->code[i + 1], stack) = ~*address<i64>(this->code[i + 1], stack);
            i+=1;
            break;


        case boolean_not:
            *address<boolean>(this->code[i + 1], stack) = not *address<boolean>(this->code[i + 2], stack);
            i+=2;
            break;
        case most:
            stack+=this->code[i + 1]; // NOLINT
            i+=1;
            break;
        case jump:
            i+=this->code[i + 1];
            break;
        case jump_if:
            if (*address<boolean>(this->code[i + 1], stack))
                i+=this->code[i + 2];
            else
                i+=2;
            break;
        default:
            fmt::println(stderr, "fatal error: unsupported operation {} (zen vm halted at {})", this->code[i], i);
        case hlt:
            return;
        }
    }
    if (not stack.empty())
    {
        fmt::println(stderr, "fatal error: stack not cleared, {} items remaining", stack.size());
    }
}


