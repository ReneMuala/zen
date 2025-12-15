//
// Created by dte on 12/11/2025.
//

#include "table.hpp"

#include <expected>
#include <sstream>

namespace zen::builder
{
    static std::vector<std::string> split_name(const std::string& name)
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

    std::expected<std::shared_ptr<value>, std::string> table::get_value(const std::string& name)
    {
        if (name.empty())
        {
            return std::unexpected("invalid symbol name");
        }
        std::vector<std::string> tokens = split_name(name);
        std::shared_ptr<value> target = function->get_scope(true)->get_local(tokens[0]);
        if (target)
        {
            for (auto field_tuple = tokens.begin()+1; field_tuple != tokens.end(); ++field_tuple)
            {
                std::optional<std::pair<types::stack::i64, std::shared_ptr<builder::type>>> result = target->type->
                    get_field(*field_tuple);
                if (result.has_value())
                {
                    target->type = result->second;
                    target->offset += result->first;
                } else
                {
                    return std::unexpected(fmt::format("no such field {} in {}", *field_tuple, target->type->name));
                }
            }
            return target;
        }
        return std::unexpected(fmt::format("no such symbol {}", tokens[0]));
    }

    std::shared_ptr<table> table::create(const std::shared_ptr<builder::function>& function, std::shared_ptr<builder::type> type)
    {
        return std::make_shared<table>(function, type);
    }
}
