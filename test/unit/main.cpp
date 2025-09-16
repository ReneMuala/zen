//
// Created by dte on 9/15/2025.
//
#include "lexer/lexer_tests.hpp"
#include "parser/parser.hpp"
std::vector<zen::token> tokens;

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}