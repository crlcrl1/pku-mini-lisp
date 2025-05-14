#include <gtest/gtest.h>
#include <pool.h>

#include "eval_env.h"
#include "parser.h"
#include "rjsj_test.hpp"
#include "test_util.h"
#include "tokenizer.h"
#include "value.h"

std::string eval(const std::string& input) {
    const auto env = ValuePool::instance()->root();
    ValuePool::instance()->gc();
    auto tokens = Tokenizer::tokenize(input, "<test>", 0);
    Parser parser(std::move(tokens));
    const auto value = parser.parse();
    const auto result = env->eval(value);
    return result->toString();
}

TEST(lisp_test_eval, Lv2) {
    ValuePool::instance()->root()->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv2, eval);
    ValuePool::dispose();
}

TEST(list_test_eval, Lv3) {
    ValuePool::instance()->root()->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv3, eval);
    ValuePool::dispose();
}

TEST(list_test_eval, Lv4) {
    ValuePool::instance()->root()->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv4, eval);
    ValuePool::dispose();
}

TEST(list_test_eval, Lv5) {
    ValuePool::instance()->root()->reset();
    ValuePool::instance()->gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv5, eval);
    ValuePool::dispose();
}

TEST(list_test_eval, Lv5Extra) {
    ValuePool::instance()->root()->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv5Extra, eval);
    ValuePool::dispose();
}

TEST(list_test_eval, Lv6) {
    ValuePool::instance()->root()->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv6, eval);
    ValuePool::dispose();
}

TEST(list_test_eval, Lv7) {
    ValuePool::instance()->root()->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv7, eval);
    ValuePool::dispose();
}

TEST(list_test_eval, Lv7Lib) {
    ValuePool::instance()->root()->reset();
    RUN_TEST(rjsj_mini_lisp_test_Lv7Lib, eval);
    ValuePool::dispose();
}

TEST(list_test_eval, Sicp) {
    ValuePool::instance()->root()->reset();
    RUN_TEST(rjsj_mini_lisp_test_Sicp, eval);
    ValuePool::dispose();
}