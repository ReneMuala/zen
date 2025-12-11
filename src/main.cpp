
#include <iostream>
#include <memory>
#include <optional>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "parser/builder_parser.hpp"
#include "builder/function.hpp"
#include <vm/vm.hpp>
#include <sstream>
#include "utils/utils.hpp"

#ifdef KAIZEN_WASM
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif
#include <chrono>
#include <float.h>
std::vector<zen::token> tokens;

extern "C" {
void EMSCRIPTEN_KEEPALIVE zen_sum(int a, int b)
{
	fmt::print("{} + {} = {}\n", a, b, a + b);
}

inline void setup_parser(const std::shared_ptr<parser>& parser, const std::string& code)
{
	tokens.clear();
	std::stringstream stream(code);
	zen::lexer lexer(stream);
	while (auto token = lexer.next())
	{
		parser->chain.push_back(token->type);
		tokens.emplace_back(token.value());
	}
	parser->chain_size = parser->chain.size();
}

void define_print_string(zen::composer::vm::composer* composer)
{
	composer->begin("print");
	composer->set_parameter("string", "string");
	{
		composer->push("string.[data]");
		const auto deref = composer->dereference(composer->top());
		composer->pop();
		composer->push(deref);
	}
	{
		composer->push("string.[len]");
		const auto deref = composer->dereference(composer->top());
		composer->pop();
		composer->push(deref);
	}
	composer->zen::composer::composer::push<zen::i64>(reinterpret_cast<zen::i64>(stdout), "long");
	composer->call(std::to_string(zen::write_str), 3);
	composer->end();

	composer->begin("println");
	composer->set_parameter("string", "string");
	composer->push("string");
	composer->call("print", 1);
	composer->zen::composer::composer::push<zen::string*>(zen::string::from_string("\n"), "string");
	composer->call("print", 1);
	composer->end();

	composer->begin("println");
	composer->zen::composer::composer::push<zen::string*>(zen::string::from_string("\n"), "string");
	composer->call("print", 1);
	composer->end();
}

#define KAIZEN_DEFINE_PRINT_FOR(T,NT)\
	void define_print_##T(zen::composer::vm::composer* composer)\
{\
	composer->begin("print");\
	composer->set_parameter("x", #T);\
	composer->push("x");\
	composer->zen::composer::composer::push<zen::i64>(reinterpret_cast<zen::i64>(stdout), "long");\
	composer->call(std::to_string(zen::write_##NT), 2);\
	composer->end();\
	\
	composer->begin("println");\
	composer->set_parameter("x", #T);\
	composer->push("x");\
	composer->call("print", 1);\
	composer->zen::composer::composer::push<zen::string*>(zen::string::from_string("\n"), "string");\
	composer->call("print", 1);\
	composer->end();\
}

#define KAIZEN_DEFINE_PRINT_AS_STRING_FOR(T,NT)\
	void define_print_##T(zen::composer::vm::composer* composer)\
{\
	composer->begin("print");\
	composer->set_parameter("x", #T);\
	composer->push("x");\
	composer->call("string", 1);\
	composer->call("print", 1);\
	composer->end();\
	\
	composer->begin("println");\
	composer->set_parameter("x", #T);\
	composer->push("x");\
	composer->call("print", 1);\
	composer->zen::composer::composer::push<zen::string*>(zen::string::from_string("\n"), "string");\
	composer->call("print", 1);\
	composer->end();\
}

void define_print_bool(zen::composer::vm::composer* composer)
{
	composer->begin("print");
	composer->set_parameter("x", "bool");
	composer->push("x");
	composer->begin_if_then();
	composer->zen::composer::composer::push<zen::string*>(zen::string::from_string("true"), "string");
	composer->call("print", 1);
	composer->close_branch();
	composer->else_then();
	composer->zen::composer::composer::push<zen::string*>(zen::string::from_string("false"), "string");
	composer->call("print", 1);
	composer->end_if();
	composer->end();

	composer->begin("println");
	composer->set_parameter("x", "bool");
	composer->push("x");
	composer->call("print", 1);
	composer->zen::composer::composer::push<zen::string*>(zen::string::from_string("\n"), "string");
	composer->call("print", 1);
	composer->end();
}

KAIZEN_DEFINE_PRINT_FOR(byte, i8)
KAIZEN_DEFINE_PRINT_AS_STRING_FOR(short, i16)
KAIZEN_DEFINE_PRINT_AS_STRING_FOR(int, i32)
KAIZEN_DEFINE_PRINT_AS_STRING_FOR(long, i64)
KAIZEN_DEFINE_PRINT_AS_STRING_FOR(float, f32)
KAIZEN_DEFINE_PRINT_AS_STRING_FOR(double, f64)

bool EMSCRIPTEN_KEEPALIVE zen_run(const char* code)
try
{
	if (code == nullptr)
	{
		fmt::println("[zen_compile] called with null argument.");
		return false;
	}
	auto parser = parser::make();
	zen::composer::vm::composer* composer = static_cast<zen::composer::vm::composer*>(parser->composer.get());
	composer->reset();
	define_print_string(composer);
	define_print_bool(composer);
	define_print_byte(composer);
	define_print_short(composer);
	define_print_int(composer);
	define_print_long(composer);
	define_print_float(composer);
	define_print_double(composer);
	composer->link();
	setup_parser(parser, std::string(code));
	parser->parse();
	composer->bake();
	const std::list<zen::composer::vm::function> main_functions = composer->functions["main"];
	if (main_functions.empty())
	{
		fmt::print("[runtime error: main function not found]\n");
		return false;
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

void if_test()
{
	zen::utils::constant_pool pool;
	zen::i64 offset;
	const auto fb = zen::builder::function::create(pool, offset, true);
	const auto _b = fb->set_local(zen::builder::function::_bool(), "b");
	fb->branch(zen::builder::scope::in_if, _b, [&](auto b, auto pel, auto pen)
	{
		auto _i = b->set_local(zen::builder::function::_int(), "i");
		b->add(_i, _i, _i);
		fb->branch(zen::builder::scope::in_else, {}, [&](auto b, auto pel, auto pen)
		{
			auto _i = b->set_local(zen::builder::function::_long(), "i");
			b->add(_i, _i, _i);
		}, pel, pen);
	});
	auto _a = fb->set_local(zen::builder::function::_int(), "a");
	fb->add(_a, _a, _a);
	fb->build();
}

void if_else_if_else_test()
{
	zen::utils::constant_pool pool;
	zen::i64 offset;
	const auto fb = zen::builder::function::create(pool, offset, true);
	const auto _b = fb->set_local(zen::builder::function::_bool(), "b");
	fb->branch(zen::builder::scope::in_if, _b, [&](auto fb, auto pel, auto pen)
	{
		auto _i = fb->set_local(zen::builder::function::_int(), "i");
		fb->add(_i, _i, _i);
		fb->branch(zen::builder::scope::in_between, {}, [&](auto fb, auto pel, auto pen)
		{
			auto _b1 = fb->set_local(zen::builder::function::_bool(), "b1");
			fb->branch(zen::builder::scope::in_else_if, _b1, [&](auto fb, auto pel, auto pen)
			{
				auto _i = fb->set_local(zen::builder::function::_int(), "i");
				fb->add(_i, _i, _i);
				fb->branch(zen::builder::scope::in_between, {}, [&](auto fb, auto pel, auto pen)
				{
					auto _b1 = fb->set_local(zen::builder::function::_bool(), "b1");
					auto _i = fb->set_local(zen::builder::function::_int(), "i");
					fb->branch(zen::builder::scope::in_else_if, _b1, [&](auto fb, auto pel, auto pen)
					{
						auto _i = fb->set_local(zen::builder::function::_int(), "i");
						fb->add(_i, _i, _i);
						fb->branch(zen::builder::scope::in_between, {}, [&](auto fb, auto pel, auto pen)
						{
							auto _b1 = fb->set_local(zen::builder::function::_bool(), "b1");
							fb->branch(zen::builder::scope::in_else_if, _b1, [&](auto fb, auto pel, auto pen)
							{
								auto _i = fb->set_local(zen::builder::function::_int(), "i");
								fb->add(_i, _i, _i);
								fb->branch(zen::builder::scope::in_between, {}, [&](auto fb, auto pel, auto pen)
								{
									auto _b1 = fb->set_local(zen::builder::function::_bool(), "b1");
									auto _i = fb->set_local(zen::builder::function::_int(), "i");
									fb->branch(zen::builder::scope::in_else_if, _b1, [&](auto fb, auto pel, auto pen)
									{
										auto _i = fb->set_local(zen::builder::function::_int(), "i");
										fb->add(_i, _i, _i);
										fb->branch(zen::builder::scope::in_else, {}, [&](auto fb, auto pel, auto pen)
										{
											auto _i = fb->set_local(zen::builder::function::_long(), "i");
											fb->add(_i, _i, _i);
										}, pel, pen);
									}, pel, pen);
								}, pel, pen);
							}, pel, pen);
						}, pel, pen);
					}, pel, pen);
				}, pel, pen);
			}, pel, pen);
		}, pel, pen);
	});
}

void if_else_if_test()
{
	zen::utils::constant_pool pool;
	zen::i64 offset;
	const auto fb = zen::builder::function::create(pool, offset, true);
	const auto _b = fb->set_local(zen::builder::function::_bool(), "b");
	fb->branch(zen::builder::scope::in_if, _b, [&](auto fb, auto pel, auto pen)
	{
		auto _i = fb->set_local(zen::builder::function::_int(), "i");
		fb->add(_i, _i, _i);
		fb->branch(zen::builder::scope::in_between, {}, [&](auto fb, auto pel, auto pen)
		{
			auto _b1 = fb->set_local(zen::builder::function::_bool(), "b1");
			fb->branch(zen::builder::scope::in_else_if, _b1, [&](auto fb, auto pel, auto pen)
			{
				auto _i = fb->set_local(zen::builder::function::_int(), "i");
				fb->add(_i, _i, _i);
				fb->branch(zen::builder::scope::in_between, {}, [&](auto fb, auto pel, auto pen)
				{
					auto _b1 = fb->set_local(zen::builder::function::_bool(), "b1");
					fb->branch(zen::builder::scope::in_else_if, _b1, [&](auto fb, auto pel, auto pen)
					{
						auto _i = fb->set_local(zen::builder::function::_int(), "i");
						fb->add(_i, _i, _i);
					}, pel, pen);
				}, pel, pen);
			}, pel, pen);
		}, pel, pen);
	});
}

void for_test()
{
	zen::utils::constant_pool pool;
	zen::i64 offset;
	const auto fb = zen::builder::function::create(pool, offset, true);
	std::vector params = {
		fb->set_local(zen::builder::function::_int(), "it"),
		fb->set_local(zen::builder::function::_int(), "beg"),
		fb->set_local(zen::builder::function::_int(), "end")
	};
	fb->loop_for(params, [&](auto fb)
	{
		auto _sum = fb->set_local(zen::builder::function::_int(), "sum");
		fb->add(_sum, params.at(0), _sum);
		std::vector<std::shared_ptr<zen::builder::value>> sub_params = {
			fb->set_local(zen::builder::function::_int(), "sub_it"),
			fb->set_local(zen::builder::function::_int(), "sub_beg"),
			fb->set_local(zen::builder::function::_int(), "sub_end")
		};
		auto _sub_sum = fb->set_local(zen::builder::function::_int(), "sub_sum");
		fb->loop_for(sub_params,
		             [&](auto fb)
		             {
			             fb->add(_sub_sum, sub_params.at(0), _sub_sum);
		             });
	});
	// auto _a = fb->set_local(zen::builder::function::_int(), "a");
	// fb->add(_a, _a, _a);
	fb->build();
}

int main(int argc, char** argv) try
{
#ifdef KAIZEN_WASM
	zen_run(R"(
        sum(x:int, y:int) = int(x+y)

		rect(lines: int, cols: int) = {
			for(l: int = 1, lines){
				for(c: int = 1, cols){
					print("*")
				}
				println()
			}
		}

		divide(x: int, y: int) = int {
			if(y != 0){
				x/y
			} else {
				println("[detected division by 0]")
				0
			}
		}

		main2 = {
			_ : unit
			rows: int = 13
			for(i: int = 1,rows,2){
				for(j: int = (rows-i)/2, 1, -1){
					print(" ")
				}
				for(k: int = 1, i){
					print("*")
				}
				print("\n")
			}
		}
class point {
	x: double
	y: double
}

        main = {
		    print("Click 'Run' or hit CTR+R to execute your ZEN code. Output will appear here.")
        }
/*
	- float format
	- range clip
	- duration format
 */
    )");
#else
	zen::utils::constant_pool pool;
	zen::i64 offset;
	const auto fb = zen::builder::function::create(pool, offset, true);
	std::vector<std::shared_ptr<zen::builder::value>> params;
	auto _i = fb->set_local(zen::builder::function::_int(), "i");
	auto _j = fb->set_local(zen::builder::function::_int(), "j");

	fb->loop_while(params, [&](auto fb)
	               {
		               auto _c = fb->set_local(zen::builder::function::_bool(), "c");
		               fb->lower_equal(_c, _i, _j);
		               params.push_back(_c);
	               }, [&](auto fb)
	               {
		               auto _sum = fb->set_local(zen::builder::function::_int(), "sum");
		               fb->add(_sum, _i, _sum);
	               });
	// auto _a = fb->set_local(zen::builder::function::_int(), "a");
	// fb->add(_a, _a, _a);
	fb->build();
	// implement symbol manager
	// implement deference wrappers
#endif
}
catch (std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
