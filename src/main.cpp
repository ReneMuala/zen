
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

EMSCRIPTEN_KEEPALIVE

bool zen_run(const char* entrance)
{
    if (entrance == nullptr)
    {
        fmt::println("[zen_run] called with null argument.");
        return false;
    }
    fmt::println("[run not implemented.]");
    return true;
}

EMSCRIPTEN_KEEPALIVE

bool zen_compile(const char* code)
try
{
    if (code == nullptr)
    {
        fmt::println("[zen_compile] called with null argument.");
        return false;
    }

    tokens.clear();
    ILC::chain.clear();
    std::stringstream stream0;
    stream0.str(code);
    zen::lexer lexer(stream0);
    while (auto token = lexer.next())
    {
        // fmt::println(">> {}: '{}'", static_cast<int>(token->type), token->value);
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    ILC::chain_size = ILC::chain.size();
    if (parse())
    {
        get_composer()->bake();
    }
    else
    {
        std::cout << "Failed " << std::endl;
    }
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

void test_vm()
{
    using namespace zen;
    i64 ptr, ptr2, size = 16;
    i64 a = 1, b = 2, aptr = 0, bptr = 0, fsize = 8;
    std::vector<i64> code;
    code = {
        allocate, vm::ref(ptr), vm::ref(size),
        refer, vm::ref(aptr), vm::ref(a),
        refer, vm::ref(bptr), vm::ref(b),
        copy, vm::ref(ptr), vm::ref(aptr), vm::ref(fsize),
        add_i64, vm::ref(ptr2), vm::ref(ptr), vm::ref(fsize),
        copy, vm::ref(ptr2), vm::ref(bptr), vm::ref(fsize),
    };
    zen::vm vm;
    vm.load(code);
    vm.run();
    auto it = (demo_struct_string*)ptr;
    fmt::println("demo_struct_string {{ .a = {}, .b = {} }}\n", it->a, it->b);
    // fmt::println("a -> {} {}", (i64)&a, aptr);

    // for (int i = 0 ; i < size ; i++)
    // {
    //     fmt::print("[{}]({},", i, text[i]);
    //     fmt::print("{})", ((const char*)ptr)[i]);
    // }
    // std::cout << std::endl;
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

inline void setup_integration_test(const std::string& code, zen::composer::composer* composer)
{
    composer->reset();
    setup_parser_test(code);
}

int main(int argc, char** argv) try
{
    // test_vm();
    // return 0;
#ifdef KAIZEN_WASM
return 0;
#else
    if (true)
    {
        // zen::composer::composer* composer = get_composer();
        fmt::println("starting");
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


        composer->begin("main");
        composer->begin_for();
        composer->set_local("i", "int");
        composer->push("i");
        composer->zen::composer::composer::push<zen::i32>(1, "int");
        composer->zen::composer::composer::push<zen::i32>(3, "int");
        composer->set_for_begin_end();
        // composer->push("")
        // composer->set_local("x", "int");
        composer->zen::composer::composer::push<zen::types::heap::string*>(zen::types::heap::string::from_string("hello world\n"), "string");
        composer->call("print", 1, zen::composer::call_result::pushed);
        composer->end_for();
        composer->end();
        composer->link();


        // setup_parser_test(R"(
        // main = {
        //     for(i: int = 1, 5){
        //     }
        // })");
        // auto t0 = std::chrono::system_clock::now();
        // parse();
        // auto t01 = std::chrono::system_clock::now();
        composer->bake();
        const std::list<zen::composer::vm::function> main_functions = composer->functions["main"];
        if (main_functions.empty())
        {
            fmt::print("main functions empty\n");
            return 0;
        }
        const zen::composer::vm::function main = main_functions.front();
        zen::vm::stack stack;
        stack.push<zen::i64>(0); // returning address
        zen::vm vm1;
        vm1.load(composer->code);
        // auto t1 = std::chrono::system_clock::now();
        fmt::println("running");
        vm1.run(stack, main.address);
        // auto t2 = std::chrono::system_clock::now();
        // fmt::println("----------------\n1.compiled in {} ms \n2.ran in {} ms\n----------------",
        //              (long long)std::chrono::duration_cast<std::chrono::milliseconds>(t01 - t0).count(),
        //              (long long)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
        return 0;
    }
    // #define  NATIVE

    std::stringstream stream0;
    stream0.str(R"(
sum_using_for(x: int, y: int) = int(result) {
    result: int = 0
    for(i:int = x, y){
        result = result + i
    }
}
/*
duplicate(x: int) = int(x * 2)

callDuplicate() = int {
    duplicate(1)
}

one() = long {
    1l
}

sum(x: long, y: long) = long(x+y)

callSum() = {
    result: long = sum(100l,200l)
}

magic(x: long, y: long) = long(result) {
    sum: long = x + y
    sub: long = x - y
    result: long = (sum * sub + sum / sub) * sum * sum / 2l + 3l * sub
}

printSum(x: long, y: long) = {
//    print(sum(x,y))
}

main() = {
    x : double = 10.0
    // x
}

intToDouble(x: int) = double(r) {
    r: double = double(10i)
}

sumsub(x: int, y: int) = float {
    float(x + y)
}

roo(a: int, b: float, c: byte, d: long, e: short, f: double) = short{
    x: int = 1
    y: int = 2
    x = y
    z: short = short(3)
    z = short(4)
    short(a + int(b))
}

test_asgn(y: int) = {
    x: int = int(0f)
}
add(a: int, b: int, c: int) = int { a + b + c }
add2(a: int, b: int, c: int) =  int { a + b }
*/
)");

    zen::lexer lexer(stream0);
    while (auto token = lexer.next())
    {
        // fmt::println(">> {}: '{}'", static_cast<int>(token->type), token->value);
        ILC::chain.push_back(token->type);
        tokens.emplace_back(token.value());
    }
    ILC::chain_size = ILC::chain.size();
    if (parse())
    {
        get_composer()->bake();
    }
    else
    {
        std::cout << "Failed " << std::endl;
    }
    return 0;
#endif
}
catch (std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
