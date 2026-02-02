//
// Created by dte on 12/16/2025.
//

#include "test.hpp"

#include "builder/table.hpp"

namespace zen::libraries::test
{
    inline std::shared_ptr<builder::function> create_test_all(utils::constant_pool& pool,
                                                              std::shared_ptr<builder::program> program)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "runGlobalTests");
        const auto tab = zen::builder::table::create(fn);
        auto print_function = fn->create("print", {zen::builder::function::_string()}, nullptr);
        auto println_function = fn->create("println", {zen::builder::function::_string()}, nullptr);
        auto string_function = fn->create("string", {zen::builder::function::_long()},
                                          zen::builder::function::_string());
        fn->set_return(zen::builder::function::_bool());
        fn->move(fn->ret, fn->constant<zen::boolean>(true));
        const auto passed = fn->set_local(zen::builder::function::_long(), "passed");
        const auto failed = fn->set_local(zen::builder::function::_long(), "failed");
        // fn->move(passed, fn->constant<i64>(0));
        // fn->move(failed, fn->constant<i64>(0));
        for (const auto [id, lib] : program->libraries)
        {
            for (const auto test : lib->tests)
            {
                const auto test_name = fmt::format("{}:{}", lib->name, test->name);
                if (const auto result = fn->call(println_function, {
                                                     fn->constant<std::string>(fmt::format("[ RUN {} ]", test_name))
                                                 }); not result.
                    has_value())
                {
                    throw exceptions::semantic_error(result.error(), fn->offset);
                }

                if (auto result = fn->call(test, {}); result.has_value())
                {
                    fn->branch(zen::builder::scope::in_if, result.value(), [&](auto&, auto& pel, auto& pen)
                    {
                        fn->add(passed, passed, fn->constant<i64>(1));
                        if (const auto result = fn->call(println_function, {
                                                             fn->constant<std::string>(
                                                                 fmt::format("[ OK {} ]", test_name))
                                                         }); not result.
                            has_value())
                        {
                            throw exceptions::semantic_error(result.error(), fn->offset);
                        }
                        fn->branch(zen::builder::scope::in_else, nullptr, [&](auto&, auto& pel, auto& pen)
                        {
                            fn->add(failed, failed, fn->constant<i64>(1));
                            if (const auto result = fn->call(println_function, {
                                                                 fn->constant<std::string>(
                                                                     fmt::format("[ FAILED {} ]", test_name))
                                                             }); not result.
                                has_value())
                            {
                                throw exceptions::semantic_error(result.error(), fn->offset);
                            }
                        }, pel, pen);
                    });
                }
            }
        }
        fn->equal(fn->ret, failed, fn->constant<i64>(0));
        fn->branch(zen::builder::scope::in_if, fn->ret, [&](auto&, auto& pel, auto& pen)
        {
            if (const auto result = fn->call(println_function, {
                                                 fn->constant<std::string>("ALL TESTS PASSED")
                                             }); not result.
                has_value())
            {
                throw exceptions::semantic_error(result.error(), fn->offset);
            }
            fn->branch(zen::builder::scope::in_else, nullptr, [&](auto&, auto& pel, auto& pen)
            {
                if (const auto cast_result = fn->call(string_function, {
                                                          failed
                                                      }); not cast_result.
                    has_value())
                {
                    throw exceptions::semantic_error(cast_result.error(), fn->offset);
                }
                else
                {
                    if (const auto result = fn->call(print_function, {
                                                         cast_result.value()
                                                     }); not result.
                        has_value())
                    {
                        throw exceptions::semantic_error(result.error(), fn->offset);
                    }
                    if (const auto result = fn->call(println_function, {
                                                         fn->constant<std::string>(" FAILED TEST(S)")
                                                     }); not result.
                        has_value())
                    {
                        throw exceptions::semantic_error(result.error(), fn->offset);
                    }
                }
            }, pel, pen);
        });
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    inline std::shared_ptr<builder::function> create_test_this(utils::constant_pool& pool,
                                                               std::shared_ptr<zen::builder::library> lib)
    {
        const auto fn = zen::builder::function::create(pool, 0, false, "runLocalTests");
        const auto tab = zen::builder::table::create(fn);
        auto print_function = fn->create("print", {zen::builder::function::_string()}, nullptr);
        auto println_function = fn->create("println", {zen::builder::function::_string()}, nullptr);
        auto string_function = fn->create("string", {zen::builder::function::_long()},
                                          zen::builder::function::_string());
        fn->set_return(zen::builder::function::_bool());
        fn->move(fn->ret, fn->constant<zen::boolean>(true));
        const auto passed = fn->set_local(zen::builder::function::_long(), "passed");
        const auto failed = fn->set_local(zen::builder::function::_long(), "failed");
        // fn->move(passed, fn->constant<i64>(0));
        // fn->move(failed, fn->constant<i64>(0));
        for (const auto test : lib->tests)
        {
            const auto test_name = fmt::format("{}:{}", lib->name, test->name);
            if (const auto result = fn->call(println_function, {
                                                 fn->constant<std::string>(fmt::format("[ RUN {} ]", test_name))
                                             }); not result.
                has_value())
            {
                throw exceptions::semantic_error(result.error(), fn->offset);
            }

            if (auto result = fn->call(test, {}); result.has_value())
            {
                fn->branch(zen::builder::scope::in_if, result.value(), [&](auto&, auto& pel, auto& pen)
                {
                    fn->add(passed, passed, fn->constant<i64>(1));
                    if (const auto result = fn->call(println_function, {
                                                         fn->constant<std::string>(
                                                             fmt::format("[ OK {} ]", test_name))
                                                     }); not result.
                        has_value())
                    {
                        throw exceptions::semantic_error(result.error(), fn->offset);
                    }
                    fn->branch(zen::builder::scope::in_else, nullptr, [&](auto&, auto& pel, auto& pen)
                    {
                        fn->add(failed, failed, fn->constant<i64>(1));
                        if (const auto result = fn->call(println_function, {
                                                             fn->constant<std::string>(
                                                                 fmt::format("[ FAILED {} ]", test_name))
                                                         }); not result.
                            has_value())
                        {
                            throw exceptions::semantic_error(result.error(), fn->offset);
                        }
                    }, pel, pen);
                });
            }
        }
        fn->equal(fn->ret, failed, fn->constant<i64>(0));
        fn->branch(zen::builder::scope::in_if, fn->ret, [&](auto&, auto& pel, auto& pen)
        {
            if (const auto result = fn->call(println_function, {
                                                 fn->constant<std::string>("ALL TESTS IN FILE PASSED")
                                             }); not result.
                has_value())
            {
                throw exceptions::semantic_error(result.error(), fn->offset);
            }
            fn->branch(zen::builder::scope::in_else, nullptr, [&](auto&, auto& , auto&)
            {
                if (const auto cast_result = fn->call(string_function, {
                                                          failed
                                                      }); not cast_result.
                    has_value())
                {
                    throw exceptions::semantic_error(cast_result.error(), fn->offset);
                }
                else
                {
                    if (const auto result = fn->call(print_function, {
                                                         cast_result.value()
                                                     }); not result.
                        has_value())
                    {
                        throw exceptions::semantic_error(result.error(), fn->offset);
                    }
                    if (const auto result = fn->call(println_function, {
                                                         fn->constant<std::string>(" FAILED TEST(S)")
                                                     }); not result.
                        has_value())
                    {
                        throw exceptions::semantic_error(result.error(), fn->offset);
                    }
                }
            }, pel, pen);
        });
        fn->return_implicitly();
        fn->build();
        return fn;
    }

    std::shared_ptr<zen::builder::library> create(utils::constant_pool& pool, std::shared_ptr<zen::builder::library> lib, std::shared_ptr<zen::builder::program> program)
    {
        std::shared_ptr<zen::builder::library> library = zen::builder::library::create("test");
        library->add(create_test_all(pool, program));
        library->add(create_test_this(pool, lib));
        return library;
    }
}
