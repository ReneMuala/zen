//
// Created by dte on 3/21/2025.
//

#pragma once
#include <vector>
#include <cstdint>
#include <ffi.h>
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <fmt/base.h>

namespace zen
{
    constexpr static auto ascii_art = R"(   .-') _   ('-.       .-') _
  (  OO) )_(  OO)     ( OO ) )
,(_)----.(,------.,--./ ,--,'
|       | |  .---'|   \ |  |\
'--.   /  |  |    |    \|  | )
(_/   /  (|  '--. |  .     |/
 /   /___ |  .--' |  |\    |
|        ||  `---.|  | \   |
`--------'`------'`--'  `--'  )";
    enum instruction
    {
        push,
        pop,
        add_i8,
        sub_i8,
        mul_i8,
        div_i8,
        mod_i8,

        // arith for i64
        add_i64,
        sub_i64,
        mul_i64,
        div_i64,
        mod_i64,

        // relational for i64
        gt_i64,
        lt_i64,
        gte_i64,
        lte_i64,
        eq_i64,
        neq_i64,

        // arith for f64
        add_f64,
        sub_f64,
        mul_f64,
        div_f64,

        // relational for f64
        gt_f64,
        lt_f64,
        gte_f64,
        lte_f64,
        eq_f64,
        neq_f64,

        // logic for boolean
        boolean_and,
        boolean_or,

        // conversions & copies
        i64_to_f64,
        i64_to_i64,
        i64_to_boolean,
        i64_to_i8,

        f64_to_i64,
        f64_to_f64,
        f64_to_boolean,
        f64_to_i8,

        boolean_to_i8,
        boolean_to_i64,
        boolean_to_f64,
        boolean_to_boolean,

        i8_to_i8,
        i8_to_i64,
        i8_to_f64,
        i8_to_boolean,

        // unary for i64
        inc_i64,
        dec_i64,

        // unary for f64
        inc_f64,
        dec_f64,

        bit_and,
        bit_or,
        bit_xor,
        bit_not,
        // unary for boolean
        boolean_not,

        // etc
        hlt,
        // modify stack
        most,
        go,
        go_if,
        jump,
        jump_if,
        call,
        ffi_call,
        ret,
        // fetch_i8,
        // fetch_i64,
        // fetch_f64,
        // fetch_boolean,
        // send_i8,
        // send_i64,
        // send_f64,
        // send_boolean,
        // memory_of,
        placeholder,
    };
    class vm
    {
    public:
        using boolean = bool;
        using i8 = int8_t;
        using i64 = int64_t;
        using f64 = double;

        struct stack
        {
            void * data = nullptr;
            i64 negative_stack_size = 0;
            void * operator-(const i64 & size);
            bool operator-=(const i64 & size);
            bool operator+=(const i64 & size);
            ~stack();
            bool empty() const;
            i64 size() const;
        };

    private:
        std::vector<i64> code;
        std::map<i64, ffi_cif> callables;

        template<typename type>
        static constexpr  void fetch_lhs_rhs(std::stack<type>& stack, type& lhs, type& rhs)
        {
            rhs = stack.top();
            stack.pop();
            lhs = stack.top();
            stack.pop();
        }

        template<typename type>
        static constexpr bool check_stack_for_pop(std::stack<type>& stack){
            if (stack.empty())
            {
                fmt::println(stderr, "runtime error: pop on an empty stack");
                return false;
            }
            return true;
        }

        template<typename type>
        static inline type* address(i64 address, vm::stack& stack)
        {
            return address > 0 ? reinterpret_cast<type*>(address) : static_cast<type*>(stack - -address);
        }

    public:
        bool register_ffi_callable(i64 function_pointer,std::vector<i64> args, i64 return_type)
        {
            ffi_cif & cif = callables[function_pointer];
            return ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.size(), reinterpret_cast<ffi_type*>(return_type), reinterpret_cast<ffi_type**>(args.data())) == FFI_OK;
        }
        static constexpr i64 ref(auto & some)
        {
           return reinterpret_cast<vm::i64>(&some);
        }
        void load(std::vector<i64>& code);
        void run(const i64 & entry_point = 0);
        void run(stack & stack, const i64 & entry_point = 0);
    };


    namespace utils
    {
        template<typename type = void>
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
            raii(raii && other) noexcept
            {
                this->data = other.data;
                this->type_hash = other.type_hash;
                other.data = nullptr;
            };

            type* get() const
            {
                return this->data;
            }

            template<typename other>
            [[nodiscard]] constexpr bool is() const
            {
                return type_hash == typeid(other).hash_code();
            }

            template<typename other>
            other as() const
            {
                if (not is<other>())
                    throw std::bad_cast();
                return *reinterpret_cast<other*>(this->data);
            }
            template<typename other>
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
            template<typename type>
            const raii<> & get(type value)
            {
                std::string key = typeid(type).name();
                if constexpr (std::is_same_v<type, std::string> or std::is_same_v<type, char> or std::is_same_v<type, char *> or std::is_same_v<type, const char *>)
                    key += value;
                else
                    key += std::to_string(value);
                if (!data.contains(key))
                    data.emplace(key, std::move(raii<>(value)));
                return data.find(key)->second;
            }
        };
    }
}
