
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


/*

void test_vm_ref()
{
    using namespace zen;
    vm vm1;
    struct _person_t
    {
        int age;
        const char* name;
    };

    _person_t p = {
        .age = 20,
        .name = "Descartes"
    };

    std::vector<i64> code = {
        most,-8,
        i64_to_i64, -8, vm::ref(p),
        mod_ptr,-8, 3,
    };
}
void test_vm()
{
    using namespace zen;
    vm vm1;
    i64 a = 10, b = 20, sum = 0, sub, mul, div, mod;
    f64 c = 3.14, d = 5.67, sum_f, sub_f, mul_f, div_f;
    boolean a_gt_b, a_lt_b, a_gte_b, a_lte_b, a_eq_b, a_neq_b;
    boolean c_gt_d, c_lt_d, c_gte_d, c_lte_d, c_eq_d, c_neq_d;
    boolean a_gt_b_and_a_lt_b, a_gt_b_or_a_lt_b, not_a_gt_b_or_a_lt_b;
    i64 a_gt_b_i64;
    f64 a_lt_b_f64;
    i64 e = 0;
    f64 f = 1.0;
    boolean boolean_e, boolean_f;
    i64 g = 1, h = 2, i = 3, j, k, l;
    i64 sum2, begin = 1, end = 5, acc, acc2;
    boolean is_i_lt_end;
    i64 v_2 = 2, v_minus_14 = -14;
    i64 m = 10, n = 15, o;
    i64 callSumAccAddress, callSumParamAddress, callSumParamResult;
    std::vector<i64> code = {
        inc_i64, vm::ref(a),
        dec_i64, vm::ref(a),
        add_i64, vm::ref(sum), vm::ref(a), vm::ref(b),
        i64_to_i64, vm::ref(sum), vm::ref(sum),
        sub_i64, vm::ref(sub), vm::ref(a), vm::ref(b),
        mul_i64, vm::ref(mul), vm::ref(a), vm::ref(b),
        div_i64, vm::ref(div), vm::ref(a), vm::ref(b),
        mod_i64, vm::ref(mod), vm::ref(a), vm::ref(b),
        i64_to_f64, vm::ref(c), vm::ref(a),
        i64_to_f64, vm::ref(d), vm::ref(b),
        inc_f64, vm::ref(c),
        dec_f64, vm::ref(c),
        add_f64, vm::ref(sum_f), vm::ref(c), vm::ref(d),
        f64_to_f64, vm::ref(sum_f), vm::ref(sum_f),
        sub_f64, vm::ref(sub_f), vm::ref(c), vm::ref(d),
        mul_f64, vm::ref(mul_f), vm::ref(c), vm::ref(d),
        div_f64, vm::ref(div_f), vm::ref(c), vm::ref(d),
        gt_i64,     vm::ref(a_gt_b),    vm::ref(a), vm::ref(b),
        lt_i64,     vm::ref(a_lt_b),    vm::ref(a), vm::ref(b),
        gte_i64,    vm::ref(a_gte_b),   vm::ref(a), vm::ref(b),
        lte_i64,    vm::ref(a_lte_b),   vm::ref(a), vm::ref(b),
        eq_i64,     vm::ref(a_eq_b),    vm::ref(a), vm::ref(b),
        neq_i64,    vm::ref(a_neq_b),   vm::ref(a), vm::ref(b),
        gt_f64,     vm::ref(c_gt_d),    vm::ref(c), vm::ref(d),
        lt_f64,     vm::ref(c_lt_d),    vm::ref(c), vm::ref(d),
        gte_f64,    vm::ref(c_gte_d),   vm::ref(c), vm::ref(d),
        lte_f64,    vm::ref(c_lte_d),   vm::ref(c), vm::ref(d),
        eq_f64,     vm::ref(c_eq_d),    vm::ref(c), vm::ref(d),
        neq_f64,    vm::ref(c_neq_d),   vm::ref(c), vm::ref(d),
        boolean_and, vm::ref(a_gt_b_and_a_lt_b), vm::ref(a_gt_b), vm::ref(a_lt_b),
        boolean_or, vm::ref(a_gt_b_or_a_lt_b), vm::ref(a_gt_b), vm::ref(a_lt_b),
        boolean_not, vm::ref(not_a_gt_b_or_a_lt_b), vm::ref(a_gt_b_or_a_lt_b),
        boolean_to_i64, vm::ref(a_gt_b_i64), vm::ref(a_gt_b),
        boolean_to_f64, vm::ref(a_lt_b_f64), vm::ref(a_lt_b),
        i64_to_boolean, vm::ref(boolean_e), vm::ref(e),
        f64_to_boolean, vm::ref(boolean_f), vm::ref(f),
        push, vm::ref(g),
        push, vm::ref(h),
        push, vm::ref(i),
        pop, vm::ref(l),
        pop, vm::ref(k),
        pop, vm::ref(j),
        most, -24,
        i64_to_i64, -8, vm::ref(a),
        i64_to_i64, -16, vm::ref(b),
        add_i64,    -24, -8, -16,
        i64_to_i64, vm::ref(sum2), -24,
        most, +24,
        bit_xor, vm::ref(acc), vm::ref(acc), vm::ref(acc),
        i64_to_i64, vm::ref(i), vm::ref(begin),
        add_i64, vm::ref(acc), vm::ref(acc), vm::ref(i),
        lt_i64, vm::ref(is_i_lt_end), vm::ref(i), vm::ref(end),
        inc_i64, vm::ref(i),
        go, +2,
        hlt,
        go_if, vm::ref(is_i_lt_end), -14,
        bit_xor, vm::ref(acc2), vm::ref(acc2), vm::ref(acc2),
        i64_to_i64, vm::ref(i), vm::ref(begin),
        add_i64, vm::ref(acc2), vm::ref(acc2), vm::ref(i),
        lt_i64, vm::ref(is_i_lt_end), vm::ref(i), vm::ref(end),
        inc_i64, vm::ref(i),
        jump, vm::ref(v_2),
        hlt,
        jump_if, vm::ref(is_i_lt_end), vm::ref(v_minus_14),
        bit_xor, vm::ref(o), vm::ref(o), vm::ref(o),
        call, vm::ref(callSumAccAddress),
        call, vm::ref(callSumAccAddress),
        call, vm::ref(callSumAccAddress),
        most, -8,
        push, vm::ref(a),
        push, vm::ref(b),
        call, vm::ref(callSumParamAddress),
        i64_to_i64, vm::ref(callSumParamResult), -24,
        most, +8+16,
        hlt,
        most, -8,
        i64_to_i64, -8, vm::ref(o),
        add_i64, vm::ref(o), vm::ref(m), vm::ref(n),
        add_i64, vm::ref(o), vm::ref(o), -8,
        most, +8,
        ret,
        add_i64, -32, -24, -16,
        ret,
        hlt,
    };
    callSumAccAddress = 226;
    callSumParamAddress = 242;
    // fmt::println("placeholder: {}", std::find(code.begin(), code.end(), placeholder) - code.begin());
    // fmt::println("code {}", code[callSumAccAddress]);
    // return;
    vm1.load(code);
    vm1.run();
    fmt::println(zen::ascii_art);
    fmt::println(">> arith i64");
    fmt::println("{} + {} = {}", a, b, sum);
    fmt::println("{} - {} = {}", a, b, sub);
    fmt::println("{} * {} = {}", a, b, mul);
    fmt::println("{} / {} = {}", a, b, div);
    fmt::println("{} % {} = {}", a, b, mod);
    fmt::println(">> arith f64");
    fmt::println("{} + {} = {}", c, d, sum_f);
    fmt::println("{} - {} = {}", c, d, sub_f);
    fmt::println("{} * {} = {}", c, d, mul_f);
    fmt::println("{} / {} = {}", c, d, div_f);
    fmt::println(">> cmp i64");
    fmt::println("{} > {} = {}", a, b, a_gt_b);
    fmt::println("{} < {} = {}", a, b, a_lt_b);
    fmt::println("{} >= {} = {}", a, b, a_gte_b);
    fmt::println("{} <= {} = {}", a, b, a_lte_b);
    fmt::println("{} == {} = {}", a, b, a_eq_b);
    fmt::println("{} != {} = {}", a, b, a_neq_b);
    fmt::println(">> cmp f64");
    fmt::println("{} > {} = {}", c, d, c_gt_d);
    fmt::println("{} < {} = {}", c, d, c_lt_d);
    fmt::println("{} >= {} = {}", c, d, c_gte_d);
    fmt::println("{} <= {} = {}", c, d, c_lte_d);
    fmt::println("{} == {} = {}", c, d, c_eq_d);
    fmt::println("{} != {} = {}", c, d, c_neq_d);
    fmt::println(">> boolean logic");
    fmt::println("{0} > {1} && {0} < {1}  = {2}", a, b, a_gt_b_and_a_lt_b);
    fmt::println("{0} > {1} || {0} < {1}  = {2}", a, b, a_gt_b_or_a_lt_b);
    fmt::println("!({0} > {1} || {0} < {1})  = {2}", a, b, not_a_gt_b_or_a_lt_b);
    fmt::println("{0} > {1} as i64 = {2}", a, b, a_gt_b_i64);
    fmt::println("{0} < {1} as f64 = {2}", a, b, a_lt_b_f64);
    fmt::println("i32({0}) as boolean = {1}", e, boolean_e);
    fmt::println("f32({0}) as boolean = {1}", f, boolean_f);
    fmt::println(">> stack");
    fmt::println("1 = {}", j);
    fmt::println("2 = {}", k);
    fmt::println("3 = {}", l);
    fmt::println("{} = {}", sum2, sum);
    fmt::println(">> go");
    fmt::println("1 + 2 + 3 + 4 + 5 = {}", acc, i);
    fmt::println(">> jump");
    fmt::println("1 + 2 + 3 + 4 + 5 = {}", acc2, i);
    fmt::println("sumAcc();sumAcc();sumAcc(); {}", o);
    fmt::println("sumParam({},{}) = {}", a,b, callSumParamResult);
}
*/

