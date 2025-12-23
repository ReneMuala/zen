//
// Created by dte on 12/11/2025.
//

#include "table.hpp"

#include <expected>
#include <sstream>

namespace zen::builder
{
    std::vector<std::string> table::split_name(const std::string& name)
    {
        std::vector<std::string> result;
        std::istringstream iss(name);
        std::string token;
        while (std::getline(iss, token, '.'))
        {
            if (not token.empty())
                result.push_back(token);
        }
        return result;
    }

    std::expected<std::shared_ptr<value>, std::string> table::get_field(std::shared_ptr<value> target,
                                                                        const std::vector<std::string>& tokens, const std::function<void(std::shared_ptr<value>&, const std
                                                                            ::shared_ptr<value>&)>& pointer_handler)
    {
        if (target)
        {
            const auto original = target;
            std::shared_ptr<value> new_target = {};
            pointer_handler(new_target, original);
            new_target->type = target->type;
            new_target->is_reference = true;
            target = new_target;
            for (const auto & token : tokens)
            {
                std::optional<std::pair<types::stack::i64, std::shared_ptr<builder::type>>> result = target->type->
                    get_field(token);
                if (result.has_value())
                {
                    target->type = result->second;
                    target->offset += result->first;
                } else
                {
                    return std::unexpected(fmt::format("no such field {} in {}", token, target->type->name));
                }
            }
            pointer_handler(target, original);
            return target;
        }
        return std::unexpected(fmt::format("no such field {} in unit", tokens[0]));
    }

    std::expected<std::shared_ptr<value>, std::string> table::get_value(const std::string& name, const std::function<void(std::shared_ptr<value>&, const std
                                                                            ::shared_ptr<value>&)>& pointer_handler)
    {
        if (name.empty())
        {
            return std::unexpected("invalid symbol name");
        }
        std::vector<std::string> tokens = split_name(name);
        std::shared_ptr<value> target = function->get_scope(true)->get_local(tokens[0]);
        if (target)
        {
            if (tokens.size() > 1)
            {
                tokens.erase(tokens.begin());
                return get_field(target, tokens, pointer_handler);
            }
            return target;
        }
        return std::unexpected(fmt::format("no such symbol {}", tokens[0]));
    }

    std::expected<std::shared_ptr<value>, std::string> table::get_value(const std::string& name)
    {
        auto result = get_value(name, [&](std::shared_ptr<value> & ptr, const std::shared_ptr<value>& original)
        {
            if (not ptr)
            {
                ptr = this->function->set_local(zen::builder::function::_long(), "temp::field");
            } else
            {
                this->function->gen<zen::add_i64>(ptr, original, ptr->offset, fmt::format("@offset:{}", ptr->offset));
            }
        });
        if (not result.has_value() and type and not name.starts_with("this."))
        {
            return get_value("this."+name);
        }
        return result;
    }

    std::expected<std::shared_ptr<builder::function>, std::string> table::get_function(const std::string& name,
        const std::vector<std::shared_ptr<zen::builder::type>>& params)
    {
        auto target = function->create(name, params, nullptr);
        for (const auto & lib : program->libraries)
        {
            if (const auto & r = lib.second->get_function(target->hash()))
            {
                target = r;
                return target;
            }
        }
        if (type and not name.starts_with("this."))
        {
            return get_function("this."+name, params);
        }
        return std::unexpected(fmt::format("no such function {} found at compilation phase", target->get_canonical_name()));
    }

    std::expected<std::shared_ptr<struct type>, std::string> table::get_type(const std::string& name)
    {
        for (const auto & lib : program->libraries)
        {
            for (const auto & type : lib.second->types)
            {
                if (type.second->name == name)
                {
                    return type.second;
                }
            }
        }
        if (name == "byte") return zen::builder::function::_byte();
        if (name == "bool") return zen::builder::function::_bool();
        if (name == "short") return zen::builder::function::_short();
        if (name == "int") return zen::builder::function::_int();
        if (name == "long") return zen::builder::function::_long();
        if (name == "float") return zen::builder::function::_float();
        if (name == "double") return zen::builder::function::_double();
        if (name == "string") return zen::builder::function::_string();
        if (name == "unit") return zen::builder::function::_unit();
        return std::unexpected(fmt::format("no such type {}", name));
    }

    std::shared_ptr<table> table::create(const std::shared_ptr<builder::function>& function, const std::shared_ptr<builder::type>& type, const std::shared_ptr<builder::program>& program)
    {
        return std::make_shared<table>(function, type, program);
    }
}
