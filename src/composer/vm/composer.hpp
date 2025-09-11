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
protected:
    std::shared_ptr<const type>& get_type(const std::string& name) override;
    std::unordered_map<std::string,std::tuple<signature, i64>> functions;
    std::vector<i64> code;
    std::unordered_map<std::string, std::shared_ptr<const type>> types;
    struct scope_data
    {
        std::string function_name;
        i64 local_most_size{};
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
    value top();
    void push(const value&);
public:
    void reset() override;
    composer(int & ilc_offset);
    ~composer() override = default;
    void begin(std::string name) override;
    void set_parameter(std::string name, const std::string& type) override;
    void set_return_type(const std::string& name) override;
    void return_value() override;
    void set_return_name(const std::string& name) override;
    void set_local(std::string name, const std::string& type) override;
    void end() override;
    void bake() override;
    void assign() override;
    void push(const std::string& name) override;
    void pop() override;
    void plus() override;
    void minus() override;
    void times() override;
    void slash() override;
    void modulo() override;
    std::optional<value> _push_calle_return_value(const signature& sig);
    void _push_calle_arguments(const signature& sig, const i8& args_count);
    call_result _call_caster(const std::string& name, const i8& args_count, const std::unordered_map<std::string, std::unordered_map<std::string, i64>>::iterator & caster_set);
    call_result call(const std::string& name, const i8& args_count) override;

protected:
    void push(const std::shared_ptr<const type>& type) override;

};

} // zen

#endif //VM_COMPOSER_HPP
