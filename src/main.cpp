
#include <iostream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

#include <vm/vm.hpp>
#include <sstream>
#include "utils/utils.hpp"
std::vector<zen::token> tokens;

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

void test_stack()
{
    using namespace zen;
    vm::stack stack;
    stack -= 8;
    stack -= 8;
    stack -= 8;
    int * ptr = static_cast<int*>(stack - 4);
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
    auto & name = pool.get("name");

    for (auto & [k,v] : pool.data)
    {
        std::cout << k << std::endl;
    }

    std::cout << ( name.type_hash == typeid(char const*).hash_code() )<< std::endl;
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
int main(int argc, char** argv) try
{
    // for (int i = 0; i < 10000; ++i)
    // {
    //     test_stack();
    // }
    // return 0;
    // test_vm();
    // test_constants();
    // test_ffi();
    // test_vm_ffi();
    // return 0;
#ifdef NATIVE
    std::string filename = "test.zen";
    std::ifstream stream0(filename);
    stream0.open(filename);
    if (not stream0.is_open())
    {
        throw zen::exceptions::file_not_found(filename);
    }
#else
    std::stringstream stream0;

    stream0.str(R"(
sub(x: long, y: long) = long(result) {
    result: long = x - y
}

sum(x: long, y: long) = long(x+y)

printSum(x: long, y: long) = {
//    print(sum(x,y))
}

main() = {
    x : double = 10
//    print(sum(10,20))
}
)");
#endif

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
        std::cout << "Success" << std::endl;
        get_composer().bake();
    }
    else
    {
        std::cout << "Failed " << std::endl;
    }
    return 0;
}
catch (std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
