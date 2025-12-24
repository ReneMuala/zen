//
// Created by dte on 12/3/2025.
//
#pragma once

#include <expected>
#include <functional>
#include <memory>
#include <regex>
#include <vector>

#include "block.hpp"
#include "global_label.hpp"
#include "label.hpp"
#include "signature.hpp"
#include "type.hpp"
#include "value.hpp"
#include "types/stack.hpp"
#include "utils/utils.hpp"
#include "vm/vm.hpp"

namespace zen::builder
{
    struct function: std::enable_shared_from_this<function>
    {
        static std::shared_ptr<type> _bool();;
        static std::shared_ptr<type> _byte();
        static std::shared_ptr<type> _short();
        static std::shared_ptr<type> _int();
        static std::shared_ptr<type> _long();
        static std::shared_ptr<type> _float();
        static std::shared_ptr<type> _double();
        static std::shared_ptr<type> _unit();
        static std::shared_ptr<type> _string();

        std::vector<types::stack::i64> code;
        std::shared_ptr<zen::builder::signature> signature;
        bool logging = false;
        std::shared_ptr<value> ret;
        std::shared_ptr<block> scope;
        types::stack::i64 offset;
        utils::constant_pool & pool;
        std::string name;

        std::unordered_map<i64, std::shared_ptr<global_label>> dependencies;
        bool is_extern = false;
        inline std::shared_ptr<block> get_scope(const bool root  = false) const;
        static std::shared_ptr<function> create(utils::constant_pool & pool, const i64 & offset,const bool& logging = false, const std::string & name = "");
        std::shared_ptr<function> create(const std::string & name, const std::vector<std::shared_ptr<builder::type>>& params, const std::shared_ptr<builder::type> &type) const;
        std::shared_ptr<value> set_parameter(const std::shared_ptr<zen::builder::type>& t, const std::string& name);
        std::shared_ptr<value> set_return(const std::shared_ptr<zen::builder::type>& t);
        [[nodiscard]] std::shared_ptr<value> set_local(const std::shared_ptr<zen::builder::type>& t,
                                                       const std::string& name, bool param = false);
        std::shared_ptr<value> set_alias(const std::shared_ptr<zen::builder::value>& val,
                                                       const std::string& name) const;
        void add(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        void sub(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        void mul(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        void div(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        void mod(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        void equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                   const std::shared_ptr<value>& rhs);
        void not_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                       const std::shared_ptr<value>& rhs);
        void lower(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                   const std::shared_ptr<value>& rhs);
        void lower_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                         const std::shared_ptr<value>& rhs);
        void greater(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                     const std::shared_ptr<value>& rhs);
        void greater_equal(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs,
                           const std::shared_ptr<value>& rhs);
        void move(const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        void not_(const std::shared_ptr<value>& r, const std::shared_ptr<value>& val);
        void and_(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        void or_(const std::shared_ptr<value>& r, const std::shared_ptr<value>& lhs, const std::shared_ptr<value>& rhs);
        void return_value(const std::shared_ptr<value>& r);
        void return_implicitly() const;
        void go(const  std::shared_ptr<label>& l);
        void go_if_not(const std::shared_ptr<value>& c, const  std::shared_ptr<label>& l);
        std::expected<std::shared_ptr<value>, std::string> call(const std::shared_ptr<builder::function>& fb,
                                                                const std::vector<std::shared_ptr<value>>& args);
        i64 hash() const;
        std::string get_canonical_name() const;
        std::shared_ptr<value> dereference(const std::shared_ptr<value>& r);
        void branch(enum builder::scope::type, const std::shared_ptr<value>& c, const std::function<void(const std::shared_ptr<builder::
                            function>&, const std::shared_ptr<builder::label>&,
                        const std::shared_ptr<builder::label>&)>&, const std::shared_ptr<builder::label>& pel = nullptr, const std::
                    shared_ptr<builder::label>& pen = nullptr);
        void loop_for(const std::vector<std::shared_ptr<value>>& params, const std::function<void(const std::shared_ptr<
                          builder::function>&)>& body);
        template<typename T>
        std::shared_ptr<value> constant(const T & value, const std::shared_ptr<zen::builder::type> & t) const
        {
            if (*t == *_byte())
                return constant<i8>(static_cast<i8>(value));
            if (*t == *_short())
                return constant<i16>(static_cast<i16>(value));
            if (*t == *_int())
                return constant<i32>(static_cast<i32>(value));
            if (*t == *_long())
                return constant<i64>(static_cast<i64>(value));
            if (*t == *_float())
                return constant<f32>(static_cast<f32>(value));
            if (*t == *_double())
                return constant<f64>(static_cast<f64>(value));
            if (*t == *_bool())
                return constant<boolean>(static_cast<boolean>(value));
            throw exceptions::semantic_error(fmt::format("cannot create a constant of type {}", t->name), offset);
        }
        template<typename T>
        std::shared_ptr<value> constant(const T & value) const
        {
            if constexpr (std::is_same_v<T, bool>)
            {
                auto it = std::make_shared<builder::value>(fmt::format("_{}", value), _bool(), (i64)pool.get<T>(value).get());
                it->kind = value::kind::constant;
                return it;
            } else if constexpr  (std::is_same_v<T, i8>)
            {
                const auto it = std::make_shared<builder::value>(fmt::format("_{}", value), _byte(), (i64)pool.get<T>(value).get());
                it->is_negated = value < 0;
                it->kind = value::kind::constant;
                return it;
            } else if constexpr (std::is_same_v<T, i16>)
            {
                const auto it = std::make_shared<builder::value>(fmt::format("_{}", value), _short(), (i64)pool.get<T>(value).get());
                it->is_negated = value < 0;
                it->kind = value::kind::constant;
                return it;
            } else if constexpr (std::is_same_v<T, i32>)
            {
                const auto it = std::make_shared<builder::value>(fmt::format("_{}", value), _int(), (i64)pool.get<T>(value).get());
                it->is_negated = value < 0;
                it->kind = value::kind::constant;
                return it;
            } else if constexpr (std::is_same_v<T, i64>)
            {
                const auto it = std::make_shared<builder::value>(fmt::format("_{}", value), _long(), (i64)pool.get<T>(value).get());
                it->is_negated = value < 0;
                it->kind = value::kind::constant;
                return it;
            }  else if constexpr (std::is_same_v<T, f32>)
            {
                const auto it = std::make_shared<builder::value>(fmt::format("_{}", value), _float(), (i64)pool.get<T>(value).get());
                it->is_negated = value < 0;
                it->kind = value::kind::constant;
                return it;
            } else if constexpr (std::is_same_v<T, f64>)
            {
                const auto it = std::make_shared<builder::value>(fmt::format("_{}", value), _double(), (i64)pool.get<T>(value).get());
                it->is_negated = value < 0;
                it->kind = value::kind::constant;
                return it;
            } else if constexpr (std::is_same_v<T, std::string>)
            {
                static const std::regex replaceable("[^\\w\\d]");
                std::string label = std::regex_replace(value.substr(0, 5), replaceable, "_");
                auto it = std::make_shared<builder::value>(fmt::format("_{}", label), _string(), (i64)pool.get<zen::types::heap::string*>(zen::types::heap::string::from_string(value)).get());
                it->kind = value::kind::constant;
                return it;
            } else
            {
                throw exceptions::semantic_error("unsupported type for constant", offset);
            }
        };
        void loop_while(const std::vector<std::shared_ptr<value>>& params, const std::function<void(const std::shared_ptr<
                      builder::function>&)>& prologue, const std::function<void(const std::shared_ptr<
                      builder::function>&)>& body);
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
        [[nodiscard]]  std::shared_ptr<builder::label> label()
        {
            return builder::label::create();
        }
        void bind(const std::shared_ptr<builder::label>& label)
        {
            label->bind(code);
            if (logging)
            {
                fmt::println("%{}:", label->id);
            }
        }

        [[nodiscard]] std::string get_address_or_label(const std::shared_ptr<value>& _1) const
        {
            auto address = _1->address(get_stack_usage());
            return _1->name.empty() ? fmt::format("{}", address) : fmt::format("{}:{}", _1->name, address);
        }

        template <zen::instruction ins>
        void gen(const std::shared_ptr<value>& _1, const std::shared_ptr<value>& _2, const std::shared_ptr<value>& _3)
        {
            const std::shared_ptr<block>& sc = get_scope(true);
            code.push_back(ins);
            code.push_back(_1->address(get_stack_usage()));
            code.push_back(_2->address(get_stack_usage()));
            code.push_back(_3->address(get_stack_usage()));

            if (logging)
            {
                fmt::println("<{}> {} {} {} {}", code.size() - 4, code.at(code.size() - 4),
                             get_address_or_label(_1),
                             get_address_or_label(_2),
                             get_address_or_label(_3)
                );
            }
        }

        template <zen::instruction ins>
        void gen(const std::shared_ptr<value>& _1, const std::shared_ptr<value>& _2, const i64& _3, const std::string _3prefix = "")
        {
            const std::shared_ptr<block>& sc = get_scope(true);
            code.push_back(ins);
            code.push_back(_1->address(get_stack_usage()));
            code.push_back(_2->address(get_stack_usage()));
            code.push_back(_3);

            if (logging)
            {
                fmt::println("<{}> {} {} {} {}", code.size() - 4, code.at(code.size() - 4),
                             get_address_or_label(_1),
                             get_address_or_label(_2),
                             _3prefix.empty() ? std::to_string(_3) : _3prefix
                );
            }
        }


        template <zen::instruction ins>
        void gen(const std::shared_ptr<value>& _1, const std::shared_ptr<value>& _2)
        {
            const std::shared_ptr<block>& sc = get_scope(true);
            code.push_back(ins);
            code.push_back(_1->address(get_stack_usage()));
            code.push_back(_2->address(get_stack_usage()));

            if (logging)
            {
                fmt::println("<{}> {} {} {}", code.size() - 3, code.at(code.size() - 3),
                             get_address_or_label(_1),
                             get_address_or_label(_2)
                );
            }
        }

        template <zen::instruction ins>
        void gen(const std::shared_ptr<value>& _1, const i64& _2, const std::string _2prefix = "")
        {
            const std::shared_ptr<block>& sc = get_scope(true);
            code.push_back(ins);
            code.push_back(_1->address(get_stack_usage()));
            code.push_back(_2);

            if (logging)
            {
                fmt::println("<{}> {} {} {}", code.size() - 3, code.at(code.size() - 3),
                             get_address_or_label(_1),
                             _2prefix.empty() ? std::to_string(_2) : _2prefix
                );
            }
        }

        template <zen::instruction ins>
        void gen(const std::shared_ptr<value>& _1)
        {
            const std::shared_ptr<block>& sc = get_scope(true);
            code.push_back(ins);
            code.push_back(_1->address(get_stack_usage()));

            if (logging)
            {
                fmt::println("<{}> {} {}", code.size() - 2, code.at(code.size() - 2), get_address_or_label(_1));
            }
        }

        template <zen::instruction ins>
        void gen(const i64& _1, const std::string _1prefix = "")
        {
            const std::shared_ptr<block>& sc = get_scope(true);
            code.push_back(ins);
            code.push_back(_1);

            if (logging)
            {
                fmt::println("<{}> {} {}", code.size() - 2, code.at(code.size() - 2), _1prefix.empty() ? std::to_string(_1) : _1prefix);
            }
        }
        template <zen::instruction ins>
       void gen()
        {
            const std::shared_ptr<block>& sc = get_scope(true);
            code.push_back(ins);
            if (logging)
            {
                fmt::println("<{}> {}", code.size() - 1, code.at(code.size() - 1));
            }
        }
        function(utils::constant_pool & pool, const i64 & offset): offset(offset), pool(pool)
        {
        }
        function(const function&) = delete;
        function& operator=(const function&) = delete;
        void peek();
        void build();
    private:
        inline i64 get_stack_usage() const;
        void pop(bool final = false);
        std::shared_ptr<value> resolve(const std::shared_ptr<value> & variable_or_ref)
        {
            return variable_or_ref->is_reference ? dereference(variable_or_ref) : variable_or_ref;
        }
    };
}
