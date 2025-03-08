#include <gtest/gtest.h>

#include "eval_env.h"
#include "parser.h"
#include "rjsj_test.hpp"
#include "test_util.h"
#include "tokenizer.h"
#include "value.h"

EvalEnv envLv4;

std::string evalLv4(const std::string& input) {
    auto tokens = Tokenizer::tokenize(input);
    Parser parser(std::move(tokens));
    auto value = parser.parse();
    const auto result = envLv4.eval(std::move(value));
    return result->toString();
}

TEST(lisp_test_lv4, Lv2) {
    envLv4.reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv2, evalLv4);
}

TEST(list_test_lv4, Lv3) {
    envLv4.reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv3, evalLv4);
}

TEST(list_test_lv4, Lv4) {
    envLv4.reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv4, evalLv4);
}