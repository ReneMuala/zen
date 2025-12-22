
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
	auto parser = builder_parser::make();
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
		//@debug() // you will be able to see the bytecode of this function
		sum(x:int, y:int) = int(x+y)
		main = {
			y: int = sum0(2,3)+sum0(3,4)
			x: int = sum(1,2)
		}
		sum0(x:int, y:int) = int(x+y)
		@debug
		test = {
			name: string = "R
ene"
		}
	)");
	// implement symbol manager
	// implement deference wrappers
#endif
}
catch (std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
