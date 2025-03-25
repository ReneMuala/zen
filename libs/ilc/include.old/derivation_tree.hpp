#pragma once
#include "symbol.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ILC {
using namespace std;
using namespace symbol;
namespace derivation_tree {
template <typename T> struct derivation_tree_node_t {
  shared_ptr<derivation_tree_node_t> super = nullptr;
  symbol_t symbol;
  optional<T> value;
  vector<shared_ptr<derivation_tree_node_t>> children = {};

  static void __default_print_callbacl(const symbol_t &symbol,
                                       std::ostream &ostream,
                                       const optional<T> &) {
    ostream << symbol;
  }

  void print(ostream &ostream,
             const pair<string, string> &depth_indicator = {"{", "}"},
             function<void(const symbol_t &, std::ostream &, optional<T> &)>
                 callback = __default_print_callbacl) {
    for (auto &child : children) {
      if (child && child->children.size()) {
        if (child->children.size() > 1)
          ostream << depth_indicator.first;
        child->print(ostream, depth_indicator, callback);
        if (child->children.size() > 1)
          cout << depth_indicator.second;
      } else if (child) {
        try {
          callback(child->symbol, ostream, child->value);
        } catch (std::runtime_error e) {
          ostream << e.what();
        }
      } else {
        ostream << "null";
      }
    }
  }

  void for_each_leaf(
      function<void(shared_ptr<derivation_tree_node_t<T>>)> callback) {
    for (auto &child : children) {
      if (child && child->children.size()) {
        child->for_each_leaf(callback);
      } else if (child) {
        try {
          callback(child);
        } catch (std::runtime_error e) {
          cerr << e.what();
        }
      } else {
        cerr << "child is null";
      }
    }
  }

  void for_each_child(
      function<void(shared_ptr<derivation_tree_node_t<T>>)> callback) {
    for (auto &child : children) {
      if (child && child->children.size()) {
        child->for_each_child(callback);
      }
    }
    for (auto &child : children) {
      if (child) {
        try {
          callback(child);
        } catch (std::runtime_error e) {
          cerr << e.what();
        }
      }
    }
  }

  shared_ptr<derivation_tree_node_t> get_child(int &index) {
    for (auto &child : children) {
      if (child->children.size()) {
        auto res = child->get_child(index);
        if (res)
          return res;
      } else if (index <= 0) {
        return child;
      } else {
        --index;
      }
    }
    return nullptr;
  };

  optional<T> get_first_value(bool is_first = false) {
    if (is_first && value)
      return value;
    for (auto &child : children) {
      if (child->value) {
        return child->value;
      } else if (child->children.size()) {
        auto res = child->get_first_value();
        if (res)
          return res;
      }
    }
    return nullptr;
  };

  bool replace_child(int index,
                     shared_ptr<derivation_tree_node_t> &_new_child) {
    auto old = get_child(index);
    if (old) {
      auto new_child = move(_new_child);
      old->symbol = new_child->symbol;
      old->children = new_child->children;
      return true;
    }
    return false;
  };

  const vector<symbol_t> composeSymbolVector() const {
    vector<symbol_t> vec;
    if (children.empty()) {
      vec.push_back(symbol);
    } else {
      for (const auto &child : children) {
        const auto &child_vec = child->composeSymbolVector();
        vec.insert(vec.end(), child_vec.begin(), child_vec.end());
      }
    }
    return vec;
  }

  static const vector<shared_ptr<derivation_tree_node_t>>
  compose_children_vector(const vector<symbol_t> &symbols,
                          shared_ptr<derivation_tree_node_t> _super) {
    vector<shared_ptr<derivation_tree_node_t>> vec;
    for (const auto &symbol : symbols) {
      auto child = make_shared<derivation_tree_node_t>();
      child->super = _super;
      child->symbol = symbol;
      vec.push_back(child);
    }
    return vec;
  }
};
} // namespace derivation_tree
using namespace derivation_tree;
} // namespace ILC