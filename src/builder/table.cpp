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
        const std::vector<std::string>& tokens, const std::function<std::shared_ptr<value>()>& pointer_creator)
    {
        if (target)
        {
            const auto new_target = pointer_creator();
            new_target->type = target->type;
            new_target->is_reference = target->is_reference;
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
            return target;
        }
        return std::unexpected(fmt::format("no such field {} in unit", tokens[0]));
    }

    std::expected<std::shared_ptr<value>, std::string> table::get_value(const std::string& name,  const std::function<std::shared_ptr<value>()>& pointer_creator)
    {
        if (name.empty())
        {
            return std::unexpected("invalid symbol name");
        }
        std::vector<std::string> tokens = split_name(name);
        std::shared_ptr<value> target = function->get_scope(true)->get_local(tokens[0]);
        if (target and tokens.size() > 1)
        {
            tokens.erase(tokens.begin());
            return get_field(target, tokens, pointer_creator);
        }
        return std::unexpected(fmt::format("no such symbol {}", tokens[0]));
    }

    std::expected<std::shared_ptr<value>, std::string> table::get_value(const std::string& name)
    {
        return get_value(name, [&]()
        {
            return this->function->set_local(zen::builder::function::_long(), "pointer::table");
        });
    }

    std::expected<std::shared_ptr<struct type>, std::string> table::get_type(const std::string& name)
    {
        if (name == "byte") return zen::builder::function::_byte();
        if (name == "bool") return zen::builder::function::_bool();
        if (name == "short") return zen::builder::function::_short();
        if (name == "int") return zen::builder::function::_int();
        if (name == "long") return zen::builder::function::_long();
        if (name == "float") return zen::builder::function::_float();
        if (name == "double") return zen::builder::function::_double();
        if (name == "string") return zen::builder::function::_string();

        return std::unexpected(fmt::format("no such type {}", name));
    }

    std::shared_ptr<table> table::create(const std::shared_ptr<builder::function>& function, std::shared_ptr<builder::type> type)
    {
        return std::make_shared<table>(function, type);
    }
}
