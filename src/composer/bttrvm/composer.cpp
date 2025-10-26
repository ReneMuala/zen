#include "composer.hpp"
#include <memory>

std::shared_ptr<zen::composer::bttrvm::scope> zen::composer::bttrvm::scope::make(int type){
    auto it = std::make_shared<zen::composer::bttrvm::scope>();
    it->type = type;
    return it;
}

std::shared_ptr<zen::composer::bttrvm::context> zen::composer::bttrvm::context::make(){
    return std::make_shared<zen::composer::bttrvm::context>();
}

void zen::composer::bttrvm::context::push(const std::shared_ptr<context> &ctt){
    ;
}


bool zen::composer::bttrvm::function::begin(const std::shared_ptr<context> & ctt, const std::string & name){
    return false;
}

bool zen::composer::bttrvm::function::end(const std::shared_ptr<context> & ctt){
    return false;
}

