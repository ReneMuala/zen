
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

inline void setup_parser(const std::shared_ptr<parser>& parser,const std::string& code)
{
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
	setup_parser(parser,std::string(code));
	parser->parse();
	// composer->bake();
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

		_size(this: string) = long {
			this.len
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
zen_run(R"(
class person {
	name: string
	surname: string
	age: int
	registered: bool
	getAge = int {
		this.age
	}

	getName = string {
		this.name
	}

	getAge2 = int {
		age
	}

	displayName = {
		println(name)
	}

	greet = {
		println("Hi, im " + name)
		displayName()
		println(string(getAge()))
	}
}

sum(x: int, y: int) = int {
	x + y
}

person(name: string, age: int) = person {
	p: person
	p.name = name
	p.age = age
	p
}

test_value_equality = bool {
	1b == 1b &&
	1s == 1s &&
	1i == 1i &&
	1 == 1 &&
	1l == 1l &&
	1.5f == 1.5f &&
	3e-10f == 3e-10f &&
	3e-10d == 3e-10d &&
	3e-10 == 3e-10 &&
	3e10 == 3e+10 &&
	1.5d == 1.5d &&
	"1" == "1" &&
	person("Zendaya", 20) == person("Zendaya", 20) &&
	1 != 2 &&
	1s != 2s &&
	1i != 2i &&
	1 != 2 &&
	1l != 2l &&
	1.5f != 2.5f &&
	3e-10f != 3e10f &&
	3e-10d != 3e10d &&
	3e-10 != 3e10 &&
	3e-10 != 3e+10 &&
	1.5d != 2.5d &&
	"1" != "2" &&
	person("Zendaya", 20) != person("Zenia", 20)
}

class Point {
	x: double
	y: double
}

operator >(a: Point, b: Point) = bool {
	a.x > b.x && a.y > b.y
}

string(pt: Point) = string("")

main = {
	s: string
	s = "Hello World!"
	println(s)
	if(test_value_equality()){
		println("[test_value_equality: PASSED]")
	} else {
		println("[test_value_equality: FAILED]")
	}

	p: person = person("Zendaya", 10)
	println(p.name.len())
	println(p.name.empty())
	for(i: long = 0l, p.name.len() - 1l){
		print(p.name.at(i))
	}
	println()
	println(p.name.slice(4l,9l))
	p.name = p.name.sub(4l,3l)
	println(p.name)
	sum: int = 0
	for(i: int = 1, 1_000_000){
	    sum = sum + i
	}
	println(sum)
	//println(p.name.su)
}
)");
#endif
}
catch (std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
