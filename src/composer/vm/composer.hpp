//
// Created by dte on 8/31/2025.
//

#ifndef VM_COMPOSER_HPP
#define VM_COMPOSER_HPP
#include <list>

#include "../composer.hpp"
#include <vm/vm.hpp>

#include "utils/utils.hpp"

namespace zen::composer::vm {
class composer : zen::composer::composer
{
    std::list<std::tuple<std::string, signature, i64>> functions;
    std::vector<i64> code;
    std::unordered_map<std::string, type> types;
    struct scope_data
    {
        i64 local_most_size;
        std::map<std::string, symbol> locals;
        std::optional<value> return_value;
        std::optional<std::string> return_name;

        scope_data()
        {
            clear();
        }

        void clear()
        {
            local_most_size = 0;
            locals.clear();
            return_value = std::nullopt;
            return_name = std::nullopt;
        }
    } scope;
public:
    composer();
    ~composer() override;

    void begin_function(std::string name) override;
    void set_function_parameter(std::string name, const type& type) override;
    void set_function_return(const type& type) override;
    void set_function_return_name(std::string name) override;
    void set_function_local(std::string name, const type& type) override;
    void end_function() override;
    const type& get_type(const std::string& name) override;
    const symbol& get_function_local(const std::string& name) override;
    void operation_assign(const value& destination, const value& source) override;

    void bake() override;
    void set_function_return_value(const value& value) override;
    void push_local_temp(const type& type) override;
    void pop() override;
    void operation_arith_plus(const value& destination, const value& first, const value& second) override;
    void operation_arith_minus(const value& destination, const value& first, const value& second) override;
    void operation_arith_multi(const value& destination, const value& first, const value& second) override;
    void operation_arith_divide(const value& destination, const value& first, const value& second) override;
    void operation_arith_mod(const value& destination, const value& first, const value& second) override;
};

} // zen

#endif //VM_COMPOSER_HPP
