//
// Created by dte on 8/31/2025.
//

#ifndef VM_COMPOSER_HPP
#define VM_COMPOSER_HPP
#include <list>

#include "../composer.hpp"
#include <vm/vm.hpp>

#include "function.hpp"
#include "function_scope.hpp"
#include "utils/utils.hpp"
#include "label.hpp"

namespace zen::composer::vm {
class composer : public zen::composer::composer
{
public:
    std::unique_ptr<function_scope> scope;
    std::vector<i64> code;
    std::unordered_map<std::string, std::list<function>> functions;
    std::unordered_map<std::string, std::shared_ptr<const type>> types;
    std::shared_ptr<type> class_;
    std::shared_ptr<const type>& get_type(const std::string& name) override;
    void begin_type(std::shared_ptr<type>&) override;
    void end_type() override;
    std::shared_ptr<value> top();
    void push(const std::shared_ptr<value>&);
    void reset() override;
    explicit composer(int & ilc_offset);
    ~composer() override = default;
    void begin(std::string name) override;
    void set_parameter(std::string name, const std::string& type) override;
    void set_return_type(const std::string& name) override;
    void return_value() override;
    void assume_returned() override;
    void set_return_name(const std::string& name) override;
    void construct(const std::shared_ptr<const zen::composer::type>& t);
    void set_local(std::string name, const std::string& type) override;
    void end() override;
    void bake() override;
    void assign() override;
    void _push_variable(const std::vector<std::string> & tokens, const std::shared_ptr<value>& location);
    void _push_function();
    void _push_temporary_value(const std::string& type_name);
    void push(const std::string& name) override;
    void pop() override;
    void peek_pop_pop_push() override;
    void begin_if_then() override;
    void _begin_if_then(bool chained);
    void else_if_then() override;
    void _else_then(bool partial);
    void else_then() override;
    void close_branch() override;
    void end_if() override;
    void plus() override;
    void minus() override;
    void times() override;
    void slash() override;
    void modulo() override;
    std::shared_ptr<value> _push_callee_return_value(const signature& sig,  bool construtor_call);
    void _push_callee_arguments(const std::deque<std::shared_ptr<value>>& arguments);
    void _call_caster(const std::string& name, const i8& args_count, const std::unordered_map<std::string, std::unordered_map<std::string,
                                 i64>>::iterator&
                             caster_set);
    bool _call_function_overload(const std::deque<std::shared_ptr<value>>& arguments, function& func,
                                 bool construtor_call);
    bool _call_function(const std::string& name, const i8& args_count,
                        const std::unordered_map<std::string, std::list<function>>::iterator&
                        func_it, std::shared_ptr<value> & caster_arg_buffer);
    bool _call_instruction_write_str(const std::string& name, const i8& args_count);
    bool _call_instruction(const zen::instruction& instruction, const i8& args_count, const i8& expected_args_count);
    std::optional<std::unordered_map<std::string, std::list<function>>::iterator> fetch_function(
        const std::string& name);
    bool call(const std::string& name, const i8& args_count) override;
    void and_() override;
    void or_() override;
    void not_() override;
    void negate() override;
    void greater() override;
    void greater_or_equal() override;
    void lower() override;
    void lower_or_equal() override;
    void equal() override;
    void not_equal() override;

    void begin_while() override;
    void set_while_condition() override;
    void end_while() override;

    void pre_increment() override;
    void pre_decrement() override;
    void post_increment() override;
    void post_decrement() override;

    void begin_for() override;
    void set_for_iterator() override;
    void set_for_begin_end() override;
    void set_for_begin_end_step() override;
    void end_for() override;
    void using_(const std::string& alias_function, const std::string& original_function) override;
    void _link_string_constructor();
    void _link_string_destructor();
    void _link_string_copy();
    void _link_string_equals();
    void _link_string_not_equals();
    void _link_string_plus();
    void link() override;
    std::shared_ptr<value> dereference(const std::shared_ptr<value> & value);
    void begin_block() override;
    void end_block() override;
protected:
    void self_assign_reference(const std::shared_ptr<value>& value);
    void assign_reference(const std::shared_ptr<value>& to, const std::shared_ptr<value>& from);
    std::shared_ptr<value> pop_operand(bool allow_dereferencing = true);
    void push(const std::shared_ptr<const type>& type) override;
    static i64 get_parameters_size(const signature& sig);
    static i64 get_return_size(const signature& sig);
    void increment_by_one();

public:

};

} // zen

#endif //VM_COMPOSER_HPP