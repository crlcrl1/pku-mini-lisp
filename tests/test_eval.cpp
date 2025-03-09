#include <gtest/gtest.h>

#include "eval_env.h"
#include "parser.h"
#include "rjsj_test.hpp"
#include "test_util.h"
#include "tokenizer.h"
#include "value.h"

auto env = EvalEnv::createEnv();

std::string eval(const std::string& input) {
    auto tokens = Tokenizer::tokenize(input);
    Parser parser(std::move(tokens));
    auto value = parser.parse();
    const auto result = env->eval(std::move(value));
    return result->toString();
}

TEST(lisp_test_eval, Lv2) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv2, eval);
}

TEST(list_test_eval, Lv3) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv3, eval);
}

TEST(list_test_eval, Lv4) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv4, eval);
}

TEST(list_test_eval, Lv5) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv5, eval);
}

TEST(list_test_eval, Lv5Extra) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv5Extra, eval);
}

TEST(list_test_eval, Lv6) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv6, eval);
}

TEST(list_test_eval, Lv7) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv7, eval);
}

TEST(list_test_eval, Lv7Lib) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv7Lib, eval);
}

TEST(list_test_eval, Sicp) {
    env->reset();
    RUN_TEST(rjsj_mini_lisp_test_Sicp, eval);
}