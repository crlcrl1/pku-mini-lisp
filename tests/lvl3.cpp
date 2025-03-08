#include <gtest/gtest.h>

#include "eval_env.h"
#include "parser.h"
#include "rjsj_test.hpp"
#include "test_util.h"
#include "tokenizer.h"
#include "value.h"

EvalEnv envLv3;

std::string evalLv3(const std::string& input) {
    auto tokens = Tokenizer::tokenize(input);
    Parser parser(std::move(tokens));
    auto value = parser.parse();
    const auto result = envLv3.eval(std::move(value));
    return result->toString();
}

TEST(lisp_test_lv3, Lv2) {
    envLv3.reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv2, evalLv3);
}

TEST(list_test_lv3, Lv3) {
    envLv3.reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv3, evalLv3);
}
