#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <regex>
#include <string>
#include <utility>
#include <vector>
namespace ILC {

namespace tokenization {
using namespace std;

inline string to_string(map<string, regex> src) {
  string result;
  for (const auto &[str, reg] : src) {
    result += "{ " + str + " : std::regex } ";
  }
  return result;
}

inline string to_string(map<string, string> src) {
  string result;
  for (const auto &[str, str2] : src) {
    result += "{ '" + str + "' : '" + str2 + "' } ";
  }
  return result;
}

inline string to_string(list<string> src) {
  string result;
  for (const auto &str : src) {
    result += "'" + str + "' ";
  }
  return result;
}

inline string to_string(list<pair<string, string>> src) {
  string result;
  for (const auto &[str, str2] : src) {
    result += "{ '" + str + "' : '" + str2 + "' } ";
  }
  return result;
}

template <typename T>
inline list<pair<T, string>> tokenize(list<pair<T, regex>> dict, T error_type,
                                      list<string> pre_tokens) {
  list<pair<T, string>> tokens;
  auto is_type_resolved = bool();
  for (const auto &pre_token : pre_tokens) {
    is_type_resolved = false;
    for (const auto &[type_name, type_regex] : dict) {
      if (regex_match(pre_token, type_regex)) {
        tokens.push_back({type_name, pre_token});
        is_type_resolved = true;
        break;
      }
    }
    if (!is_type_resolved)
      tokens.push_back({error_type, pre_token});
  }
  return tokens;
}

inline list<string> split(regex regex, string src) {
  auto iterator = sregex_token_iterator(src.begin(), src.end(), regex, -1);
  auto token_li = list<string>({iterator, sregex_token_iterator()});
  token_li.remove_if([](string str) { return str == ""; });
  return token_li;
}

inline list<string> detach(regex regex, string src) {
  auto iterator = sregex_token_iterator(src.begin(), src.end(), regex, 0);
  return {iterator, sregex_token_iterator()};
}

inline list<string> detach(regex regex, list<string> srcs) {
  auto result = list<string>();
  for (const auto &src : srcs) {
    for (const auto &component : detach(regex, src)) {
      result.push_back(component);
    }
  }
  return result;
}
} // namespace tokenization
using namespace tokenization;
} // namespace ILC