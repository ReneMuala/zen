//
// Created by dte on 12/16/2025.
//

#include "typing.hpp"

#include "builder/program.hpp"

namespace zen::libraries::typing
{
    struct typing_is : public builder::generic_context
    {
        utils::constant_pool & pool;
        typing_is(utils::constant_pool & pool): pool(pool){}
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
            if (hints.size() == arguments.size() and arguments.size() == 1)
            {
                const auto fn = ::zen::builder::function::create(pool, 0,false, get_name("is", arguments));
                fn->set_return(::zen::builder::function::_bool());
                fn->set_parameter(hints.front(), "it");
                fn->return_value(fn->constant<bool>(hints.front() == arguments.front()));
                fn->build();
                lib->add(fn);
                return true;
            }
            return std::unexpected("wrong usage of is<T>(t)");
        }
    };

    std::shared_ptr<::zen::builder::library> create(utils::constant_pool & pool)
    {
        std::shared_ptr<::zen::builder::library> library = ::zen::builder::library::create("typing");
        auto is = std::make_shared<typing_is>(pool);
        is->name = "is";
        is->add_parameter("T");
        is->offset = 0;
        is->chain_size = 0;
        is->lib = library;
        library->add_generic_function(is);
        return library;
    }
}
