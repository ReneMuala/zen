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

inline std::string to_string(std::map<std::string, std::regex> src) {
  std::string result;
  for (const auto &[str, reg] : src) {
    result += "{ " + str + " : std::regex } ";
  }
  return result;
}

inline std::string to_string(std::map<std::string, std::string> src) {
  std::string result;
  for (const auto &[str, str2] : src) {
    result += "{ '" + str + "' : '" + str2 + "' } ";
  }
  return result;
}

inline std::string to_string(std::list<std::string> src) {
  std::string result;
  for (const auto &str : src) {
    result += "'" + str + "' ";
  }
  return result;
}

inline std::string
to_string(std::list<std::pair<std::string, std::string>> src) {
  std::string result;
  for (const auto &[str, str2] : src) {
    result += "{ '" + str + "' : '" + str2 + "' } ";
  }
  return result;
}

template <typename T>
inline std::list<std::pair<T, std::string>>
tokenize(std::list<std::pair<T, std::regex>> dict, T error_type,
         std::list<std::string> pre_tokens) {
  std::list<std::pair<T, std::string>> tokens;
  auto is_type_resolved = bool();
  for (const auto &pre_token : pre_tokens) {
    is_type_resolved = false;
    for (const auto &[type_name, type_regex] : dict) {
      if (std::regex_match(pre_token, type_regex)) {
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

inline std::list<std::string> split(std::regex regex, std::string src) {
  auto iterator = std::sregex_token_iterator(src.begin(), src.end(), regex, -1);
  auto token_li =
      std::list<std::string>({iterator, std::sregex_token_iterator()});
  token_li.remove_if([](std::string str) { return str == ""; });
  return token_li;
}

inline std::list<std::string> detach(std::regex regex, std::string src) {
  auto iterator = std::sregex_token_iterator(src.begin(), src.end(), regex, 0);
  return {iterator, std::sregex_token_iterator()};
}

inline std::list<std::string> detach(std::regex regex,
                                     std::list<std::string> srcs) {
  auto result = std::list<std::string>();
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