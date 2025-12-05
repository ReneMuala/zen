#include "composer.hpp"
#include <memory>

std::shared_ptr<zen::composer::vm::function> zen::composer::bttr::function::begin(const std::shared_ptr<context> & ctx){
    auto func = std::shared_ptr<zen::composer::vm::function>();
    func->address = ctx->code.size();
    return func;
}

void zen::composer::bttr::function::end(const std::shared_ptr<context> & ctx, const std::shared_ptr<vm::function> & func){
    return false;
}

