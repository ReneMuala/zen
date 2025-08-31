//
// Created by dte on 8/30/2025.
//

#ifndef COMPOSER_HPP
#define COMPOSER_HPP
#include <string>

template<typename T, typename V, typename S, typename FS>
class composer {
    public:
    virtual ~composer() = default;

    virtual void begin_generic_context() = 0;
    virtual void set_generic_context_parameter(std::string) = 0;
    virtual void end_generic_context() = 0;

    virtual void begin_generic_context_resolution() = 0;
    virtual void set_generic_context_resolution_parameter(std::string, const T & type) = 0;
    virtual void end_generic_context_resolution() = 0;

    virtual void begin_function(std::string name) = 0;
    virtual void set_function_parameter(std::string name, const T & type) = 0;
    virtual void set_function_return(const T & type) = 0;
    virtual void set_function_return(const T & type, std::string name) = 0;
    virtual void set_function_local(std::string name, const T & type) = 0;
    virtual void end_function() = 0;

    virtual void operation_assign(const S & destination, const V & source) = 0;
    virtual void operation_arith_plus(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_arith_minus(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_arith_multi(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_arith_divide(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_arith_mod(const S & destination, const V & first, const V & second) = 0;

    virtual void operation_logic_and(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_logic_or(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_logic_not(const S & destination, const V & value) = 0;
    virtual void operation_logic_extract(const S & destination, const V & value) = 0;

    virtual void operation_real_g(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_real_ge(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_real_l(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_real_le(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_real_eq(const S & destination, const V & first, const V & second) = 0;
    virtual void operation_real_neq(const S & destination, const V & first, const V & second) = 0;

    virtual void begin_if_then(const V & value) = 0;
    virtual void else_if_then(const V & value) = 0;
    virtual void else_then() = 0;
    virtual void end_if() = 0;

    virtual void begin_for() = 0;
    virtual void set_for_iterator(const S & iterator) = 0;
    virtual void set_for_begin(const S & iterator, const V & value) = 0;
    virtual void set_for_end(const S & iterator, const V & value) = 0;
    virtual void set_for_step(const S & iterator, const V & value) = 0;
    virtual void set_for_collection(const S & iterator, const V & value) = 0;
    virtual void set_for_stream(const S & iterator, const V & value) = 0;
    virtual void end_for(const V & value) = 0;

    virtual void begin_while(const V & condition) = 0;
    virtual void end_while() = 0;

    virtual void begin_function_call(const S & function) = 0;
    virtual void set_function_parameter(const S & parameter, const V & value) = 0;
    virtual void end_function_call(const S & destination) = 0;
    virtual void end_function_call() = 0;

    virtual void begin_class(const std::string & name) = 0;
    virtual void set_class_field(const std::string & name, const T & type) = 0;
    virtual void S & get_class_field() = 0;
    virtual void S & get_class_method(const std::string & name) = 0;
    virtual void end_class() = 0;

    virtual void begin_static_context() = 0;
    virtual S & get_static_field() = 0;
    virtual S & get_static_method() = 0;
    virtual void end_static_context() = 0;

    virtual const T & get_type(const std::string & name) = 0;
    virtual const S & get_function(const std::string & name, const FS & signature) = 0;
    virtual const S & get_constructor(const T & name, const FS & signature) = 0;
};

#endif //COMPOSER_HPP
