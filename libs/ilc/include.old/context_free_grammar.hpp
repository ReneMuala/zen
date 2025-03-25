#pragma once
#include "symbol.hpp"
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <utility>
#include <vector>

namespace ILC {
using namespace std;
using namespace symbol;

namespace context_free_grammar {
typedef pair<symbol_t, vector<symbol_t>> production;

struct context_free_grammar_t {
  set<symbol_t> non_terminals;
  set<symbol_t> terminals;
  vector<production> production_rule;
  symbol_t start;
  optional<production>
  get_production(const symbol_t &non_terminal, int &index,
                 const bool check_non_terminal = false) const {
    if (check_non_terminal) {
      if (not is_not_terminal(non_terminal)) {
        throw runtime_error(format("{}: symbol {} is not not_terminal",
                                   string(__FUNCTION__), (int)non_terminal));
      }
    }

    for (; index < production_rule.size();) {
      production prod = production_rule.at(index++);
      if (prod.first == non_terminal) {
        return prod;
      }
    }
    return {};
  }

  bool is_not_terminal(const symbol_t &symbol) const {
    return non_terminals.find(symbol) != non_terminals.end();
  }
  
  bool is_terminal(const symbol_t &symbol) const {
    return terminals.find(symbol) != terminals.end();
  }

  void verify() {
    for (const auto &[S, SS] : production_rule)
      if (non_terminals.find(S) == non_terminals.end()) {
        throw runtime_error(
            format("{}: invalid production, symbol {} is not not_terminal",
                   string(__FUNCTION__), (int)S));
      }
  }
};
} // namespace context_free_grammar
using namespace context_free_grammar; // namespace cfg
} // namespace ILC