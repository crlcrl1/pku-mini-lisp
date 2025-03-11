#include <gtest/gtest.h>
#include <pool.h>

#include "eval_env.h"
#include "parser.h"
#include "rjsj_test.hpp"
#include "test_util.h"
#include "tokenizer.h"
#include "value.h"

std::string eval(const std::string& input) {
    const auto env = pool.root();
    pool.gc();
    auto tokens = Tokenizer::tokenize(input);
    Parser parser(std::move(tokens));
    const auto value = parser.parse();
    const auto result = env->eval(value);
    return result->toString();
}

TEST(lisp_test_eval, Lv2) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv2, eval);
}

TEST(list_test_eval, Lv3) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv3, eval);
}

TEST(list_test_eval, Lv4) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv4, eval);
}

TEST(list_test_eval, Lv5) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv5, eval);
}

TEST(list_test_eval, Lv5Extra) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv5Extra, eval);
}

TEST(list_test_eval, Lv6) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv6, eval);
}

TEST(list_test_eval, Lv7) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv7, eval);
}

TEST(list_test_eval, Lv7Lib) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Lv7Lib, eval);
}

TEST(list_test_eval, Sicp) {
    pool.root()->reset();
    pool.gc();
    RUN_TEST(rjsj_mini_lisp_test_Sicp, eval);
}