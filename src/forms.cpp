#include "forms.h"

#include <ranges>

#include "error.h"
#include "pool.h"
#include "utils.h"
#include "value.h"

#ifdef USE_LLVM
#include "jit/jit.h"
#endif

// clang-format off
const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS = {
    {"define", &defineForm},
    {"quote", &quoteForm},
    {"if", &ifForm},
    {"and", &andForm},
    {"or", &orForm},
    {"lambda", &lambdaForm},
    {"eval", &evalForm},
    {"cond", &condForm},
    {"begin", &beginForm},
    {"let", &letForm},
    {"quasiquote", &quasiquoteForm},
    {"jit", &jitForm},
};
// clang-format on

ValuePtr defineForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    if (const auto name = params[0]->asSymbolName(); name.has_value()) {
        env->addVariable(*name, env->eval(params[1]));
    } else if (const auto pair = dynamic_cast<PairValue*>(params[0]); pair != nullptr) {
        const auto procName = pair->getCar()->asSymbolName();
        if (!procName.has_value()) {
            throw ValueError("define: expected a symbol as the first argument");
        }
        std::vector procParams = {pair->getCdr()};
        procParams.insert(procParams.end(), params.begin() + 1, params.end());
        const auto lambda = lambdaForm(procParams, env);
        env->addVariable(*procName, lambda);
    } else {
        throw ValueError("define: expected a symbol or a pair as the first argument");
    }
    return LISP_NIL;
}

ValuePtr quoteForm(const std::vector<ValuePtr>& params, EvalEnv*) {
    CHECK_PARAM_NUM(quote, 1);
    return params[0];
}

bool convertToBool(const ValuePtr& value) {
    if (value->getType() == ValueType::BOOLEAN) {
        return dynamic_cast<BooleanValue*>(value)->getValue();
    }
    return true;
}

ValuePtr ifForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(if, 3);
    if (const auto condition = env->eval(params[0]); convertToBool(condition)) {
        return env->eval(params[1]);
    }
    if (params.size() == 3) {
        return env->eval(params[2]);
    }
    return LISP_NIL;
}

ValuePtr andForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    for (const auto& param : params) {
        if (const auto value = env->eval(param); !convertToBool(value)) {
            return LISP_BOOL(false);
        }
    }
    const size_t len = params.size();
    if (len == 0) {
        return LISP_BOOL(true);
    }
    return env->eval(params[len - 1]);
}

ValuePtr orForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    for (const auto& arg : params) {
        if (const auto value = env->eval(arg); convertToBool(value)) {
            return value;
        }
    }
    return LISP_BOOL(false);
}

ValuePtr lambdaForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_EMPTY_PARAMS(lambda);
    const auto lambdaParams = dynamic_cast<PairValue*>(params[0]);
    const auto body = std::vector(params.begin() + 1, params.end());
    if (lambdaParams == nullptr && params[0]->getType() != ValueType::NIL) {
        throw ValueError("lambda: expected a pair of parameters");
    }
    if (lambdaParams == nullptr) {  // no parameters
        return pool.makeValue<LambdaValue>(std::vector<std::string>{}, body, env);
    }
    auto paramsList = std::vector<std::string>{};
    auto paramsVec = lambdaParams->toVector();
    removeTrailingNil(paramsVec);

    for (const auto& param : paramsVec) {
        if (const auto name = param->asSymbolName(); name.has_value()) {
            paramsList.push_back(*name);
        } else {
            throw ValueError("lambda: expected a list of symbols as parameters");
        }
    }

    return pool.makeValue<LambdaValue>(paramsList, body, env);
}

ValuePtr evalForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(eval, 1);
    return env->eval(env->eval(params[0]));
}

