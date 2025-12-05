#include "composer.hpp"
#include <memory>

std::shared_ptr<zen::composer::bttr::scope> zen::composer::bttr::scope::make(int type){
    auto it = std::make_shared<zen::composer::bttr::scope>();
    it->type = type;
    return it;
}

std::shared_ptr<zen::composer::bttr::context> zen::composer::bttr::context::make(){
    return std::make_shared<zen::composer::bttr::context>();
}

void zen::composer::bttr::context::push(const std::shared_ptr<context> &ctt){
}

