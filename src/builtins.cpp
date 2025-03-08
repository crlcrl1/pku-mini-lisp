#include "builtins.h"

#include <iostream>

#include "error.h"
#include "eval_env.h"

#define BUILTIN_PAIR(procName, builtinName) \
    std::make_pair(#builtinName, std::make_shared<BuiltinProcValue>(&procName))

// clang-format off
const std::unordered_map<std::string, ValuePtr> BUILTINS = {
    BUILTIN_PAIR(builtins::add, +),
    BUILTIN_PAIR(builtins::sub, -),
    BUILTIN_PAIR(builtins::mul, *),
    BUILTIN_PAIR(builtins::div, /),
    BUILTIN_PAIR(builtins::eq, =),
    BUILTIN_PAIR(builtins::lt, <),
    BUILTIN_PAIR(builtins::gt, >),
    BUILTIN_PAIR(builtins::le, <=),
    BUILTIN_PAIR(builtins::ge, >=),
    BUILTIN_PAIR(builtins::apply, apply),
    BUILTIN_PAIR(builtins::display, display),
    BUILTIN_PAIR(builtins::print, print),
    BUILTIN_PAIR(builtins::exit, exit),
    BUILTIN_PAIR(builtins::length, length),
    BUILTIN_PAIR(builtins::car, car),
    BUILTIN_PAIR(builtins::cdr, cdr),
};
// clang-format on

#define BUILTIN_NUMBER_CHECK(var, op)                            \
    if (!var->isNumber()) {                                      \
        throw ValueError("Cannot " #op " a non-numeric value."); \
    }

#define CHECK_EMPTY_PARAMS(name)                                   \
    if (params.empty()) {                                          \
        throw ValueError(#name " requires at least one argument"); \
    }

#define BUILTIN_BINARY_OP(name, op, displayName, initValue, returnTy)                             \
    ValuePtr name(const std::vector<ValuePtr>& params) {                                          \
        CHECK_EMPTY_PARAMS(displayName)                                                           \
        const size_t numParams = params.size();                                                   \
        if (numParams == 1) {                                                                     \
            BUILTIN_NUMBER_CHECK(params[0], subtract);                                            \
            return std::make_shared<returnTy>(initValue op * params[0]->asNumber());              \
        }                                                                                         \
        if (numParams == 2) {                                                                     \
            BUILTIN_NUMBER_CHECK(params[0], subtract);                                            \
            BUILTIN_NUMBER_CHECK(params[1], subtract);                                            \
            return std::make_shared<returnTy>(*params[0]->asNumber() op * params[1]->asNumber()); \
        }                                                                                         \
        throw ValueError(#displayName " requires exactly one or two arguments.");                 \
    }

#define BUILTIN_MULTI_OP(name, op, displayName, initValue) \
    ValuePtr name(const std::vector<ValuePtr>& params) {   \
        double result = initValue;                         \
        for (const auto& i : params) {                     \
            BUILTIN_NUMBER_CHECK(i, multiply);             \
            result = result op * i->asNumber();            \
        }                                                  \
        return std::make_shared<NumericValue>(result);     \
    }

BUILTIN_MULTI_OP(builtins::add, +, Addition, 0.0)
BUILTIN_MULTI_OP(builtins::mul, *, Multiplication, 1.0)

BUILTIN_BINARY_OP(builtins::sub, -, Subtraction, 0.0, NumericValue)
BUILTIN_BINARY_OP(builtins::div, /, Division, 1.0, NumericValue)
BUILTIN_BINARY_OP(builtins::eq, ==, Equality, 0.0, BooleanValue)
BUILTIN_BINARY_OP(builtins::lt, <, Less than, 0.0, BooleanValue)
BUILTIN_BINARY_OP(builtins::gt, >, Greater than, 0.0, BooleanValue)
BUILTIN_BINARY_OP(builtins::le, <=, Greater than, 0.0, BooleanValue)
BUILTIN_BINARY_OP(builtins::ge, >=, Greater than, 0.0, BooleanValue)

ValuePtr builtins::apply(const std::vector<ValuePtr>& params) {
    CHECK_EMPTY_PARAMS(apply)
    return EvalEnv::apply(params[0], std::vector(params.begin() + 1, params.end()));
}

ValuePtr builtins::display(const std::vector<ValuePtr>& params) {
    for (const auto& i : params) {
        std::cout << i->toString() << std::endl;
    }
    return std::make_shared<NilValue>();
}

ValuePtr builtins::print(const std::vector<ValuePtr>& params) {
    for (const auto& i : params) {
        std::cout << i->toString() << ' ';
    }
    std::cout << std::endl;
    return std::make_shared<NilValue>();
}

ValuePtr builtins::exit(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        std::exit(0);
    }
    if (params.size() == 1) {
        BUILTIN_NUMBER_CHECK(params[0], exit);
        std::exit(static_cast<int>(*params[0]->asNumber()));
    }
    throw ValueError("exit requires zero or one argument");
}

ValuePtr builtins::length(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw ValueError("length requires exactly one argument");
    }
    if (params[0]->getType() != ValueType::PAIR) {
        throw ValueError("length requires a list as its argument");
    }
    const auto first = dynamic_cast<PairValue*>(params[0].get());
    const auto values = first->toVector();
    size_t size = values.size();
    if (values.back()->getType() == ValueType::NIL) {
        size--;
    }
    return std::make_shared<NumericValue>(size);
}

ValuePtr builtins::car(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw ValueError("car requires exactly one argument");
    }
    if (params[0]->getType() != ValueType::PAIR) {
        throw ValueError("car requires a list as its argument");
    }
    const auto first = dynamic_cast<PairValue*>(params[0].get());
    return first->getCar();
}

ValuePtr builtins::cdr(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw ValueError("cdr requires exactly one argument");
    }
    if (params[0]->getType() != ValueType::PAIR) {
        throw ValueError("cdr requires a list as its argument");
    }
    const auto first = dynamic_cast<PairValue*>(params[0].get());
    return first->getCdr();
}