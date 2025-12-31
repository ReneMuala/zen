
#include <iostream>
#include <memory>
#include <optional>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "parser/builder_parser.hpp"
#include "builder/function.hpp"
#include <sstream>

#include "builder/table.hpp"
#include "exceptions/link_error.hpp"
#include "library/casting.hpp"
#include "library/io.hpp"
#include "library/string.hpp"
#include "library/zen.hpp"
#include "fmt/core.h"
#include "library/test.hpp"

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
	program->add(zen::library::_zen::create(parser->pool));
	program->add(zen::library::casting::create(parser->pool));
	program->add(zen::library::string::create(parser->pool));
	program->add(zen::library::io::create(parser->pool));
	setup_parser(parser, std::string(code));
	parser->discover();
	program->add(zen::library::test::create(parser->pool, parser->lib, parser->prog));
	parser->parse();
	auto params = std::vector<std::shared_ptr<zen::builder::type>>{};
	std::string hint;
	if (const auto result = parser->tab->get_function("main",params, hint); result.has_value())
	{
		const auto main = result.value().second;
		const auto entry_point = program->link(main);
		// fmt::println("compiled with {} word(s)", program->code.size());
		zen::vm vm;
		vm.load(program->code);
		vm.run(entry_point);
		fmt::println("");
	} else
	{
		throw zen::exceptions::link_error("failed to find main function");
	}
	return true;
}
catch (std::exception& e)
{
	std::cout << e.what() << std::endl;
	return false;
}
}

// #ifndef KAIZEN_WASM
// #define KAIZEN_CLI
// #endif

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
#elifdef KAIZEN_CLI
	const auto program = zen::builder::program::create();
	std::vector<std::shared_ptr<builder_parser>> parsers;
	if (argc < 2)
	{
		throw std::logic_error(fmt::format("You must provide at least one argument.\n\t{} file1 file2 ... fileN", argv[0]));
	}
	for (int i = 1; i < argc; i++)
	{
		const auto parser = builder_parser::make();
		parser->prog = program;
		program->add(parser->lib);
		program->add(zen::library::_zen::create(parser->pool));
		program->add(zen::library::casting::create(parser->pool));
		program->add(zen::library::string::create(parser->pool));
		program->add(zen::library::io::create(parser->pool));
		std::string filename = argv[i];

		if (filename.empty())
			continue;

		std::ifstream file(filename);
		if (!file.is_open())
			throw zen::exceptions::file_not_found(filename);
		std::string code = {std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
		setup_parser(parser, std::string(code));
		parser->id = filename;
		parser->lib->name = filename;
		parser->discover();
		program->add(zen::library::test::create(parser->pool, parser->lib, parser->prog));
		parsers.push_back(parser);
	}
	for (auto & parser : parsers)
	{
		parser->parse();
	}
	auto params = std::vector<std::shared_ptr<zen::builder::type>>{};
	std::string hint;
	if (const auto result = parsers.front()->tab->get_function("main",params, hint); result.has_value())
	{
		const auto main = result.value().second;
		const auto entry_point = program->link(main);
		zen::vm vm;
		vm.load(program->code);
		vm.run(entry_point);
	} else
	{
		throw zen::exceptions::link_error("failed to find main function");
	}
#else
	zen_run(R"(
class point {
    x: double
    y: double

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
	"1" != "2" && person("Zendaya", 20) != person("Zenia", 20)
}

class Point {
	x: double
	y: double
}

operator >(a: Point, b: Point) = bool {
	a.x > b.x && a.y > b.y
}

string(pt: Point) = string("")

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

	class person {
		name: string
		age: int

		new(name: string,age: int) = {
			this.name = name
			this.age = age
		}
	}
	@test
	sum = bool {
		1 + 1 == 2
	}
	@test
	sum2 = bool {
		1 == 2
	}
	bool(p: person) = {}

	main = {
		println(string("hello".len()))
		println(string("".empty()))
		println(string("abc".at(1l)))
		println("abc".slice(1l,11l))
		println("[sub]")
		println("abc".sub(1l,2l))
		println("abc".sub(1l,1l))
		println("abc".sub(0l,0l))
		println(string("rene" == "muala"))
		println(string("rene" == "rene"))
		println(string("rene" != "muala"))
		// é ação
		println(string("rene" != "rene")
		s: string
			s = "Hello World!"
			println(s)
			if(test_value_equality()){
				println("[test_value_equality: PASSED]")
			} else {
				println("[test_value_equality: FAILED]")
			}
		runGlobalTests()
		runLocalTests()
		x: int
		println(string(x))
	}
	example(x: int) = string("hello")

	test = {
		println(getZenArt())
		println(getZenVersion())
		println("begin")
		for(i: int = 1, 2){
			p1: person = person("Rene",20)
			println(p1.name)
			println(string(p1.age))
		}
		println("end")
	}
	by2(x: int) = int(x*2)
	sum(x: int, y: int) = int(x+y)
	sub(x: int, y: int) = int(x-y)
	)");
	// implement symbol manager
	// implement deference wrappers
#endif
}
catch (std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
