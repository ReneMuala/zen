#include <iostream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

#include <vm/vm.hpp>
std::vector<zen::token> tokens;

void test_vm()
{
    using namespace zen;
    vm vm1;
    vm::i64 a = 10, b = 20, sum = 0, sub, mul, div, mod;
    vm::f64 c = 3.14, d = 5.67, sum_f, sub_f, mul_f, div_f;
    vm::boolean a_gt_b, a_lt_b, a_gte_b, a_lte_b, a_eq_b, a_neq_b;
    vm::boolean c_gt_d, c_lt_d, c_gte_d, c_lte_d, c_eq_d, c_neq_d;
    vm::boolean a_gt_b_and_a_lt_b, a_gt_b_or_a_lt_b, not_a_gt_b_or_a_lt_b;
    vm::i64 a_gt_b_i64;
    vm::f64 a_lt_b_f64;
    vm::i64 e = 0;
    vm::f64 f = 1.0;
    vm::boolean boolean_e, boolean_f;
    vm::i64 g = 1, h = 2, i = 3, j, k, l;
    vm::i64 sum2, begin = 1, end = 5, acc;
    vm::boolean is_i_lt_end;
    std::vector<vm::i64> code = {
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
        hlt,
    };
    vm1.load(code);
    vm1.run();
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
    fmt::println(">> jmp");
    fmt::println("1 + 2 + 3 + 4 + 5 = {}", acc, i);
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

int main(int argc, char** argv) try
{
    // for (int i = 0; i < 10000; ++i)
    // {
    //     test_stack();
    // }
    // return 0;
    test_vm();
    return 0;
    zen::lexer lexer("main.zen");
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
