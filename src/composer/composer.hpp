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
            for (const auto & [fst, snd] : fields)
            {
                if (fst == name) return {offset, snd};
                offset += snd->_size;
            }
            throw exceptions::semantic_error(fmt::format("no such field {} in type {}", name, this->name), ilc_offset);
        }
        void add_field(const std::string & name, const std::shared_ptr<const type> & type)
        {
            _size += type->_size;
            fields.emplace_back(name, type);
        }
        explicit operator const std::string&() const { return name; }
        explicit type(std::string  name, const i64 size, enum kind kind = stack) : name(std::move(name)), _size(size), kind(kind) {}
        bool operator==(const type& other) const
        {
            return name == other.name && _size == other._size;
        }
        [[nodiscard]] i64 get_size() const { return kind == stack ? _size : static_cast<i64>(sizeof(i64)); }
    private:
        i64 _size;
    };



    enum call_result
    {
        result,
        casting
    };

    struct value
    {
        enum kind
        {
            temporary,
            constant,
            variable
        };
        kind kind;
        i64 offset;
        std::shared_ptr<const type> type;
        void prepare(std::vector<i64> & code, const i64 & most_size)
        {
            if (offset)
            {
                code.push_back(zen::walk);
                code.push_back(address(most_size));
                code.push_back(offset);
                offset = 0;
            }
        }

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
            if (kind == constant) return _address;
            return  _address - st_point;
        }
        explicit value(const std::shared_ptr<const composer::type> & type, const i64 & address, const enum kind & kind = variable) :
            type(type), kind(kind), offset(0), _address(address)
        {
        }

        i64 _address;
    };

    struct symbol : value
    {
        const std::string name;
        symbol(std::string  name, const std::shared_ptr<const composer::type> & type, const i64 & address) : value(type, address), name(std::move(name)) {}
    };

    struct signature
    {
        std::shared_ptr<const type> type;
        std::vector<std::shared_ptr<const composer::type>> parameters {};
    };

class composer {
public:
    int & _ilc_offset;
    utils::constant_pool _pool;
    std::stack<value> _stack;
    virtual std::shared_ptr<const type>& get_type(const std::string& name) = 0;
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
    virtual void set_return_name(const std::string & name) = 0;
    virtual void set_local(std::string name, const std::string & type) = 0;
    virtual void end() = 0;
    virtual void bake() = 0;
    // if args count is < 0, it means no assignment is occurring
    virtual call_result call(const std::string& name, const i8 & args_count) = 0;
    // TS <- TS+1
    virtual void assign() = 0;
    virtual void push(const std::string & name) = 0;
    template <typename native>
    void push(const native && data, const std::string & type)
    {
        auto t = get_type(type);
        if constexpr (std::is_same_v<native, value>)
            _stack.push(value(t, data.address));
        else
        {
            const i64 address = (i64)(_pool.get<native>(data).get());
            _stack.emplace(t, address, value::constant);
        }
    }

    virtual void pop() = 0;
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
    // virtual void operation_logic_extract(const symbol & destination, const value & value) = 0;
    virtual void greater() = 0;
    virtual void greater_or_equal() = 0;
    virtual void lower() = 0;
    virtual void lower_or_equal() = 0;
    virtual void equal() = 0;
    virtual void not_equal() = 0;
    virtual void ternary() = 0;

    virtual void pre_increment() = 0;
    virtual void pre_decrement() = 0;
    virtual void post_increment() = 0;
    virtual void post_decrement() = 0;
    // //
    virtual void begin_if_then() = 0;
    virtual void else_if_then() = 0;
    virtual void else_then() = 0;
    virtual void end_if() = 0;
    //
    // virtual void begin_for() = 0;
    // virtual void set_for_iterator(const symbol & iterator) = 0;
    // virtual void set_for_begin(const symbol & iterator, const value & value) = 0;
    // virtual void set_for_end(const symbol & iterator, const value & value) = 0;
    // virtual void set_for_step(const symbol & iterator, const value & value) = 0;
    // virtual void set_for_collection(const symbol & iterator, const value & value) = 0;
    // virtual void set_for_stream(const symbol & iterator, const value & value) = 0;
    // virtual void end_for(const value & value) = 0;
    //
    virtual void begin_while() = 0;
    virtual void end_while() = 0;
    //

    // virtual void begin_class(const std::string & name) = 0;
    // virtual void set_class_field(const std::string & name, const type & type) = 0;
    // virtual void symbol & get_class_field(const std::string & name) = 0;
    // virtual void symbol & get_class_method(const std::string & name) = 0;
    // virtual void end_class() = 0;
};

}
#endif //COMPOSER_HPP
