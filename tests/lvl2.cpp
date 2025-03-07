#include <gtest/gtest.h>

#include "parser.h"
#include "rjsj_test.hpp"
#include "tokenizer.h"
#include "util.h"
#include "value.h"

std::string eval(const std::string& input) {
    auto tokens = Tokenizer::tokenize(input);
    Parser parser(std::move(tokens));
    const auto value = parser.parse();
    return value->toString();
}

TEST(lisp_test, Lv2) {
    RUN_TEST(rjsj_mini_lisp_test_Lv2, eval);
}

TEST(list_test, Lv2Only) {
    RUN_TEST(rjsj_mini_lisp_test_Lv2Only, eval);
}