void test_stack()
{
    using namespace zen;
    vm::stack stack;
    stack -= 8;
    stack -= 8;
    stack -= 8;
    int* ptr = static_cast<int*>(stack - 4);
    *ptr = 10;
    std::cout << *ptr << std::endl;
    stack += 4;
    std::cout << stack.negative_stack_size << std::endl;
}

void test_constants()
{
    using namespace zen::utils;
    raii c(2);
    std::cout << c.as<int>() << std::endl;

    constant_pool pool;
    auto& name = pool.get("name");

    for (auto& [k,v] : pool.data)
    {
        std::cout << k << std::endl;
    }

    std::cout << (name.type_hash == typeid(char const*).hash_code()) << std::endl;
    // std::cout << (char*)(name_addr) << std::endl;
}

/*
#include <iostream>
#include <ffi.h>
extern "C" {
        int sum(int a, int b)
        {
            std::cout << "a : " << a << std::endl;
            std::cout << "b : " << b << std::endl;
            return a + b;
        }
    }

void test_ffi()
{
    ffi_cif cif;
    ffi_type * args[2];
    void * values[2];
    int a = 100, b = 101;
    args[0] = &ffi_type_sint;
    args[1] = &ffi_type_sint;
    values[0] = &a;
    values[1] = &b;
    int r;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 2, &ffi_type_sint, args) == FFI_OK)
    {
        ffi_call(&cif, (void(*)())sum, &r, values);
        std::cout << r << std::endl;
    }
}

void test_vm_ffi()
{
    using namespace zen;
    vm vm1;
    i64 a = 10, b = 20;
    if (vm1.register_ffi_callable(vm::ref(sum),
        std::vector {
            vm::ref(ffi_type_sint),
            vm::ref(ffi_type_sint),
        }, vm::ref(ffi_type_sint)))
    {
        std::cout << "vm1.register_callable(vm::ref(sum) done";
    }

    std::vector<i64> code = {
        push, vm::ref(a),
        push, vm::ref(a),
        push, vm::ref(b),
        zen::ffi_call, vm::ref(sum)
    };

    vm1.load(code);

}
*/
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

// struct return_data
// {
//     std::optional<zen::composer::value> value = std::nullopt;
//     std::optional<std::string> name = std::nullopt;
// } ;
struct function_scope1 : public zen::composer::vm::block_scope
{
    // return_data return_data = {};
    std::stack<zen::composer::vm::label> labels = {};
    // std::string name = {};
    // zen::i64 stack_usage = {};
    // std::map<std::string, zen::composer::symbol> locals = {};

    [[nodiscard]] bool is(const enum type& t) const override
    {
        return t & type::in_function;
    }

    ~function_scope1() override = default;
};


int main(int argc, char** argv) try
{
#ifdef KAIZEN_WASM
return 0;
#else
    if (true)
    {
        zen::composer::composer* composer = get_composer();

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

