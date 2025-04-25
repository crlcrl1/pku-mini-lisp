#include "form_type_infer.h"

#include <unordered_map>

#include "../error.h"
#include "../utils.h"
#include "jit.h"
#include "type_infer.h"

namespace jit {

// clang-format off
const std::unordered_map<std::string, FormInferFunc*> FORM_INFER{
    {"define", &defineForm},
    {"quote", &quoteForm},
    {"if", &ifForm},
    {"and", &logicForm},
    {"or", &logicForm},
    {"lambda", &lambdaForm},
    {"eval", &evalForm},
};
// clang-format on

JITValueType defineForm(const std::vector<ValuePtr>&, EvalEnv*) {
    throw ValueError("define is not supported in JIT mode");
}

JITValueType quoteForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    return TypeInfer::toJITType(params[0]->getType());
}

JITValueType ifForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(if, 3);
    const TypeInfer infer1(params[1], env);
    const TypeInfer infer2(params[2], env);
    const auto ty1 = infer1.infer();
    const auto ty2 = infer2.infer();
    if (ty1 != ty2) {
        throw ValueError("if: inconsistent types");
    }
    return ty1;
}

JITValueType logicForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    for (const auto& param : params) {
        const TypeInfer infer(param, env);
        if (infer.infer() != JITValueType::BOOLEAN) {
            throw ValueError("and: expected boolean values");
        }
    }
    return JITValueType::BOOLEAN;
}

JITValueType lambdaForm(const std::vector<ValuePtr>&, EvalEnv*) {
    return JITValueType::FUNCTION;
}

JITValueType evalForm(const std::vector<ValuePtr>&, EvalEnv*) {
    throw ValueError("eval is not supported in JIT mode");
}

JITValueType condForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_EMPTY_PARAMS(cond);
    throw UnimplementedError("cond: not implemented");
}

}  // namespace jit