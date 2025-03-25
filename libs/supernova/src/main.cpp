#include "jit/function.hpp"


void playground(bool sum)
{
    using namespace supernova::jit;

    auto rt = std::make_shared<asmjit::JitRuntime>();
    auto builder = function_builder::create(rt, asmjit::FuncSignature::build<int, int, int>());
    auto a = builder->i32();
    auto b = builder->i32();
    auto c = builder->i32();
    builder->fetch_argument(0, a);
    builder->fetch_argument(1, b);
    if (sum)
        builder->add(c, a, b);
    else
        builder->sub(c, a, b);
    builder->return_value(c);

    auto func = builder->build<int(int, int)>();
    if (func)
    {
        int a = 40, b = 60;
        fmt::println("{}({},{}) = {}", sum ? "sum" : "sub", a,b, func(a,b));
    }
}

int main(int argc, char** argv)
try {
    playground(true);
    playground(false);
} catch (std::exception& e)
{
    fmt::println(stderr, "Uncaught exception:\n{}", e.what());
}