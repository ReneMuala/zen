//
// Created by dte on 8/30/2025.
//

#ifndef COMPOSER_HPP
#define COMPOSER_HPP
#include <string>
#include <utility>

#include "exceptions/semantic_error.hpp"
#include "utils/utils.hpp"
#include "vm/vm.hpp"

namespace zen::composer
{
    struct type
    {
        enum kind
        {
            stack,
            heap
        } kind;
        const std::string name;
        std::shared_ptr<type> base;
        std::vector<std::pair<std::string, std::shared_ptr<const type>>> fields;
        [[nodiscard]] std::pair<i64, std::shared_ptr<const type>> get_field(const std::string & name, const i32 & ilc_offset = 0) const
        {
            i64 offset = 0;
            std::string options;
            for (const auto & [fst, snd] : fields)
            {
                if (fst == name) return {offset, snd};
                offset += snd->_size;
                options += fst + ",";
            }
            if (not options.empty())
            {
                options.pop_back();
                options="(options: " + options + ")";
            }
            throw exceptions::semantic_error(fmt::format("no such field {} in type {}{}", name, this->name, options), ilc_offset);
        }
        void add_field(const std::string & name, const std::shared_ptr<const type> & type)
        {
            _size += type->_size;
            fields.emplace_back(name, type);
        }
        explicit operator const std::string&() const { return name; }
        explicit type(std::string  name, const i64 size, const enum kind kind = stack) : kind(kind), name(std::move(name)), _size(size) {}
        bool operator==(const type& other) const
        {
            return name == other.name && _size == other._size;
        }
        [[nodiscard]] i64 get_size() const { return kind == stack ? _size : static_cast<i64>(sizeof(i64)); }
        [[nodiscard]] i64 get_full_size() const { return _size; }
    private:
        i64 _size;
    };



    enum call_result
    {
        pushed,
        assignment,
        pushed_from_constructor
    };

    struct value
    {
        enum kind
        {
            temporary,
            constant,
            fake_constant,
            variable
        };
        kind kind;
        i64 offset;
        std::shared_ptr<const type> type;
        bool no_destructor = false;
        std::string label;
        bool is_reference = false;
        bool is_negated = false;
        [[nodiscard]] bool is(const std::string & type_name) const
        {
            return type->name == type_name;
        }
        [[nodiscard]] bool has_same_type_as(const value & other) const
        {
            return *type == *other.type;
        }
        [[nodiscard]] i64 address(const i64 & st_point) const
        {
            return kind == kind::constant or kind == kind::fake_constant ? _address : _address - st_point;
        }
        explicit value(const std::shared_ptr<const composer::type> & type, const i64 & address, const enum kind & kind = variable) :
            kind(kind), offset(0), type(type), _address(address)
        {
        }

        explicit  value(const std::string& label, const std::shared_ptr<const composer::type> & type, const i64 & address) : value(type, address)
        {
            this->label = label;
        }

        i64 _address;
    };

    struct signature
    {
        std::shared_ptr<const type> type;
        std::vector<std::shared_ptr<const composer::type>> parameters {};

        bool operator==(const signature& other) const
        {
            return this->type == other.type and this->parameters == other.parameters;
        }
    };

class composer {
public:
    int & _ilc_offset;
    utils::constant_pool _pool;
    std::stack<std::shared_ptr<value>> _stack;
    virtual std::shared_ptr<const type>& get_type(const std::string& name) = 0;
    virtual void begin_type(std::shared_ptr<type>&) = 0;
    virtual void end_type(std::shared_ptr<type>&) = 0;
    virtual void push(const std::shared_ptr<const type>& type) = 0;

    virtual void reset()
    {
        _pool.data.clear();
        while (not _stack.empty()) _stack.pop();
    }
    explicit composer(int & ilc_offset): _ilc_offset(ilc_offset) {}
    virtual ~composer() = default;

    // virtual void begin_generic_context() = 0;
    // virtual void set_generic_context_parameter(std::string) = 0;
    // virtual void end_generic_context() = 0;
    //
    // virtual void begin_generic_context_resolution() = 0;
    // virtual void set_generic_context_resolution_parameter(std::string, const type & type) = 0;
    // virtual void end_generic_context_resolution() = 0;

    virtual void begin(std::string name) = 0;
    virtual void set_parameter(std::string name, const std::string & type) = 0;
    virtual void set_return_type(const std::string & name) = 0;
    virtual void return_value() = 0;
    virtual void assume_returned() = 0;
    virtual void set_return_name(const std::string & name) = 0;
    virtual void set_local(std::string name, const std::string & type) = 0;
    virtual void end() = 0;
    virtual void bake() = 0;
    // returns true if the called functions returns a value
    virtual bool call(const std::string& name, const i8 & args_count) = 0;
    // TS <- TS+1
    virtual void assign() = 0;
    virtual void push(const std::string & name) = 0;
    template <typename native>
    void push(const native & data, const std::string & type, bool negate = false)
    {
        auto t = get_type(type);
        if constexpr (std::is_same_v<native, value>)
        {
            auto v = std::make_shared<value>(t, data.address);
            v->is_negated = negate;
            _stack.push(v);
        } else
        {
            const i64 address = (i64)_pool.get<native>(data).get();
            auto v = std::make_shared<value>(t, address, value::constant);
            v->is_negated = negate;
            _stack.push(v);
        }
    }

    virtual void pop() = 0;
    virtual void peek_pop_pop_push() = 0;
    // PUSH(TS + TS+1)
    virtual void plus() = 0;
    // PUSH(TS - TS+1)
    virtual void minus() = 0;
    // PUSH(TS * TS+1)
    virtual void times() = 0;
    // PUSH(TS / TS+1)
    virtual void slash() = 0;
    // PUSH(TS % TS+1)
    virtual void modulo() = 0;

    virtual void and_() = 0;
    virtual void or_() = 0;
    virtual void not_() = 0;
    virtual void negate() = 0;
    // virtual void operation_logic_extract(const symbol & destination, const value & value) = 0;
    virtual void greater() = 0;
    virtual void greater_or_equal() = 0;
    virtual void lower() = 0;
    virtual void lower_or_equal() = 0;
    virtual void equal() = 0;
    virtual void not_equal() = 0;

    virtual void pre_increment() = 0;
    virtual void pre_decrement() = 0;
    virtual void post_increment() = 0;
    virtual void post_decrement() = 0;
    // //
    virtual void begin_if_then() = 0;
    virtual void else_if_then() = 0;
    virtual void else_then() = 0;
    virtual void close_branch() = 0;
    virtual void end_if() = 0;

    virtual void begin_for() = 0;
    virtual void set_for_iterator() = 0;
    virtual void set_for_begin_end() = 0;
    virtual void set_for_begin_end_step() = 0;
    virtual void end_for() = 0;
    //
    virtual void begin_while() = 0;
    virtual void set_while_condition() = 0;
    virtual void end_while() = 0;

    virtual void begin_block() = 0;
    virtual void end_block() = 0;

    virtual void using_(const std::string & alias_function, const std::string & original_function) = 0;
    virtual void link() = 0;
    //

    // virtual void begin_class(const std::string & name) = 0;
    // virtual void set_class_field(const std::string & name, const type & type) = 0;
    // virtual void symbol & get_class_field(const std::string & name) = 0;
    // virtual void symbol & get_class_method(const std::string & name) = 0;
    // virtual void end_class() = 0;
};

}
#endif //COMPOSER_HPP
