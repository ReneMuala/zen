
#include <iostream>
#include <memory>
#include <optional>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "parser/builder_parser.hpp"
#include "builder/function.hpp"
#include <sstream>

#include "builder/table.hpp"

#ifdef KAIZEN_WASM
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif
#include <chrono>
std::vector<zen::token> tokens;

extern "C" {
void EMSCRIPTEN_KEEPALIVE zen_sum(int a, int b)
{
	fmt::print("{} + {} = {}\n", a, b, a + b);
}

inline void setup_parser(const std::shared_ptr<builder_parser>& parser, const std::string& code)
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

bool EMSCRIPTEN_KEEPALIVE zen_run(const char* code)
try
{
	if (code == nullptr)
	{
		fmt::println("[zen_compile] called with null argument.");
		return false;
	}
	const auto program = zen::builder::program::create();
	const auto parser = builder_parser::make();
	parser->prog = program;
	program->add(parser->lib);
	setup_parser(parser, std::string(code));
	parser->discover();
	parser->parse();
	return true;
}
catch (std::exception& e)
{
	std::cout << e.what() << std::endl;
	return false;
}
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
	zen_run(R"(
class point {
    x: double
    y: double
	@debug
    new(x: double, y: double) = {
        this.x = x
        this.y = y
    }
}
class CC {
	val: int
	mul(x: int, y: int) = int(x*y)
}

class BB {
	val: int
	c: CC
	sub(x: int, y: int) = int(x-y)
	getC = CC(c)
}
class AA {
	val: int
	b: BB
	sum(x: int, y: int) = int(x+y)
	getB = BB(b)
}
@debug
main3 = {
	pt : point = point(2.0,3.0)
	a: AA
	x: int = a.getB().getC().mul(3,2)
	y: bool = a.getB().getC().mul(3,2) > a.getB().getC().mul(2,4) == a.getB().getC().mul(3,2) < a.getB().getC().mul(2,4)
	y: bool = a.getB().getC().mul(3,2) > a.getB().getC().mul(2,4) == a.getB().getC().mul(3,2) < a.getB().getC().mul(2,4)
}
class person {
	name: string
	surname: string
	age: int
	registered: bool
}

test = {
	x: bool = person("Zendaya", 20).name == person("Zendaya", 20).name
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
	1.5d == 1.5d &&
	"1" == "1" &&
	person("Zendaya", 20) == person("Zendaya", 20) &&
	person("Zendaya", 20).name == person("Zendaya", 20).name &&
	person("Zendaya", 20).age == person("Zendaya", 20).age &&
	1 != 2 &&
	1s != 2s &&
	1i != 2i &&
	1 != 2 &&
	1l != 2l &&
	1.5f != 2.5f &&
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
}
		main2 = {
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
divide(x: int, y: int) = int {
			if(y != 0){
				x/y
			} else {
				println("[detected division by 0]")
				0
			}
		}
rect(lines: int, cols: int) = {
			for(l: int = 1, lines){
				for(c: int = 1, cols){
					print("*")
				}
				println()
			}
		}
	@extern
	print(_:string) = unit
	@extern
	println(_:string) = unit
	@extern
	println() = unit
	)");
	// implement symbol manager
	// implement deference wrappers
#endif
}
catch (std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
