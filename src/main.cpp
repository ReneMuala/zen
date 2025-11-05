
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
#include <float.h>
std::vector<zen::token> tokens;

extern "C" {
void EMSCRIPTEN_KEEPALIVE zen_sum(int a, int b)
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

void define_print_string(zen::composer::vm::composer* composer)
{
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
	composer->call(std::to_string(zen::write_str), 3);
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
}

KAIZEN_DEFINE_PRINT_FOR(byte, i8)
KAIZEN_DEFINE_PRINT_FOR(short, i16)
KAIZEN_DEFINE_PRINT_FOR(int, i32)
KAIZEN_DEFINE_PRINT_FOR(long, i64)
KAIZEN_DEFINE_PRINT_FOR(float, f32)
KAIZEN_DEFINE_PRINT_FOR(double, f64)

bool EMSCRIPTEN_KEEPALIVE zen_run(const char* code)
try
{
	if (code == nullptr)
	{
		fmt::println("[zen_compile] called with null argument.");
		return false;
	}
	zen::composer::vm::composer* composer = dynamic_cast<zen::composer::vm::composer*>(get_composer());
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
	setup_parser_test(std::string(code));
	parse();
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

inline void setup_integration_test(const std::string& code, zen::composer::composer* composer)
{
	composer->reset();
	setup_parser_test(code);
}

int main(int argc, char** argv) try
{
	zen_run(R"(
        sum(x:int, y:int) = int(x+y)
		println() = {
			print("\n")
		}

		println(string: string) = {
			print(string)
			print("\n")
		}

		print(number: int) = {
			if(number < 0) {
				print("-")
				number = -number
			}
			if(number >= 10){
				part: int = number % 10
				print(number / 10)
				print(part)
			} else{
				if(number == 1){
					print("1")
				} else if(number == 2) {
					print("2")
				} else if(number == 3) {
					print("3")
				} else if(number == 4) {
					print("4")
				} else if(number == 5) {
					print("5")
				} else if(number == 6) {
					print("6")
				} else if(number == 7) {
					print("7")
				} else if(number == 8) {
					print("8")
				} else if(number == 9) {
					print("9")
				} else {
					print("0")
				}
			}
		}

		println(int: int) = {
			print(int)
			print("\n")
		}

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
        main = {
			a: string = string(65b)
            print("Click 'Run' or hit CTR+R to execute your ZEN code. Output will appear here.")
        }
/*
	- float format
	- range clip
	- duration format
 */
    )");
}
catch (std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
