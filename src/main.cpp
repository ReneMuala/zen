
#include <iostream>
#include <memory>
#include <optional>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

#include <vm/vm.hpp>
#include <sstream>
#include "utils/utils.hpp"

#ifdef KAIZEN_WASM
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif
#include <chrono>
std::vector<zen::token> tokens;

extern "C" {
EMSCRIPTEN_KEEPALIVE

void zen_sum(int a, int b)
{
    fmt::print("{} + {} = {}\n", a, b, a + b);
}

void EMSCRIPTEN_KEEPALIVE zen_reset()
{
    get_composer()->reset();
}
    inline void setup_parser_test(const std::string& code)
{
    ILC::chain.clear();
    tokens.clear();
    parser::reset();
    std::stringstream stream(code);
    zen::lexer lexer(stream);
    while (auto token = lexer.next())
    {
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    ILC::chain_size = ILC::chain.size();
}

EMSCRIPTEN_KEEPALIVE
bool zen_run(const char* code)
try
{
    if (code == nullptr)
    {
        fmt::println("[zen_compile] called with null argument.");
        return false;
    }

    // zen::composer::composer* composer = get_composer();
        auto composer = dynamic_cast<zen::composer::vm::composer*>(get_composer());
        composer->reset();

        composer->begin("print");
        composer->set_parameter("string", "string");
        {
            composer->push("string.data");
            const auto deref = composer->dereference(composer->top());
            composer->pop();
            composer->push(deref);
        }
        {
            composer->push("string.len");
            const auto deref = composer->dereference(composer->top());
            composer->pop();
            composer->push(deref);
        }
        composer->zen::composer::composer::push<zen::i64>(reinterpret_cast<zen::i64>(stdout), "long");
        composer->call(std::to_string(zen::write_str), 3, zen::composer::call_result::pushed);
        composer->end();

        composer->link();
        setup_parser_test(std::string(code));
        parse();
        // composer->bake();
        const std::list<zen::composer::vm::function> main_functions = composer->functions["main"];
        if (main_functions.empty())
        {
            fmt::print("[runtime error: main function not found]\n");
            return 0;
        }
        const zen::composer::vm::function main = main_functions.front();
        zen::vm::stack stack;
        stack.push<zen::i64>(0); // returning address
        zen::vm vm1;
        vm1.load(composer->code);
        vm1.run(stack, main.address);
        fmt::println("");
    return true;
}
catch (std::exception& e)
{
    std::cout << e.what() << std::endl;
    return false;
}
}

void* make_zen_string(const std::string& str)
{
    void* data = malloc(sizeof(char) * str.size() + sizeof(zen::i64));
    *static_cast<zen::i64*>(data) = str.length();
    memcpy((char*)(data) + sizeof(zen::i64), str.data(), str.length());
    return data;
}

void print_zen_string(void* str)
{
    zen::i64 len = *static_cast<zen::i64*>(str);
    zen::i64 i = 0;
    while (i < len)
    {
        printf("%c", *(char*)((char*)str + sizeof(zen::i64) + i++));
    }
    // printf("(%d bytes)", len);
}

extern char foo_case0[];

struct demo_struct_string
{
    zen::i64 a;
    zen::i64 b;
};

inline void setup_integration_test(const std::string& code, zen::composer::composer* composer)
{
    composer->reset();
    setup_parser_test(code);
}

int main(int argc, char** argv) try
{
#ifdef KAIZEN_WASM
    std::cout << "Click 'Run' or hit CTR+R to execute your ZEN code. Output will appear here." << std::endl;
return 0;
#else
/*
    auto composer = dynamic_cast<zen::composer::vm::composer*>(get_composer());
    composer->reset();

    composer->begin("main");
    composer->set_local("x", "int");
    composer->push("x");
    composer->push("x");
    composer->zen::composer::composer::push<zen::i32>(10, "int");
    composer->assign();
    composer->zen::composer::composer::push<zen::i32>(10, "int");
    composer->greater();
    composer->begin_if_then();
    // // composer->call(std::to_string(zen::placeholder), 0, zen::composer::call_result::pushed);
    composer->close_branch();
    composer->push("x");
    composer->zen::composer::composer::push<zen::i32>(10, "int");
    composer->lower();
    composer->else_if_then();
    composer->close_branch();
    composer->push("x");
    composer->zen::composer::composer::push<zen::i32>(10, "int");
    composer->lower();
    composer->else_if_then();
    // composer->call(std::to_string(zen::placeholder), 0, zen::composer::call_result::pushed);
    // std::cout << __LINE__ << std::endl;
    composer->close_branch();
    // std::cout << __LINE__ << std::endl;
    composer->else_then();
    // composer->call(std::to_string(zen::placeholder), 0, zen::composer::call_result::pushed);
    // composer->call(std::to_string(zen::placeholder), 0, zen::composer::call_result::pushed);
    composer->end_if();
    composer->end();
    composer->bake();

    return 0;
    */
    zen_run(R"(main = {
    x : int = 0

    if (x > 0) {
        _ : unit = print("positive")
    } else if(x == 0){
        _ : unit = print("zero")
    } else {
        _ : unit = print("negative")
    }
})");

#endif
}
catch (std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
