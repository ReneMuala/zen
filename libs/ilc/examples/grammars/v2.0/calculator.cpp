#include "../../../include/ilc.hpp"
#include <exception>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>

enum SYMBOL { VAL, ID, NUM, PLUS, MINUS, MULT, DIV, EQU, ERROR };

std::stack<double> values;
std::vector<std::string> str_chain;
std::map<std::string, double> variables_map;

BEGIN_ILC_CODEGEN

/* clang-format off */

BEGIN_PRODUCTION(PREFIX_VAL)
BEGIN_CHAIN_DECLARATION VAL END_CHAIN_DECLARATION 
  REQUIRE_NON_TERMINAL(0)
END_PRODUCTION

BEGIN_PRODUCTION(SUM_VAL)
BEGIN_CHAIN_DECLARATION PLUS, VAL END_CHAIN_DECLARATION 
  REQUIRE_TERMINAL(0)
  REQUIRE_NON_TERMINAL(1)
  SET_CALLBACK([&]{
    const auto lhs = values.top();
    values.pop();

    const auto rhs = values.top();
    values.pop();
    
    values.push(
      lhs + rhs
    );
  })
END_PRODUCTION

BEGIN_PRODUCTION(SUB_VAL)
BEGIN_CHAIN_DECLARATION MINUS, VAL END_CHAIN_DECLARATION 
  REQUIRE_TERMINAL(0)
  REQUIRE_NON_TERMINAL(1)
  SET_CALLBACK([&]{
    const auto lhs = values.top();
    values.pop();

    const auto rhs = values.top();
    values.pop();
    
    values.push(
      lhs - rhs
    );
  })
END_PRODUCTION

BEGIN_PRODUCTION(MULT_VAL)
BEGIN_CHAIN_DECLARATION MULT, VAL END_CHAIN_DECLARATION 
  REQUIRE_TERMINAL(0)
  REQUIRE_NON_TERMINAL(1)
  SET_CALLBACK([&]{
    const auto lhs = values.top();
    values.pop();

    const auto rhs = values.top();
    values.pop();
    
    values.push(
      lhs * rhs
    );
  })
END_PRODUCTION

BEGIN_PRODUCTION(DIV_VAL)
BEGIN_CHAIN_DECLARATION DIV, VAL END_CHAIN_DECLARATION 
  REQUIRE_TERMINAL(0)
  REQUIRE_NON_TERMINAL(1)
  SET_CALLBACK([&]{
    const auto lhs = values.top();
    values.pop();

    const auto rhs = values.top();
    values.pop();
    
    values.push(
      lhs / rhs
    );
  })
END_PRODUCTION

BEGIN_PRODUCTION(P004_VAL)
BEGIN_CHAIN_DECLARATION NUM END_CHAIN_DECLARATION 
  REQUIRE_TERMINAL(0)
  SET_CALLBACK([&]{
    values.push(
      atof(
        str_chain.at(ILC::offset-1).c_str()
      )
    );
  })
END_PRODUCTION

BEGIN_PRODUCTION(P005_VAL)
BEGIN_CHAIN_DECLARATION ID END_CHAIN_DECLARATION 
  REQUIRE_TERMINAL(0)
  const std::string id = str_chain[ILC::offset-1];
  SET_CALLBACK([&]{
    try {
      values.push(
        variables_map.at(id)
      );
    } catch(std::exception & e){
      values.push(0);
      std::cerr << "Variable '" << id <<"' not found, using zero" << std::endl;
    }
  })
END_PRODUCTION

BEGIN_PRODUCTION(P000_ASGN)
BEGIN_CHAIN_DECLARATION ID, EQU, VAL END_CHAIN_DECLARATION 
  REQUIRE_TERMINAL(0)
  const std::string id = str_chain[ILC::offset-1];
  REQUIRE_TERMINAL(1)
  REQUIRE_NON_TERMINAL(2)
  SET_CALLBACK([&]{
    std::cout << id << ": " << values.top() << std::endl;
    variables_map[id] = values.top();
    values.pop();
  })
END_PRODUCTION

BEGIN_BINDINGS
  BEGIN_SYMBOL_BINDING(VAL)
    (PREFIX_VAL() and ( SUM_VAL() or SUB_VAL() or MULT_VAL() or DIV_VAL() or true)) or 
    P004_VAL() or P005_VAL()
  END_SYMBOL_BINDING
END_BINDINGS

      /* clang-format on */
      END_ILC_CODEGEN

      using namespace ILC;

    bool _parse() {
      compilation_id++;
      offset = 0;
      while ((P000_ASGN()) && not(offset >= chain_size))
        ;
      return offset == chain_size;
    }

/// use jumps and labels

std::tuple<std::vector<SYMBOL>, std::vector<std::string>>
get_sentence_elements(const std::string sentence) {
  using namespace ILC;
  const auto sentence_splited = tokenization::split(std::regex(" "), sentence);

  const auto sentence_detached =
      tokenization::detach(std::regex(R"(\w+|\+|=+|.+)"), sentence_splited);

  /* clang-format off */
  const auto dictionary_symbol = std::list<std::pair<SYMBOL,std::regex>>{
      {EQU, std::regex(R"(=)")},
      {MULT, std::regex(R"(\*)")},
      {DIV, std::regex(R"(/)")},
      {PLUS, std::regex(R"(\+)")},
      {MINUS, std::regex(R"(-)")},
      {NUM, std::regex(R"(\d+(\.\d+)?)")},
      {ID, std::regex(R"(\w+)")},
  };
  /* clang-format on */
  const auto tokens = tokenization::tokenize<SYMBOL>(dictionary_symbol, ERROR,
                                                     sentence_detached);
  auto sentence_symbols = std::vector<SYMBOL>{};
  auto sentence_values = std::vector<std::string>{};

  for (const auto [symbol, value] : tokens) {
    sentence_symbols.push_back(symbol);
    sentence_values.push_back(value);
    // cout << ILC::symbol::symbols_string[symbol];
  }
  return std::tuple(sentence_symbols, sentence_values);
}

void print_vars() {
  std::cout << "[ printing all (" << variables_map.size() << ") variables ]\n";
  for (const auto &[var, value] : variables_map) {
    std::cout << var << ": " << value << std::endl;
  }
}

int main() {
  std::cout
      << "ILCalc 1.0" << std::endl
      << " Syntax: {empty + enter} : Print all variables\n [id] = [expr] ... : "
         "Define a variable with name [id] and value [expr]\n"
      << std::endl
      << "Where:\n  [id] is an identifier\n  [expr] is [expr] + "
         "[expr] | [expr] - [expr] | [expr] * [expr] | [expr] / [expr] | "
         "NUMBER | [ID]\n\n";

  bool should_exit = false;
  std::string input;
  do {
    std::cout << "% ";
    std::getline(std::cin, input);

    auto [schain, vchain] = get_sentence_elements(input);
    chain = std::move(schain);

    str_chain = std::move(vchain);
    chain_size = chain.size();
    if (input == "exit")
      break;
    else if (input.empty()) {
      print_vars();
      continue;
    }
    bool is_valid = _parse();

    if (not is_valid) {
      std::cout << "Syntax error at token: " << offset << std::endl;
    }
  } while (true);
  // std::cout << (is_valid ? "is valid" : "is not valid") << std::endl;
}
