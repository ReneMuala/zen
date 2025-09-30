
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

std::vector<zen::token> tokens;

extern "C" {
EMSCRIPTEN_KEEPALIVE
void zen_sum(int a, int b){
    fmt::print("{} + {} = {}\n", a, b, a+b);
}

void EMSCRIPTEN_KEEPALIVE zen_reset(){
    get_composer()->reset();
}

EMSCRIPTEN_KEEPALIVE
bool zen_run(const char* entrance)
{
    if (entrance == nullptr){
        fmt::println("[zen_run] called with null argument.");
        return false;
    }
    fmt::println("[run not implemented.]");
    return true;
}

EMSCRIPTEN_KEEPALIVE
bool zen_compile(const char* code)
try {
    if (code == nullptr){
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
} catch (std::exception& e){
    std::cout << e.what() << std::endl;
    return false;
}
}

void* make_zen_string(const std::string & str)
{
    void * data = malloc(sizeof(char) * str.size() + sizeof(zen::i64));
    *static_cast<zen::i64*>(data) = str.length();
    memcpy((char*)(data) + sizeof(zen::i64), str.data(), str.length());
    return data;
}

void print_zen_string(void * str)
{
    zen::i64 len = *static_cast<zen::i64*>(str);
    zen::i64 i= 0;
    while (i < len)
    {
        printf("%c", *(char*)((char*)str + sizeof(zen::i64) + i++));
    }
    // printf("(%d bytes)", len);
}

int main(int argc, char** argv) try
{
#ifdef KAIZEN_WASM
return 0;
#else
    if (true)
    {

        zen::composer::composer* composer = get_composer();

        composer->begin("zenDestructor");
        composer->set_parameter("it", "string::noDestructor");
        composer->push("it.data");
        composer->push("bool");
        composer->call("bool", -1);
        composer->begin_if_then();
        composer->push("it.data");
        composer->call(std::to_string(zen::deallocate), 1);
        composer->end_if();
        composer->push("it");
        composer->call(std::to_string(zen::deallocate), 1);
        composer->end();

        composer->begin("zenCopy");
        composer->set_parameter("to", "string");
        composer->set_parameter("from", "string");

        composer->push("to.data");
        composer->push("bool");
        composer->call("bool", -1);
        composer->begin_if_then();
        composer->push("to.data");
        composer->call(std::to_string(zen::deallocate), 1);
        composer->end_if();

        composer->set_local("data", "long");
        composer->push("data");
        composer->push("from.len");
        composer->call(std::to_string(zen::allocate), 2);

        composer->push("data");
        composer->push("from.data");
        composer->push("from.len");
        composer->call(std::to_string(zen::copy), 3);

        composer->push("to.data");
        composer->push("data");
        composer->assign();

        composer->push("to.len");
        composer->push("from.len");
        composer->assign();
        composer->end();

        composer->begin("zenConstructor");
        composer->set_return_type("string");
        composer->set_local("new", "string::noConstructor");
        composer->push("new");
        composer->push<zen::i64>(composer->get_type("string")->get_full_size(), "long");
        composer->call(std::to_string(zen::allocate), 2);
        composer->push("new");
        composer->push<zen::types::heap::string*>(zen::types::heap::string::empty(), "string");
        composer->assign();
        composer->push("new.len");
        composer->push<zen::i64>(0, "long");
        composer->assign();
        composer->push("new.data");
        composer->push<zen::i64>(0, "long");
        composer->assign();
        composer->push("new");
        composer->return_value();
        composer->end();

        composer->begin("stringTest");
        composer->set_local("name", "string");
        composer->set_local("count", "short");
        // composer->push("name");
        // composer->push<zen::types::heap::string*>(zen::types::heap::string::from_string("zen"), "string");
        // composer->assign();
        composer->end();
        composer->bake();
        return 0;
        composer->begin("scope_test");
        composer->set_local("result", "int");
        composer->set_local("x", "int");
        composer->set_local("y", "int");
        composer->begin_while();
        composer->push<zen::boolean>(true, "bool");
        composer->set_while_condition();
        composer->set_local("result", "int");
        composer->set_local("x", "int");
        composer->set_local("y", "int");
        composer->push("result");
        composer->push("x");
        composer->push("y");
        composer->plus();
        composer->assign();
        composer->end_while();
        composer->push("result");
        composer->push("x");
        composer->push("y");
        composer->plus();
        composer->assign();
        composer->end();
        composer->bake();

        composer->begin("sum_using_reverse");
        composer->set_return_type("int");
        composer->set_return_name("result");
        composer->set_parameter("begin", "int");
        composer->set_parameter("end", "int");
        composer->set_local("result", "int");
        composer->push("result");
        composer->push<zen::i32>(0, "int");
        composer->assign();
        composer->begin_for();
        composer->set_local("i", "int");
        composer->push("i");
        composer->push("end");
        composer->push("begin");
        composer->push<zen::i32>(-1, "int");
        composer->set_for_begin_end_step();
        composer->push("result");
        composer->push("result");
        composer->push("i");
        composer->plus();
        composer->assign();
        composer->end_for();
        composer->end();
        composer->bake();

        composer->begin("sum_using_for");
        composer->set_return_type("int");
        composer->set_return_name("result");
        composer->set_parameter("begin", "int");
        composer->set_parameter("end", "int");
        composer->set_local("result", "int");
        composer->push("result");
        composer->push<zen::i32>(0, "int");
        composer->assign();
        composer->begin_for();
        composer->set_local("i", "int");
        composer->push("i");
        composer->push("begin");
        composer->push("end");
        composer->set_for_begin_end();
        composer->push("result");
        composer->push("result");
        composer->push("i");
        composer->plus();
        composer->assign();
        composer->end_for();
        composer->end();
        composer->bake();

        composer->begin("sum_using_while");
        composer->set_return_type("int");
        composer->set_return_name("result");
        composer->set_parameter("begin", "int");
        composer->set_parameter("end", "int");
        composer->set_local("result", "int");
        composer->push("result");
        composer->push<zen::i32>(0, "int");
        composer->assign();
        composer->begin_while();
        composer->push("begin");
        composer->push("end");
        composer->lower_or_equal();
        composer->set_while_condition();
        composer->push("result");
        composer->push("result");
        composer->push("begin");
        composer->post_increment();
        composer->plus();
        composer->end_while();
        composer->end();

        composer->begin("ternary_a_or_b");
        composer->set_return_type("int");
        composer->set_parameter("a", "int");
        composer->set_parameter("b", "int");
        composer->set_parameter("c", "bool");
        composer->push("c");
        composer->push("a");
        composer->push("b");
        composer->ternary();
        composer->return_value();
        composer->end();

        composer->begin("x_if_cond_or_y");
        composer->set_return_type("int");
        composer->set_parameter("cond", "bool");
        composer->set_parameter("x", "int");
        composer->set_parameter("y", "int");
        composer->push("cond");
        composer->begin_if_then();
        composer->push("x");
        composer->return_value();
        composer->else_then();
        composer->push("y");
        composer->return_value();
        composer->end_if();
        composer->end();

        composer->begin("write_string");
        composer->set_parameter("fd", "long");
        composer->set_parameter("str", "string");
        composer->push("str.data");
        composer->push("str.len");
        composer->push("fd");
        composer->call(std::to_string(zen::write_str), 3);
        composer->end();

        composer->begin("print_string");
        composer->set_parameter("string", "string");
        composer->push("write_string");
        composer->push<zen::i64>(reinterpret_cast<zen::i64>(stdout), "long");
        composer->push("string");
        composer->call("write_string", 2);
        composer->end();

        composer->begin("test::print_string");
        composer->push("print_string");
        composer->push<zen::types::heap::string*>(zen::types::heap::string::from_string("hello world"), "string");
        composer->call("print_string", 1);
        composer->end();

        composer->begin("internal::1_param_test");
        composer->set_return_type("double");
        composer->set_parameter("x", "double");
        composer->push("x");
        composer->return_value();
        composer->end();

        composer->begin("internal::0_param_test");
        composer->set_return_type("double");
        composer->push<double>(0.0, "double");
        composer->return_value();
        composer->end();

        composer->begin("internal::call_test");
        composer->push("internal::1_param_test");
        composer->push<double>(1.0, "double");
        composer->call("internal::1_param_test", 1);
        composer->end();

        composer->begin("internal::cast_test");
        composer->set_local("x", "double");
        composer->push("x");
        composer->push(20, "int");
        composer->call("double", 1);
        composer->end();

        composer->begin("internal::float_to_int");
        composer->set_return_type("int");
        composer->set_parameter("x", "float");
        composer->push("x");
        composer->call("int", -1);
        composer->return_value();
        composer->end();

        composer->begin("internal::sum_ints_as_doubles");
        composer->set_return_type("double");
        composer->set_parameter("x", "int");
        composer->set_parameter("y", "int");
        composer->push("<double>");
        composer->push("x");
        composer->call("double", -1);
        composer->push("<double>");
        composer->push("y");
        composer->call("double", -1);
        composer->plus();
        composer->return_value();
        composer->end();

        composer->begin("internal::sum");
        composer->set_return_type("long");
        composer->set_parameter("x", "long");
        composer->set_parameter("y", "long");
        composer->push("x");
        composer->push("y");
        composer->plus();
        composer->return_value();
        composer->end();

        composer->begin("internal::timesTwo");
        composer->set_return_type("double");
        composer->set_parameter("x", "double");
        composer->push(2.0, "double");
        composer->push("x");
        composer->times();
        composer->return_value();
        composer->end();

        composer->begin("internal::timesThree");
        composer->set_return_type("long"); // most x
        composer->set_return_name("result");
        composer->set_parameter("x", "long"); // most x
        composer->set_local("result", "long"); // most x
        composer->push("result");
        composer->push(3, "long");
        composer->push("x");
        composer->times(); // most x
        composer->assign();
        composer->end();
        composer->bake();
    }
    // #define  NATIVE

    std::stringstream stream0;
    stream0.str(R"(

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