//
// Created by dte on 12/3/2025.
//
#pragma once

#include <memory>
#include <vector>

#include "block.hpp"
#include "signature.hpp"
#include "type.hpp"
#include "value.hpp"
#include "types/stack.hpp"
#include "vm/vm.hpp"

namespace zen::builder
{
    struct function
    {
        static std::shared_ptr<type> _bool();;
        static std::shared_ptr<type> _byte();
        static std::shared_ptr<type> _short();
        static std::shared_ptr<type> _int();
        static std::shared_ptr<type> _long();
        static std::shared_ptr<type> _float();
        static std::shared_ptr<type> _double();
        static std::shared_ptr<type> _string();

        std::vector<types::stack::i64> code;
        std::shared_ptr<zen::builder::signature> signature;
        std::shared_ptr<block> scope;
        types::stack::i64 offset;
        bool logging = false;
        // Factory methods
        std::shared_ptr<block> get_scope();
        static std::shared_ptr<function> create(const bool& logging = false);
        std::shared_ptr<value> set_parameter(const std::shared_ptr<zen::builder::type>& t, const std::string& name);
        std::shared_ptr<value> set_return(const std::shared_ptr<zen::builder::type>& t);
        // Delete copy constructor and assignment
        function(const function&) = delete;
        function& operator=(const function&) = delete;
        [[nodiscard]] std::shared_ptr<value> set_local(const std::shared_ptr<zen::builder::type>& t, const std::string& name, bool param = false);
        std::string address_or_label(const std::shared_ptr<value>& _1, const std::shared_ptr<block> & scp) const
        {
            auto address = _1->address(scope->get_stack_usage());
            return _1->label.empty() ?  fmt::format("{}", address) : fmt::format("{}:{}", _1->label, address);
        }

        template<zen::instruction ins>
        void gen(const std::shared_ptr<value>& _1, const std::shared_ptr<value>& _2, const std::shared_ptr<value>& _3)
        {
            const std::shared_ptr<block> & sc = get_scope();
            code.push_back(ins);
            code.push_back(_1->address(sc->get_stack_usage()));
            code.push_back(_2->address(sc->get_stack_usage()));
            code.push_back(_3->address(sc->get_stack_usage()));

            if (logging)
            {
                fmt::println("<{}> {} {} {} {}", code.size() - 4, code.at(code.size() - 4),
                    address_or_label(_1, sc),
                    address_or_label(_2, sc),
                    address_or_label(_3, sc)
                    );
            }
        }
        template<zen::instruction ins>
        void gen(const std::shared_ptr<value>& _1)
        {
            const std::shared_ptr<block> & sc = get_scope();
            code.push_back(ins);
            code.push_back(_1->address(sc->get_stack_usage()));

            if (logging)
            {
                fmt::println("<{}> {} {}", code.size() - 2, code.at(code.size() - 2), address_or_label(_1, sc));
            }
        }
        template<zen::instruction ins>
        void gen(const i64& _1)
        {
            const std::shared_ptr<block> & sc = get_scope();
            code.push_back(ins);
            code.push_back(_1);

            if (logging)
            {
                fmt::println("<{}> {} {}", code.size() - 2, code.at(code.size() - 2), code.at(code.size() - 1));
            }
        }
        void add(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        /*
        // Integer register creation
        [[nodiscard]] std::shared_ptr<value> i8() const;
        [[nodiscard]] std::shared_ptr<value> i8(char val);

        [[nodiscard]] std::shared_ptr<value> i16() const;
        [[nodiscard]] std::shared_ptr<value> i16(short val);

        [[nodiscard]] std::shared_ptr<value> i32() const;
        [[nodiscard]] std::shared_ptr<value> i32(int val);
        [[nodiscard]] std::shared_ptr<value> i32fromI8or16(const std::shared_ptr<value>& i8or16) const;

        [[nodiscard]] std::shared_ptr<value> i64() const;
        [[nodiscard]] std::shared_ptr<value> i64(long long val);
        [[nodiscard]] std::shared_ptr<value> i64_const(long long val);

        // Floating point register creation
        [[nodiscard]] std::shared_ptr<value> f32() const;
        [[nodiscard]] std::shared_ptr<value> f32(float val);
        [[nodiscard]] std::shared_ptr<value> f32_const(float val);

        [[nodiscard]] std::shared_ptr<value> f64() const;
        [[nodiscard]] std::shared_ptr<value> f64(double val);
        [[nodiscard]] std::shared_ptr<value> f64_const(double val);

        // Label operations
        [[nodiscard]]  std::shared_ptr<label> label() const;
        void bind(const std::shared_ptr<label>& label) const;

        // Argument fetching
        void fetch_argument(size_t index, const std::shared_ptr<value>& r) const;
        void fetch_argument(size_t index, const asmjit::x86::Reg& r, bool f64 = false) const;

        // Arithmetic operations
        void add(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;

        void sub(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;
        void sub(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;

        void mul(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;
        void mul(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;

        void div(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;
        void div(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;

        void mod(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;

        void increment(const std::shared_ptr<value>& r) const;
        void increment(const std::shared_ptr<value>& r);

        void decrement(const std::shared_ptr<value>& r) const;
        void decrement(const std::shared_ptr<value>& r);

        // Return operations
        void return_value(const std::shared_ptr<value>& r) const;
        void return_value(const std::shared_ptr<value>& r) const;
        void return_void() const;

        // Comparison operations
        void compare(const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs) const;
        void compare(const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs, bool possible_equal = false) const;

        // Jump operations
        void jump(const  std::shared_ptr<label>& l) const;
        void jump_equal(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_equal(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_not_equal(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_not_equal(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_lower(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_lower(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_lower_equal(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_lower_equal(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_greater(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_greater(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_greater_equal(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;
        void jump_greater_equal(const std::shared_ptr<value>& t1, const std::shared_ptr<value>& t2, const  std::shared_ptr<label>& l) const;

        // Move operations
        void move(const std::shared_ptr<value>& to, const std::shared_ptr<value>& from, bool is_char = false) const;
        void move(const std::shared_ptr<value>& to, const std::shared_ptr<value>& from, bool is_char = false) const;
        void move(const std::shared_ptr<value>& to, const std::shared_ptr<value>& from) const;
        void move(const std::shared_ptr<value>& to, const std::shared_ptr<value>& from) const;
        void move(const std::shared_ptr<value>& to, const std::shared_ptr<value>& from) const;
        void move(const std::shared_ptr<value>& to, const std::shared_ptr<value>& from) const;

        // Function call
        void call(const asmjit::Imm& func, const asmjit::FuncSignature& signature,
                  const std::vector<asmjit::x86::Reg>& args,
                  const std::optional<asmjit::x86::Reg>& return_to) const;

        // Build function
        template <typename FT>
        FT* build();
        */
        function(){}
    };
}
