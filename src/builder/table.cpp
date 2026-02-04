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
                                                                        const std::vector<std::string>& tokens,
                                                                        const std::function<void(
                                                                            std::shared_ptr<value>&, const std
                                                                            ::shared_ptr<value>&)>& pointer_handler)
    {
        if (target)
        {
            const auto original = target;
            std::shared_ptr<value> new_target = {};
            pointer_handler(new_target, original);
            new_target->type = target->type;
            new_target->is_reference = true;
            new_target->no_destructor = true;
            target = new_target;
            for (const auto& token : tokens)
            {
                std::optional<std::pair<types::stack::i64, std::shared_ptr<builder::type>>> result = target->type->
                    get_field(token);
                if (result.has_value())
                {
                    target->type = result->second;
                    target->offset += result->first;
                }
                else
                {
                    return std::unexpected(fmt::format("no such field {} in {}", token, target->type->name));
                }
            }
            pointer_handler(target, original);
            return target;
        }
        return std::unexpected(fmt::format("no such field {} in unit", tokens[0]));
    }

    std::shared_ptr<builder::value> table::get_field_or_throw(const std::shared_ptr<builder::value> &object, const std::string & field) const
    {
        auto pointer_handler = [&](std::shared_ptr<zen::builder::value> & ptr, const std::shared_ptr<zen::builder::value>& original) -> void
        {
            if (not ptr)
                ptr = function->set_local(zen::builder::function::_long(), "temp::field");
            else
                function->gen<zen::add_i64>(ptr, original, ptr->offset, fmt::format("@offset:{}", ptr->offset));
        };
        std::shared_ptr<zen::builder::value> result_field;
        if (auto result = builder::table::get_field(object, {field}, pointer_handler); not result.has_value())
            throw exceptions::semantic_error(result.error(),0);
        else
            result_field = result.value();
        return result_field;
    }

    std::expected<std::shared_ptr<value>, std::string> table::get_value(
        const std::string& name, const std::function<void(std::shared_ptr<value>&, const std
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
        auto result = get_value(name, [&](std::shared_ptr<value>& ptr, const std::shared_ptr<value>& original)
        {
            if (not ptr)
            {
                ptr = this->function->set_local(zen::builder::function::_long(), "temp::field");
            }
            else
            {
                this->function->gen<zen::add_i64>(ptr, original, ptr->offset, fmt::format("@offset:{}", ptr->offset));
            }
        });
        if (not result.has_value() and type and not name.starts_with("this."))
        {
            return get_value("this." + name);
        }
        return result;
    }

    std::string table::simple_name(const std::string& name)
    {
        if (name.contains('.'))
        {
            const auto final = name.find_last_of('.');
            return name.substr(final+1);
        }
        return name;
    }

    std::string table::resolve_type_name(const std::string& name,
        const std::unordered_map<std::string, std::shared_ptr<builder::type>>& gcm)
    {
        if (const auto r = gcm.find(name); r != gcm.end())
        {
            return r->second->name;
        }
        return name;
    }

    const std::string table::extract_specified_lib(std::string& name, std::shared_ptr<builder::library>& specified_lib, std::shared_ptr<zen::builder::program> program)
    {
        const auto final = name.find_last_of(':');
        const auto lib_name = name.substr(0, final-1);
        for (const auto& lib : program->libraries)
        {
            if (lib.second->name == lib_name)
            {
                specified_lib = lib.second;
                name = name.substr(final+1);
            }
        }
        return lib_name;
    }

    std::expected<std::pair<std::shared_ptr<builder::value>,std::shared_ptr<builder::function>>, std::string> table::get_function(std::string name,
                                                                                                                                  std::vector<std::shared_ptr<zen::builder::type>>& params, std::string & hint)
    {
        std::shared_ptr<builder::function> target;
        std::shared_ptr<builder::value> object;
        std::shared_ptr<builder::library> specified_lib;
        if (name.contains(':'))
        {
            const std::string lib_name = extract_specified_lib(name, specified_lib, program);
            if (not specified_lib)
            {
                return std::unexpected(fmt::format("no such library {}", lib_name));
            }
        }
        if (name.contains('.'))
        {
            const auto final = name.find_last_of('.');
            if (auto result = get_value(name.substr(0, final)); result.has_value())
            {
                params.insert(params.begin(), result.value()->type);
                target = function->create(result.value()->type->name + name.substr(final), params, nullptr);
                object = result.value();
            } else
            {
                return std::unexpected(result.error());
            }
        }
        else
        {
            target = function->create(name, params, nullptr);
        }
        if (specified_lib)
        {
            if (const auto& r = specified_lib->get_function(target->hash()))
            {
                target = r;
                return std::pair{object, target};
            }
        }
        for (const auto& lib : program->libraries)
        {
            if (not specified_lib)
            {
                if (const auto& r = lib.second->get_function(target->hash()))
                {
                    target = r;
                    return std::pair{object, target};
                }
            }
            bool found = false;
            for (const auto & fn : lib.second->functions)
            {
                if (target->name == fn.second->name)
                {
                    if (not found)
                        hint += "\n\t- ";
                    found = true;
                    hint += fmt::format("{}, ", fn.second->get_canonical_name());
                }
            }
            if (found and hint.size() > 2)
            {
                hint = fmt::format("{} [library: {}]", hint.substr(0, hint.size()-2), lib.second->name);
            }
        }

        if (const auto g = generic_context_mapping.find(name); g != generic_context_mapping.end())
            return get_function(g->second->name, params, hint);
        if (type and not name.starts_with("this."))
            return get_function("this." + name, params, hint);
        if (not hint.empty())
        {
            hint = fmt::format("available alternatives: {}", hint);
            return std::unexpected(specified_lib ? fmt::format("no such function {}::{}",
                                           specified_lib->name,target->get_canonical_name()) : fmt::format("no such function {}",
                                           target->get_canonical_name()));
        }
        return std::unexpected(fmt::format("no such function {}",
                                           target->get_canonical_name()));
    }

    std::expected<std::shared_ptr<generic_context>, std::string>  table::get_generic_function_or_type(std::string name, const size_t param_count)
    {
        i64 hash;
        std::shared_ptr<builder::library> specified_lib;
        if (name.contains(':'))
        {
            const std::string lib_name = extract_specified_lib(name, specified_lib, program);
            if (not specified_lib)
            {
                return std::unexpected(fmt::format("no such library {}", lib_name));
            }
        }
        if (name.contains('.'))
        {
            const auto final = name.find_last_of('.');
            if (auto result = get_value(name.substr(0, final)); result.has_value())
            {
                const std::shared_ptr<builder::value>& object = result.value();
                name = object->type->name + name.substr(final);
                hash = zen::builder::generic_context::get_hash(name,param_count);
            } else
            {
                return std::unexpected(result.error());
            }
        } else
        {
            hash = zen::builder::generic_context::get_hash(name,param_count);
        }
        if (specified_lib)
        {
            if (const auto& r = specified_lib->get_generic_function(hash))
                return r;
            if (const auto& r = specified_lib->get_generic_type(hash))
                return r;
        } else
        {
            for (const auto& lib : program->libraries)
            {
                if (const auto& r = lib.second->get_generic_function(hash))
                    return r;
                if (const auto& r = lib.second->get_generic_type(hash))
                    return r;
            }
        }
        return std::unexpected(fmt::format("no such generic function {}", name));
    }

    std::expected<std::shared_ptr<generic_context>, std::string> table::get_generic_type(std::string name,
        size_t param_count, const std::shared_ptr<builder::program>& program)
    {
        i64 hash = zen::builder::generic_context::get_hash(name,param_count);
        std::shared_ptr<builder::library> specified_lib;
        if (name.contains(':'))
        {
            const std::string lib_name = extract_specified_lib(name, specified_lib, program);
            if (not specified_lib)
            {
                return std::unexpected(fmt::format("no such library {}", lib_name));
            }
        }
        if (specified_lib)
        {
            if (const auto& r = specified_lib->get_generic_type(hash))
            {
                return r;
            }
        } else
        {
            for (const auto& lib : program->libraries)
            {
                if (const auto& r = lib.second->get_generic_type(hash))
                {
                    return r;
                }
            }
        }
        return std::unexpected(fmt::format("no such generic class {}", name));
    }

    std::expected<std::pair<std::shared_ptr<builder::value>, std::shared_ptr<builder::function>>, std::string> table::
    get_function(const std::shared_ptr<zen::builder::value>& object, const std::string& name,
        std::vector<std::shared_ptr<zen::builder::type>>& params, std::string & hint)
    {
        params.insert(params.begin(), object->type);
        auto target = function->create(object->type->name + name, params, nullptr);
        for (const auto& lib : program->libraries)
        {
            if (const auto& r = lib.second->get_function(target->hash()))
            {
                target = r;
                return std::pair{object, target};
            }
        }
        const auto dotted_class_name = fmt::format("{}.", object->type->name);
        for (const auto& lib : program->libraries)
        {
            bool found = false;
            for (const auto & fn : lib.second->functions)
            {
                if (fn.second->name.starts_with(dotted_class_name))
                {
                    if (not found)
                        hint += "\n\t- ";
                    found = true;
                    hint += fmt::format("{}, ", fn.second->get_canonical_name());
                }
            }
            if (found and hint.size() > 2)
            {
                hint = fmt::format("{} [library: {}]", hint.substr(0, hint.size()-2), lib.second->name);
            }
        }
        if (not hint.empty())
        {
            hint = fmt::format("available overloads: {}", hint);
            return std::unexpected(fmt::format("no such overload {}",
                                           target->get_canonical_name()));
        }
        return std::unexpected(fmt::format("no such method {} found in class {}",
                                           target->get_canonical_name(), object->type->name));
    }

    // std::expected<std::shared_ptr<struct type>, std::string> table::get_type(const std::string& name)
    // {
    //     return get_type(name, program, generic_context_mapping);
    // }

    std::expected<std::vector<std::shared_ptr<builder::type>>, std::string> table::get_types(
        const std::vector<std::string>& names, const std::shared_ptr<builder::program>& program,  const std::unordered_map<std::string, std::shared_ptr<builder::type>> & gcm)
    {
        std::vector<std::shared_ptr<builder::type>> types;
        for (const auto& name : names)
        {
            if (auto result = get_type(name, program, gcm); result.has_value())
            {
                types.push_back(result.value());
            } else
            {
                return std::unexpected(result.error());
            }
        }
        return types;
    }

    std::expected<std::shared_ptr<builder::type>, std::string> table::get_type(const std::string& name,
                                                                               const std::shared_ptr<builder::program>& program, const std::unordered_map<std::string, std::shared_ptr<builder::type>> & gcm)
    {
        if (const auto t_index = gcm.find(name);t_index != gcm.end())
            return t_index->second;
        for (const auto& lib : program->libraries)
        {
            for (const auto& type : lib.second->types)
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

    std::shared_ptr<table> table::create(const std::shared_ptr<builder::function>& function,
                                            const std::unordered_map<std::string, std::shared_ptr<builder::type>> & gcm,
                                         const std::shared_ptr<builder::type>& type,
                                         const std::shared_ptr<builder::program>& program)
    {
        return std::make_shared<table>(gcm, function, type, program);
    }
    table_helpers::scope::scope(table& tab, std::string& name): tab(tab)
    {

    }

    table_helpers::scope::~scope()
    {
    }
}
