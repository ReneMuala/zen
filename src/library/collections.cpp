//
// Created by dte on 12/16/2025.
//

#include "typing.hpp"

#include "builder/program.hpp"
/*
namespace zen::libraries::collections
{
    struct array : public builder::generic_context
    {
        utils::constant_pool & pool;
        array(utils::constant_pool & pool): pool(pool){}
        std::shared_ptr<::zen::builder::library> lib;
        std::vector<std::shared_ptr<builder::type>> hints;
        virtual std::shared_ptr<generic_context> with_hints(std::vector<std::shared_ptr<builder::type>>& hints) override
        {
            this->hints = hints;
            return shared_from_this();
        }
        std::expected<bool, std::string> implement(std::any program,const std::vector<std::shared_ptr<builder::type>>& arguments, const std::function<void(const std::unordered_map<std::string, std::shared_ptr<builder::type>>& mapping, long long offset, size_t chain_size, const std::shared_ptr<zen::builder::type>& class_)>& implementer) const override
        {
            std::unordered_map<std::string, std::shared_ptr<builder::type>> mapping;
            if (const auto result = get_mapping(arguments); result.has_value())
            {
                mapping = result.value();
            } else
            {
                return std::unexpected(result.error());
            }
            std::string type = hints.empty() ? std::string("") : hints.front()->name;
            if (hints.size() != arguments.size() or arguments.size() != 1)
            {
                return std::unexpected("wrong usage of collections::array<T>(t)");
            }
            const auto arg = arguments.front();
            const auto name = generic_context::get_name("array", arguments);
            const auto array = zen::builder::type::create(name);
            array->add_field("[data]", zen::builder::function::_long(), offset);
            array->add_field("[len]", zen::builder::function::_long(), offset);
            if (const auto result = lib->add(array); not result.has_value())
                return result;
            {
                auto fun = zen::builder::function::create(fmt::format("{}.{}", name, "len"), {}, zen::builder::function::_long());
            }
        }
    };

    std::shared_ptr<::zen::builder::library> create(utils::constant_pool & pool)
    {
        std::shared_ptr<::zen::builder::library> library = ::zen::builder::library::create("collections");
        auto is = std::make_shared<array>(pool);
        is->name = "array";
        is->add_parameter("T");
        is->offset = 0;
        is->chain_size = 0;
        is->lib = library;
        library->add_generic_type(is);
        return library;
    }
}
*/