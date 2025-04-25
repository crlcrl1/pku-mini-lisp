#include "type_infer.h"

#include <ranges>

#include "../error.h"
#include "../eval_env.h"
#include "../forms.h"
#include "../utils.h"
#include "form_type_infer.h"

namespace jit {
// split the operators into different categories based on their return type
// TODO: add all builtin functions
const std::set<std::string> NUMBER_OPS = {"+",  "-",  "*", "/",  ">",  "<",
                                          ">=", "<=", "=", "!=", "car"};
const std::set<std::string> LIST_OPS = {"cdr", "cons", "append", "list"};
const std::set<std::string> BOOL_OPS = {"and", "or", "not", "eq?", "null?"};

JITValueType TypeInfer::inferForm(const std::string& formName,
                                  const std::vector<ValuePtr>& args) const {
    const auto inferFunc = FORM_INFER.find(formName);
    if (inferFunc == FORM_INFER.end()) {
        throw ValueError("Type infer: unknown form");
    }
    return inferFunc->second(args, env);
}

JITValueType TypeInfer::inferLambda(const JITFunctionValue* lambda,
                                    const std::vector<ValuePtr>& args) const {
    const auto param =
        args | std::views::transform([this](const ValuePtr& v) { return this->env->eval(v); }) |
        std::ranges::to<std::vector>();
    lambda->compile(param);
    return lambda->getReturnType().value();
}

JITValueType TypeInfer::toJITType(const ValueType type) {
    switch (type) {
        case ValueType::NUMBER:
            return JITValueType::NUMBER;
        case ValueType::PAIR:
            return JITValueType::LIST;
        case ValueType::BUILTIN:
        case ValueType::LAMBDA:
            return JITValueType::FUNCTION;
        case ValueType::BOOLEAN:
            return JITValueType::BOOLEAN;
        case ValueType::NIL:
            return JITValueType::NIL;
        default:
            throw ValueError("Unsupported type");
    }
}

TypeInfer::TypeInfer(const Value* value, EvalEnv* env) : value(value), env(env) {}

JITValueType TypeInfer::infer() const {
    const auto ty = value->getType();
    if (SELF_EVAL_VALUES.contains(ty) || ty == ValueType::LAMBDA || ty == ValueType::BUILTIN) {
        return toJITType(ty);
    }
    if (ty == ValueType::SYMBOL) {
        const auto symbol = dynamic_cast<const SymbolValue*>(value);
        if (const auto v = env->lookupBinding(symbol->getValue())) {
            return toJITType(v->getType());
        }
    }

    // the type must be ValueType::PAIR
    const auto pair = dynamic_cast<const PairValue*>(value);
    if (!pair) {
        throw InternalError("TypeInfer::infer: invalid type");
    }
    auto vec = pair->toVector();
    if (vec.empty()) {
        return JITValueType::NIL;
    }
    CHECK_LIST(vec, type infer);

    // function call
    const auto first = vec.front();
    // FIXME: Function may be evaluated rather than just looked up
    CHECK_TYPE(first, SYMBOL, type infer, symbol);
    const auto symbol = dynamic_cast<const SymbolValue*>(first);
    const auto funcName = symbol->getValue();

    // if the first element is a special form, infer the type using the rule of special forms
    if (SPECIAL_FORMS.contains(funcName)) {
        return inferForm(funcName, std::vector(vec.begin() + 1, vec.end()));
    }

    // otherwise, infer the type using the rule of functions
    const auto func = env->lookupBinding(funcName);
    if (!func || (func->getType() != ValueType::BUILTIN && func->getType() != ValueType::LAMBDA)) {
        throw ValueError("Type infer: unknown function");
    }

    // builtin functions
    if (func->getType() == ValueType::BUILTIN) {
        if (NUMBER_OPS.contains(funcName)) {
            return JITValueType::NUMBER;
        }
        if (LIST_OPS.contains(funcName)) {
            return JITValueType::LIST;
        }
        if (BOOL_OPS.contains(funcName)) {
            return JITValueType::BOOLEAN;
        }
    }

    // user-defined functions
    const auto lambda = dynamic_cast<const JITFunctionValue*>(func);
    if (lambda == nullptr) {
        throw ValueError("Type infer: jit function cannot call non-jit function");
    }
    return inferLambda(lambda, std::vector(vec.begin() + 1, vec.end()));
}

JITValueType TypeInfer::inferReturnType() const {
    auto jitFunc = dynamic_cast<const JITFunctionValue*>(value);
    if (jitFunc == nullptr) {
        throw ValueError("Type infer: expected a JITFunctionValue");
    }

    const auto lastStatement = jitFunc->getBody().back();
    if (lastStatement->getType() != ValueType::PAIR) {
        throw ValueError("Type infer: invalid function body");
    }
    const TypeInfer infer(lastStatement, env);
    return infer.infer();
}

}  // namespace jit