ValuePtr condForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_EMPTY_PARAMS(cond);
    const size_t len = params.size();
    for (auto [i, v] : std::views::enumerate(params)) {
        CHECK_TYPE(v, PAIR, cond, list);
        const auto pair = dynamic_cast<PairValue*>(v);
        auto pairVec = pair->toVector();
        // check if the argument is a proper list
        CHECK_LIST(pairVec, cond);

        bool flag = false;
        ValuePtr result;
        if (i == len - 1 && pairVec[0]->asSymbolName() == "else") {
            flag = true;
            result = LISP_BOOL(true);
        } else {
            const auto cond = env->eval(pairVec[0]);
            if (cond->getType() != ValueType::BOOLEAN) {
                flag = true;  // any value other than #f is considered true
            } else {
                flag = dynamic_cast<BooleanValue*>(cond)->getValue();
            }
            result = cond;
        }
        if (flag) {
            if (pairVec.size() == 1) {  // no consequent, return the condition
                return result;
            }

            for (const auto& expr : std::vector(pairVec.begin() + 1, pairVec.end())) {
                result = env->eval(expr);
            }
            return result;
        }
        if (i == len - 1) {
            return result;
        }
    }
    throw InternalError("cond: unexpected error");
}

ValuePtr beginForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    ValuePtr result = LISP_NIL;
    for (const auto& expr : params) {
        result = env->eval(expr);
    }
    return result;
}

ValuePtr letForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_EMPTY_PARAMS(let);
    CHECK_TYPE(params[0], PAIR, let, pair);

    // add variables to the environment
    const auto varPair = dynamic_cast<PairValue*>(params[0]);
    auto varVec = varPair->toVector();
    if (varVec.back()->getType() != ValueType::NIL) {
        throw ValueError("let expected a list as the first argument");
    }
    varVec.pop_back();
    std::unordered_map<std::string, std::optional<ValuePtr>> oldVarMap;
    std::unordered_map<std::string, ValuePtr> newVarMap;
    for (const auto& var : varVec) {
        CHECK_TYPE(var, PAIR, let, pair);
        const auto pair = dynamic_cast<PairValue*>(var);
        auto pairVec = pair->toVector();
        if (pairVec.size() != 3 || pairVec[2]->getType() != ValueType::NIL) {
            throw ValueError("let: expected a list as the argument");
        }
        auto name = pairVec[0]->asSymbolName();
        if (!name.has_value()) {
            throw ValueError("let: expected a symbol as the first element of the list");
        }
        auto value = env->eval(pairVec[1]);
        newVarMap[*name] = value;
    }
    // create a new environment
    auto newEnv = pool.makeEnv(env);
    for (const auto& [name, value] : newVarMap) {
        newEnv->addVariable(name, value);
    }

    // evaluate the body
    ValuePtr result = LISP_NIL;
    for (const auto& expr : std::vector(params.begin() + 1, params.end())) {
        result = newEnv->eval(expr);
    }

    return result;
}

ValuePtr quasiquoteForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(quasiquote, 1);
    CHECK_TYPE(params[0], PAIR, quasiquote, pair);
    std::vector<ValuePtr> result;

    // convert the pair to a vector
    const auto argPair = dynamic_cast<PairValue*>(params[0]);
    auto argVec = argPair->toVector();
    if (argVec.empty()) {
        return params[0];
    }
    CHECK_LIST(argVec, quasiquote);
    for (const auto& arg : argVec) {
        if (arg->getType() == ValueType::PAIR) {
            const auto pair = dynamic_cast<PairValue*>(arg);
            if (pair->getCar()->asSymbolName() == "unquote") {
                CHECK_TYPE(pair->getCdr(), PAIR, quasiquote, pair);
                const auto unquoteArg = dynamic_cast<PairValue*>(pair->getCdr())->getCar();
                result.push_back(env->eval(unquoteArg));
                continue;
            }
        }
        result.push_back(arg);
    }
    return LISP_PAIR(PairValue::fromVector(result));
}

ValuePtr jitForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(jit, 1);
    ValuePtr func = params[0];
    CHECK_TYPE(func, SYMBOL, jit, symbol);
    ValuePtr result = env->eval(func);
#ifndef USE_LLVM
    return result;
#else
    const LambdaValue* lambda = dynamic_cast<LambdaValue*>(result);
    if (lambda == nullptr) {
        throw ValueError("jit: expected a LambdaValue");
    }
    const auto jitFunc = pool.makeValue<jit::JITFunctionValue>(*lambda);
    return jitFunc;
#endif
}
