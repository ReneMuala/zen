
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
	program->add(zen::library::zen::create(parser->pool));
	program->add(zen::library::casting::create(parser->pool));
	program->add(zen::library::string::create(parser->pool));
	program->add(zen::library::io::create(parser->pool));
	parser->prog = program;
	program->add(parser->lib);
	setup_parser(parser, std::string(code));
	parser->discover();
	parser->parse();
	auto params = std::vector<std::shared_ptr<zen::builder::type>>{};
	std::string hint;
	if (const auto result = parser->tab->get_function("main",params, hint); result.has_value())
	{
		const auto main = result.value().second;
		const auto entry_point = program->link(main);
		fmt::println("compiled with {} word(s)", program->code.size());
		if constexpr (false)
		{
			for (auto link : program->links)
			{
				for (auto lib : program->libraries)
				{
					if (auto fn = lib.second->get_function(link.first))
					{
						fmt::println("{} -> {}", link.second, fn->get_canonical_name());
						break;
					}
				}
			}
		}
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
	class person {
		name: string
		age: int

		new(name: string,age: int) = {
			this.name = name
			this.age = age
		}
	}

	bool(p: person) = {}
	@debug
	main = {
		println(string("hello".len()))
		println(string("".empty()))
		println(string("abc".at(1l)))
		println("abc".slice(1l,11l))
		println("[sub]")
		println("abc".sub(1l,2l))
		println("abc".sub(1l,1l))
		println("abc".sub(0l,0l))
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
