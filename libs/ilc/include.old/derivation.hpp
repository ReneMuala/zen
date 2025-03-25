#pragma once
#include "context_free_grammar.hpp"
#include "derivation_tree.hpp"
#include "symbol.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <utility>
#include <vector>
namespace ILC {
using namespace std;
using namespace symbol;
using namespace context_free_grammar;
using namespace derivation_tree;
namespace derivation {

struct derivation_result_t {
  int production_index;
  vector<symbol_t> new_sentence, old_sentence;
  ssize_t offset, limit;
  symbol_t symbol;
  vector<symbol_t> chain;
  shared_ptr<derivation_result_t> super, ground;
};

inline void print_sentence_vector(const vector<symbol_t> &symbols) {
  for (const auto &symbol : symbols) {
    cout << symbol << " ";
  }
}

inline shared_ptr<derivation_result_t>
print_result(const shared_ptr<derivation_result_t> &_result) {
  shared_ptr<derivation_result_t> root_result;
  for (auto i = _result; i; i = i->super)
    print_sentence_vector(i->chain);
  return root_result;
}

inline shared_ptr<derivation_result_t>
get_root_result(shared_ptr<derivation_result_t> &result) {
  shared_ptr<derivation_result_t> root = result;
  if (not root)
    return nullptr;
  while (root->super)
    root = root->super;
  return root;
}

inline bool operator==(const derivation_result_t &A, const derivation_result_t &B) {
  return A.production_index == B.production_index;
}

struct sentences_history_t : vector<vector<symbol_t>> {};

inline void for_each_derivation_result(
    const shared_ptr<derivation_result_t> &result,
    function<void(const shared_ptr<derivation_result_t> &res)> &callback) {
  if (not result) {
    cout << "null" << endl;
    return;
  }
  for (auto i = result; i; i = i->super)
    callback(i);
}

template <typename T>
inline shared_ptr<derivation_tree_node_t<T>>
make_node(const symbol_t &symbol, const vector<symbol_t> &sentence) {
  shared_ptr<derivation_tree_node_t<T>> node =
      make_shared<derivation_tree_node_t<T>>();
  node->symbol = symbol;
  node->children =
      derivation_tree_node_t<T>::compose_children_vector(sentence, node);
  return node;
}

inline vector<shared_ptr<derivation_result_t>>
derivate(context_free_grammar_t &cfg, const vector<symbol_t> &sentence,
         size_t production_offset = 0) {
  vector<shared_ptr<derivation_result_t>> results;
  vector<symbol_t> sentence_copy;
  for (auto i = cfg.production_rule.rbegin(); i < cfg.production_rule.rend();
       i++) {
    if (production_offset) {
      production_offset--;
      continue;
    }
    long offset = 0;
    const auto &symbol = i->first;
    const auto &chain = i->second;

    while (true) {
      sentence_copy = sentence;
      auto search_res = search(sentence_copy.begin() + offset,
                               sentence_copy.end(), chain.begin(), chain.end());
      if (search_res != sentence_copy.end()) {
        offset = distance(sentence_copy.begin(), search_res);
        auto limit_it = search_res + chain.size();
        sentence_copy.erase(search_res, limit_it);
        sentence_copy.insert(sentence_copy.begin() + offset, {symbol});
        results.push_back(make_shared<derivation_result_t>(derivation_result_t{
            .production_index = (int)(i - cfg.production_rule.rbegin()),
            .new_sentence = sentence_copy,
            .old_sentence = sentence,
            .offset = offset,
            .limit = distance(sentence_copy.begin(), limit_it),
            .symbol = symbol,
            .chain = chain,
        }));
        offset++;
      } else {
        break;
      }
    }
  }
  return results;
}

inline bool derivate_recursively(context_free_grammar_t &cfg,
                          const vector<symbol_t> &sentence,
                          sentences_history_t &history,
                          shared_ptr<derivation_result_t> &_result) {
  history.push_back(sentence);
  auto results = derivate(cfg, sentence);
  for (auto &result : results) {
    if (result->new_sentence.size() == 1 &&
        result->new_sentence.at(0) == cfg.start) {
      if (_result) {
        _result->super = result;
        result->ground = _result;
      }
      return true;
    } else if (find(history.rbegin(), history.rend(), result->new_sentence) ==
               history.rend()) {
      if (derivate_recursively(cfg, result->new_sentence, history, result)) {
        if (_result) {
          _result->super = result;
          result->ground = _result;
        }
        return true;
      }
    }
  }
  return false;
}

inline bool derivate_recursively(context_free_grammar_t &cfg,
                          const vector<symbol_t> &sentence,
                          sentences_history_t &history) {
  shared_ptr<derivation_result_t> _result = nullptr;
  return derivate_recursively(cfg, sentence, history, _result);
}

template <typename T>
shared_ptr<derivation_tree_node_t<T>>
build_derivation_tree(shared_ptr<derivation_result_t> &result) {
  shared_ptr<derivation_result_t> root_result = get_root_result(result);
  shared_ptr<derivation_tree_node_t<T>> root;
  for (auto i = root_result; i->ground; i = i->ground) {
    auto node = make_node<T>(i->symbol, i->chain);
    if (not root) {
      root = node;
      continue;
    }
    int offset = i->offset;
    auto child = root->get_child(offset);
    root->replace_child(i->offset, node);
  }
  return root;
}
} // namespace derivation
using namespace derivation;
} // namespace ILC