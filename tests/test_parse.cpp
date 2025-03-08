#include <gtest/gtest.h>

#include "parser.h"
#include "rjsj_test.hpp"
#include "test_util.h"
#include "tokenizer.h"
#include "value.h"

std::string parse(const std::string& input) {
    auto tokens = Tokenizer::tokenize(input);
    Parser parser(std::move(tokens));
    const auto value = parser.parse();
    return value->toString();
}

TEST(lisp_test_parse, Lv2) {
    RUN_TEST(rjsj_mini_lisp_test_Lv2, parse);
}

TEST(lisp_test_parse, Lv2Only) {
    RUN_TEST(rjsj_mini_lisp_test_Lv2Only, parse);
}
