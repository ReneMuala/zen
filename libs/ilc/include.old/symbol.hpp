#pragma once
#include <string>

namespace ILC {
namespace symbol {
using namespace std;
enum symbol_t {
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
  // operators
  PLUS,
  MINUS,
  TIMES,
  SLASH,
  EQU,
  DEQU,
  NOT,
  AND,
  OR,
  //
  NUM,
  ID,
  VAL,
  PARETHESIS_OPEN,
  PARETHESIS_CLOSE,
  ERROR,
};
string symbols_string[] = {"STAT",
                           // operations
                           "SUM", "SUB", "MUL", "DIV", "ASGN", "EQLT", "NEG",
                           "CONJ", "DISJ",
                           // operators
                           "+", "-", "*", "/", "=", "==", "!", "&", "|",
                           //
                           "NUM", "ID", "VAL", "(", ")"};
// string symbols_string[] = {"S", "A", "B", "+", "*", "#"};
}; // namespace symbol
using namespace symbol;
} // namespace ILC