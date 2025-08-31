//
// Created by dte on 8/30/2025.
//

#ifndef COMPOSER_HPP
#define COMPOSER_HPP
#include <string>
#include <utility>

#include "utils/utils.hpp"
#include "vm/vm.hpp"

namespace zen::composer
{
    struct type
    {
        const std::string name;
        const i64 size;
        explicit operator const std::string&() const { return name; }
        bool operator==(const type& other) const
        {
            return name == other.name && size == other.size;
        }
    };

    struct value
    {
        const std::reference_wrapper<const type> type;
        const i64 address;
        value(const std::reference_wrapper<const composer::type> & type, const i64 & address) : type(type), address(address) {}
    };

    struct symbol : value
    {
        const std::string name;
        symbol(std::string  name, const std::reference_wrapper<const composer::type> & type, const i64 & address) : value(type, address), name(std::move(name)) {}
    };

    struct signature
    {
        std::reference_wrapper<const type> type;
        std::list<std::reference_wrapper<const composer::type>> parameters {};
    };

class composer {
protected:
    utils::constant_pool pool;
    std::stack<std::tuple<value, bool>> stack;
    public:

    virtual ~composer() = default;

    // virtual void begin_generic_context() = 0;
    // virtual void set_generic_context_parameter(std::string) = 0;
    // virtual void end_generic_context() = 0;
    //
    // virtual void begin_generic_context_resolution() = 0;
    // virtual void set_generic_context_resolution_parameter(std::string, const type & type) = 0;
    // virtual void end_generic_context_resolution() = 0;

    virtual void begin_function(std::string name) = 0;
    virtual void set_function_parameter(std::string name, const type & type) = 0;
    virtual void set_function_return(const type & type) = 0;
    virtual void set_function_return_value(const value & value) = 0;
    virtual void set_function_return_name(std::string name) = 0;
    virtual void set_function_local(std::string name, const type & type) = 0;
    virtual const symbol & get_function_local(const std::string & name) = 0;
    virtual void end_function() = 0;
    virtual void bake() = 0;

    virtual void operation_assign(const value & destination, const value & source) = 0;

    virtual void push_local_temp(const type & type) = 0;

    template <typename native>
    void push(native && data, const type & t)
    {
        if constexpr (std::is_same_v<native, value>)
            stack.push({value(t, data.address), false});
        else
        {
            const i64 address = (i64)(pool.get<native>(data).get());
            stack.push({value(std::ref(t), address), false});
        }
    }
    value & top()
    {
        return std::get<0>(stack.top());
    }
    virtual void pop() = 0;

    virtual void operation_arith_plus(const value & destination, const value & first, const value & second) = 0;
    virtual void operation_arith_minus(const value & destination, const value & first, const value & second) = 0;
    virtual void operation_arith_multi(const value & destination, const value & first, const value & second) = 0;
    virtual void operation_arith_divide(const value & destination, const value & first, const value & second) = 0;
    virtual void operation_arith_mod(const value & destination, const value & first, const value & second) = 0;

    // virtual void operation_logic_and(const symbol & destination, const value & first, const value & second) = 0;
    // virtual void operation_logic_or(const symbol & destination, const value & first, const value & second) = 0;
    // virtual void operation_logic_not(const symbol & destination, const value & value) = 0;
    // virtual void operation_logic_extract(const symbol & destination, const value & value) = 0;
    //
    // virtual void operation_rel_g(const symbol & destination, const value & first, const value & second) = 0;
    // virtual void operation_rel_ge(const symbol & destination, const value & first, const value & second) = 0;
    // virtual void operation_rel_l(const symbol & destination, const value & first, const value & second) = 0;
    // virtual void operation_rel_le(const symbol & destination, const value & first, const value & second) = 0;
    // virtual void operation_rel_eq(const symbol & destination, const value & first, const value & second) = 0;
    // virtual void operation_rel_neq(const symbol & destination, const value & first, const value & second) = 0;
    //
    // virtual void begin_if_then(const value & value) = 0;
    // virtual void else_if_then(const value & value) = 0;
    // virtual void else_then() = 0;
    // virtual void end_if() = 0;
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
    // virtual void begin_while(const value & condition) = 0;
    // virtual void end_while() = 0;
    //
    // virtual void begin_function_call(const symbol & function) = 0;
    // virtual void set_function_parameter(const symbol & parameter, const value & value) = 0;
    // virtual void end_function_call(const symbol & destination) = 0;
    // virtual void end_function_call() = 0;

    // virtual void begin_class(const std::string & name) = 0;
    // virtual void set_class_field(const std::string & name, const type & type) = 0;
    // virtual void symbol & get_class_field(const std::string & name) = 0;
    // virtual void symbol & get_class_method(const std::string & name) = 0;
    // virtual void end_class() = 0;

    // virtual void begin_static_context() = 0;
    // virtual void end_static_context() = 0;

    // virtual symbol & get_static_field(const std::string & classname, const std::string & name) = 0;
    // virtual symbol & get_static_method(const std::string & classname, const std::string & name) = 0;
    virtual const type & get_type(const std::string & name) = 0;
    // virtual const symbol & get_function(const std::string & name, const signature & signature) = 0;
    // virtual const symbol & get_constructor(const type & name, const signature & signature) = 0;
};

}
#endif //COMPOSER_HPP
