
#include <iostream>
#include <memory>
#include <optional>
#include "lexer/lexer.hpp"
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
#include "library/typing.hpp"

#ifdef KAIZEN_WASM
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif
#include <chrono>
std::vector<zen::token> tokens;

inline std::vector<SYMBOL> setup_parser(const std::string& code)
{
	tokens.clear();
	std::vector<SYMBOL> chain;
	std::stringstream stream(code);
	zen::lexer lexer(stream);
	while (auto token = lexer.next())
	{
		chain.push_back(token->type);
		tokens.emplace_back(token.value());
	}
	return chain;
}

extern "C" {
void EMSCRIPTEN_KEEPALIVE zen_sum(int a, int b)
{
	fmt::print("{} + {} = {}\n", a, b, a + b);
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
	std::vector<SYMBOL> chain = setup_parser(std::string(code));
	const auto parser = builder_parser::make(chain, chain.size());
	parser->prog = program;
	program->add(parser->lib);
	program->add(zen::libraries::_zen::create(*parser->pool));
	program->add(zen::libraries::casting::create(*parser->pool));
	program->add(zen::libraries::string::create(*parser->pool));
	program->add(zen::libraries::io::create(*parser->pool));
	program->add(zen::libraries::typing::create(*parser->pool));
	parser->offset = 0;
	parser->discover();
	program->add(zen::libraries::test::create(*parser->pool, parser->lib, parser->prog));
	parser->offset = 0;
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
		program->add(zen::libraries::_zen::create(*parser->pool));
		program->add(zen::libraries::casting::create(*parser->pool));
		program->add(zen::libraries::string::create(*parser->pool));
		program->add(zen::libraries::io::create(*parser->pool));
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
		program->add(zen::libraries::test::create(*parser->pool, parser->lib, parser->prog));
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
main() = {
		io::println("Hello world")
		x := 1
		if(typing::is<int>(x)){
			println("x is int")
		} else {
			println("x is not int")
		}
		println(rand())
		println(add<float>(2f,3f))
		println(add(2f, 3f))
		x := 4.3
		x := 2
		ii := IntInt(20,20)
		value := money(50.25)
		println(value)
		pairSet := PairSet<int, short>(10i, 10s, 11i, 11s)
		pair := Pair<int, short>(19i, 19s)
		println(string<int, short>(pair))
		println(string<int, short>(pairSet.p1))
		println(string<int, short>(pairSet.p2))
		pair := Pair<string, money>("fifty hundred", 50e100)
		fp := FloatPair(2f,3f)
		println(string<float, float>(fp))
		runGlobalTests()
		p := person("Rene", 20)
		println("got name: " + getName<person>(p))
		println(addTwoThings<double>(3.4,5.3))
		println(addTwoThings<string>("hello"," world"))
	}
	class Pair<A, B> {
		a: A
		b: B
		new(a: A, b: B) = {
			this.a = a
			this.b = b
		}
	}
class E {
	e: int
}

printE(e: E, i: int) = {
	println(e.e)
}

class E2 {
	f: long
}

printE(e: E) = {
	println(e.e)
}

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

test_person_name_equality = {
	x: bool = person("Zendaya", 20).name == person("Zendaya", 20).name
}


person2(name: string, age: int) = person {
	p: person
	p.name = name
	p.age = age
	p
}

@test
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
	impossible_sum = bool {
		1+2 == 2
	}

	@test
	person_equality = bool {
		person("Zendaya",20) == person("Zendaya",20)
	}

	@test
	person_inequality = bool {
		person("Zenya",20) != person("Zendaya",20)
	}
	bool(p: person) = {}
	@test
	io_test = bool {
		print(true)
		println()
		println(true)
		print(65b)
		println()
		println(65b)
		print(1s)
		println()
		println(1s)
		print(1i)
		println()
		println(1i)
		print(1l)
		println()
		println(1l)
		print(1.5f)
		println()
		println(1.5f)
		print(1.5d)
		println()
		println(1.5d)
		true
	}
	/*
	gen1<T> = T {
		ignored code?
	}
	subg<T>(a: T, b: T) = T<B<int>>{
		a+b
	}
	abs<T>(a: T) = T{
		if(a < T(0)){
			-a
		} else {
			a
		}
	}
	sumg<T>(a: T, b: T) = T(abs<T>(a)+abs<T>(b))
		println(sumg<long>(-1l,-2l))
		println(sumg<int>(-1,-2))
		println(sumg<double>(1.5,-2.5))
		println(abs<long>(-1l))
*/

	main4 = {
		println(string("hello".len()))
		println("".empty())
		print("".empty())
		println()
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
		println(string("rene" != "rene"))
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

	addTwoThings<T>(x: T, y: T) = T(x+y)
	getName<N>(n: N) = string(n.name)

	@test
	str_casting = bool {
		bool("true") == true && "true" == string(true)
		&& bool("false") == false && "false" == string(false)
		&& byte("1") == 1b && "A" == string(65b)
		&& short("1") == 1s && "1" == string(1s)
		&& int("1") == 1i && "1" == string(1i)
		&& long("1") == 1l && "1" == string(1l)
		&& float("1.5") == 1.5f && "1.5" == string(1.5f)
		&& double("2.5") == 2.5d && "2.5" == string(2.5d)
	}

	sum<T>(x: T, y: T) = T(x+y)

	abs<T>(n: T) = T {
		if(n < T(0b) /* cast 0b to T */) {
			-n
		} else {
			n
		}
	}
	sum_abs<F,S,R>(first: F, second: S) = R(R(abs<F>(first)) + R(abs<S>(second)))

	class ClassWithGenericsMethods {
		data: double
		data_as<T> = T(T(data))
		sum<T>(x: T, y: T) = T(x+y)
		abs<T>(n: T) = T {
			if(n < T(0b) /* cast 0b to T */) {
				-n
			} else {
				n
			}
		}
		sum_abs<F,S,R>(first: F, second: S) = R(R(abs<F>(first)) + R(abs<S>(second)))
	}

	class ClassWithGenericsMethods2 {
		c: ClassWithGenericsMethods
	}

	@test
	generic_functions = bool {
		x: ClassWithGenericsMethods
		x.data = 1.5
		y: ClassWithGenericsMethods2
		y.c.data = 5.5
		sum<int>(1,2) == 3 && sum<double>(1.5, 2.5) == 4d && abs<int>(-1) == 1 && sum_abs<byte, float, double>(1b,1.5f) == 2.5d &&
		x.sum<int>(1,2) == 3 && x.sum<double>(1.5, 2.5) == 4d && x.abs<int>(-1) == 1 && x.sum_abs<byte, float, double>(1b,1.5f) == 2.5d &&
		x.data_as<int>() == 1 && x.data_as<string>() == "1.5" &&
		y.c.sum<int>(1,2) == 3 && y.c.sum<double>(1.5, 2.5) == 4d && y.c.abs<int>(-1) == 1 && y.c.sum_abs<byte, float, double>(1b,1.5f) == 2.5d &&
		y.c.data_as<int>() == 5 && y.c.data_as<string>() == "5.5"
	}

	class X {
		y: Y
	}

	class Y {
		k: i32
	}

class Person {
    name: string
    birth: Date
    profession: string
    alias: string
    coffeeLink: string
    reader: string
    new(name: string,birth: Date) = {
        this.name = name
        this.birth = birth
    }

    setProfession(profession: string) = {
        this.profession = profession
    }

    setAlias(alias: string) = {
        this.alias = alias
    }

    setCoffeeLink(link: string) = {
        this.coffeeLink = link
    }

    setReader(reader: string) = {
        this.reader = reader
    }

    getReader = string {
        if(reader == "PUT_YOUR_NAME_HERE") {
            "world"
        } else {
            reader
        }
    }

    getYoutubeLink = string("https://youtube.com/@" + alias)

    getLinkedinLink = string("https://linkedin.com/in/" + alias)

    getGithubLink = string("https://github.com/" + alias)

    toString() = string("Hello "+ getReader() +"!, i'm " + name + ".\nA " + profession + " born on " + birth.toString() + ".")

}

class Date {
    day: int
    year: int
    month: string

    new(day: int,month: string, year: int) = {
        this.day = day
        this.year = year
        this.month = month
    }

    toString = string(month + " " + string(day) + ", " + string(year))
}


	using FloatPair = Pair<float, float>
	using i32 = int
	using IntInt = Pair<i32, i32>
	using money = double
	class PairSet<A, B> {
		p1: Pair<A,B>
		p2: Pair<A,B>
		new(a1: A, b1: B, a2: A, b2: B) = {
			this.p1 = Pair<A,B>(a1, b1)
			this.p2 = Pair<A,B>(a2, b2)
		}
	}
	class maybe<T> {
		valid = bool(true)
	}
	string<A, B>(pair: Pair<A, B>) = string(string(pair.a) + " " + string(pair.b))
	using add(x: float, y: float) = add<float>
	add<T>(x: T, y: T) = T(x+y)
	using abs(x: double) = abs<double>
	using abs(x: float) = abs<float>
	using rand() = getRandomNumber
	getRandomNumber = int(21)

)");
	// implement symbol manager
	// implement deference wrappers
#endif
}
catch (std::exception& e)
{
	std::cerr << e.what() << std::endl;
}
