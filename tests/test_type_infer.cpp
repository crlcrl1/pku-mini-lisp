#ifdef USE_LLVM

#include <gtest/gtest.h>

#include "error.h"
#include "eval_env.h"
#include "jit/jit.h"
#include "parser.h"
#include "pool.h"
#include "tokenizer.h"

jit::JITFunctionValue* getResult(const std::vector<std::string>& input) {
    EvalEnv* env = pool.root();
    ValuePtr result = nullptr;
    for (const auto& str : input) {
        auto tokens = Tokenizer::tokenize(str);
        Parser parser(std::move(tokens));
        const auto value = parser.parse();
        result = env->eval(value);
    }
    const auto jitFunc = dynamic_cast<jit::JITFunctionValue*>(result);
    if (jitFunc == nullptr) {
        throw ValueError("Expected a JITFunctionValue");
    }
    return jitFunc;
}

void testTypeInfer(const std::vector<std::string>& code, const jit::JITValueType type,
                   auto&&... args) {
    const auto jitFunc = getResult(code);
    EXPECT_EQ(jitFunc->getReturnType(), std::nullopt);  // not compiled yet

    jitFunc->compile({args...});
    const auto returnType = jitFunc->getReturnType();
    EXPECT_TRUE(returnType.has_value());
    EXPECT_EQ(jitFunc->getReturnType().value(), type);

    pool.gc();
}

TEST(type_infer_test, basic) {
    std::vector<std::string> code = {
        "(define (add a b) (+ a b))",
        "(jit add)",
    };
    testTypeInfer(code, jit::JITValueType::NUMBER, pool.makeValue<NumericValue>(1),
                  pool.makeValue<NumericValue>(2));

    code = {
        "(define (add a b) (list a b))",
        "(jit add)",
    };
    testTypeInfer(code, jit::JITValueType::LIST, pool.makeValue<NumericValue>(1),
                  pool.makeValue<NumericValue>(2));
}

TEST(test_type_infer, nested) {
    std::vector<std::string> code = {
        "(define (add a b) (list a b))",
        "(define add_jitted (jit add))",
        "(define (add2 a b) (add_jitted a b))",
        "(jit add2)",
    };
    testTypeInfer(code, jit::JITValueType::LIST, pool.makeValue<NumericValue>(1),
                  pool.makeValue<NumericValue>(2));

    code = {
        "(define (add a b) (+ a b))",
        "(define add_jitted (jit add))",
        "(define (add2 a b) (add_jitted a b))",
        "(jit add2)",
    };
    testTypeInfer(code, jit::JITValueType::NUMBER, pool.makeValue<NumericValue>(1),
                  pool.makeValue<NumericValue>(2));
}

TEST(test_type_infer, form) {
    std::vector<std::string> code = {
        "(define (call_if) ((if #f + *) 3 4))",
        "(jit call_if)",
    };
    // FIXME: this test fails
    testTypeInfer(code, jit::JITValueType::NUMBER);
}

#endif