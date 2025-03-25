![supernova](images/supernova.svg)

# Supernova
A generic jit library based on [Asmjit](https://github.com/asmjit/asmjit) with a focus on simplicity and performance.

## How it looks

> Simple program

```c++
#include "jit/function.hpp"

int main()
{
    using namespace supernova::jit;
    const auto rt = std::make_shared<asmjit::JitRuntime>();
    const std::shared_ptr<function_builder> builder = function_builder::create(rt, asmjit::FuncSignature::build<int>());
    auto arg0 = builder->i32(39);
    auto arg1 = builder->i32(40);
    auto result = builder->i32(1);

    const auto end = builder->label();
    builder->jump_equal(arg0, arg1, end);
    builder->move(result, builder->i32_const(0));
    builder->bind(end);
    builder->return_value(result);
    if (const auto func = builder->build<int()>())
    {
        int  r = func();
        fmt::println(__FUNCTION__": func() = {}", r);
        return 0;
    }
    return 1;
}
```


## Getting started

### Prerequisites

Depending on your operating system, you'll need the following tools to build and run Super Nova:

|   OS    |           Tools            |
|:-------:|:--------------------------:|
| Windows |    Visual Studio, XMAKE    |
|  *nix   | XMAKE and a c++23 compiler |

### Building 

To build the Super Nova VM and its associated tests, follow these steps:

1. Install the necessary prerequisites for your operating system.
2. Run `xmake build` in the project directory.
3. The compiled binaries, `supernova` (the VM) and `supernova-tests` (the tests), will be located in the `build/OS/ARCH/release` directory.
