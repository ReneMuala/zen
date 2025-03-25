#include "../include/tokenization.hpp"
#include <gtest/gtest.h>
#include <map>
#include <regex>
#include <string>

TEST(TokenizationTest, ToString) {
  std::map<std::string, std::regex> test_map = {
      {"test", std::regex("test_regex")}};
  std::string result = ILC::tokenization::to_string(test_map);
  ASSERT_EQ(result, "{ test : std::regex } ");
}

TEST(TokenizationTest, ToString2) {
  std::map<std::string, std::string> test_map = {{"test", "test2"}};
  std::string result = ILC::tokenization::to_string(test_map);
  ASSERT_EQ(result, "{ 'test' : 'test2' } ");
}

TEST(TokenizationTest, ToString3) {
  std::list<std::string> test_list = {"test", "test2"};
  std::string result = ILC::tokenization::to_string(test_list);
  ASSERT_EQ(result, "'test' 'test2' ");
}

TEST(TokenizationTest, ToString4) {
  std::list<std::pair<std::string, std::string>> test_list = {
      {"test", "test2"}, {"test3", "test4"}};
  std::string result = ILC::tokenization::to_string(test_list);
  ASSERT_EQ(result, "{ 'test' : 'test2' } { 'test3' : 'test4' } ");
}

TEST(TokenizationTest, Tokenize) {
  std::list<std::pair<std::string, std::regex>> dict = {
      {"test", std::regex("test")}};
  std::list<std::string> pre_tokens = {"test", "test2"};
  std::list<std::pair<std::string, std::string>> result =
      ILC::tokenization::tokenize<std::string>(dict, "error", pre_tokens);
  std::list<std::pair<std::string, std::string>> expected = {
      {"test", "test"}, {"error", "test2"}};
  ASSERT_EQ(result, expected);
}

TEST(SplitTest, Split) {
  std::string test_string = "test test2";
  std::list<std::string> result =
      ILC::tokenization::split(std::regex(" "), test_string);
  std::list<std::string> expected = {"test", "test2"};
  ASSERT_EQ(result, expected);
}

TEST(DetachTest, Detach) {
  std::string test_string = "test+test*2";
  std::list<std::string> result =
      ILC::tokenization::detach(std::regex("\\w+|\\+|\\*"), test_string);
  std::list<std::string> expected = {"test", "+", "test", "*", "2"};
  ASSERT_EQ(result, expected);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}