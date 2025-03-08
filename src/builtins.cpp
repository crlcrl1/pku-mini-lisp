#include "builtins.h"

#include <iostream>

#include "error.h"

#define BUILTIN_NUMBER_CHECK(var, op)                            \
    if (!var->isNumber()) {                                      \
        throw ValueError("Cannot " #op " a non-numeric value."); \
    }

#define BUILTIN_BINARY_OP(name, op, displayName, initValue, returnTy)                             \
    ValuePtr name(const std::vector<ValuePtr>& params) {                                          \
        if (params.empty()) {                                                                     \
            throw ValueError(#displayName " requires exactly one or two arguments.");             \
        }                                                                                         \
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
    throw UnimplementedError("apply");
}

ValuePtr builtins::display(const std::vector<ValuePtr>& params) {
    for (const auto& i : params) {
        std::cout << i->toString() << std::endl;
    }
    return std::make_shared<NilValue>();
}

ValuePtr builtins::print(const std::vector<ValuePtr>& params) {
    for (const auto& i : params) {
        std::cout << i->toString();
    }
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