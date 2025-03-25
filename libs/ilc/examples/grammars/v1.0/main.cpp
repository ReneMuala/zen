#include "../../../include.old/ilc.hpp"
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <regex>
#include <tuple>

using namespace ILC;

void configure_cfg(ILC::context_free_grammar::context_free_grammar_t &cfg) {
  cfg.terminals = {PLUS, MINUS, TIMES, SLASH, EQU, DEQU, NOT, AND, OR,
                   //
                   NUM, ID};

  cfg.non_terminals = {
      STAT,
      // operations
      SUM,
      SUB,
      MUL,
      DIV,
      ASGN,
      EQLT,
      NEG,
      CONJ,
      DISJ,
      VAL,
  };

  cfg.start = STAT;

  /* clang-format off */
  cfg.production_rule = {
      production{STAT, {ASGN}},
      production{STAT, {STAT, ASGN}},
      production{ASGN, {ID, EQU, VAL}},
      production{VAL, {VAL, PLUS, VAL}},
      production{VAL, {VAL, MINUS, VAL}},
      production{VAL, {VAL, TIMES, VAL}},
      production{VAL, {VAL, DIV, VAL}},
      production{VAL, {NUM}},
  };
  /* clang-format on */

  cfg.verify();
}

tuple<vector<symbol_t>, vector<string>>
get_sentence_elements(const string sentence) {
  const auto sentence_splited = tokenization::split(regex(" "), sentence);

  const auto sentence_detached =
      tokenization::detach(regex(R"(\w+|\+|=+|.+)"), sentence_splited);

  /* clang-format off */
  const auto dictionary_symbol = list<pair<symbol_t, regex>>{
      {DEQU, regex(R"(==)")},
      {EQU, regex(R"(=)")},
      {TIMES, regex(R"(\*)")},
      {SLASH, regex(R"(/)")},
      {PLUS, regex(R"(\+)")},
      {MINUS, regex(R"(-)")},
      {NUM, regex(R"(\d+(\.\d+)?)")},
      {ID, regex(R"(\w+)")},
  };
  /* clang-format on */
  const auto tokens = tokenization::tokenize<symbol_t>(dictionary_symbol, ERROR,
                                                       sentence_detached);
  auto sentence_symbols = vector<symbol_t>{};
  auto sentence_values = vector<string>{};

  for (const auto [symbol, value] : tokens) {
    sentence_symbols.push_back(symbol);
    sentence_values.push_back(value);
    // cout << ILC::symbol::symbols_string[symbol];
  }
  return std::tuple(sentence_symbols, sentence_values);
}

// namespace ILC
int main() {
  auto cfg = context_free_grammar_t{};
  configure_cfg(cfg);
  shared_ptr<derivation_result_t> _result = make_shared<derivation_result_t>();
  vector<symbol_t> chain = {
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,
      PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,
      PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,
      NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,
      MINUS, NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,
      NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,
      PLUS,  NUM,   PLUS,  NUM,   MINUS, NUM,   ID,    EQU,   NUM,   ID,
      EQU,   NUM,   ID,    EQU,   NUM,   PLUS,  NUM,   PLUS,  NUM,   PLUS,
      NUM,   MINUS, NUM
  };
  auto history = sentences_history_t{};
  auto is_valid = derivate_recursively(cfg, chain, history, _result);
  if (is_valid) {
    cout << "valid\n";
  } else {
    cout << "invalid\n";
  }
}
