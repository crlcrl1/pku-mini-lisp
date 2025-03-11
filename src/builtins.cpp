#include "builtins.h"

#include <cmath>
#include <iostream>

#include "error.h"
#include "eval_env.h"
#include "pool.h"
#include "utils.h"

#define BUILTIN_NUMBER_CHECK(var, op)                            \
    if (!var->isNumber()) {                                      \
        throw ValueError("Cannot " #op " a non-numeric value."); \
    }

#define BUILTIN_BINARY_OP(name, op, displayName, initValue, returnTy)                           \
    ValuePtr name(const std::vector<ValuePtr>& params) {                                        \
        CHECK_EMPTY_PARAMS(displayName)                                                         \
        const size_t numParams = params.size();                                                 \
        if (numParams == 1) {                                                                   \
            BUILTIN_NUMBER_CHECK(params[0], subtract);                                          \
            return pool.makeValue<returnTy>(initValue op * params[0]->asNumber());              \
        }                                                                                       \
        if (numParams == 2) {                                                                   \
            BUILTIN_NUMBER_CHECK(params[0], subtract);                                          \
            BUILTIN_NUMBER_CHECK(params[1], subtract);                                          \
            return pool.makeValue<returnTy>(*params[0]->asNumber() op * params[1]->asNumber()); \
        }                                                                                       \
        throw ValueError(#displayName " requires exactly one or two arguments.");               \
    }

#define BUILTIN_MULTI_OP(name, op, displayName, initValue) \
    ValuePtr name(const std::vector<ValuePtr>& params) {   \
        double result = initValue;                         \
        for (const auto& i : params) {                     \
            BUILTIN_NUMBER_CHECK(i, displayName);          \
            result = result op * i->asNumber();            \
        }                                                  \
        return LISP_NUM(result);                           \
    }

#define BUILT_IN_IS_TYPE(name, type)                               \
    ValuePtr builtins::name(const std::vector<ValuePtr>& params) { \
        CHECK_PARAM_NUM(name?, 1);                                 \
        return LISP_BOOL(params[0]->getType() == ValueType::type); \
    }

const std::set LITERAL_TYPE = {ValueType::BOOLEAN, ValueType::NUMBER, ValueType::NIL,
                               ValueType::SYMBOL};

BUILTIN_MULTI_OP(builtins::add, +, add, 0.0)
BUILTIN_MULTI_OP(builtins::mul, *, multiply, 1.0)

BUILTIN_BINARY_OP(builtins::sub, -, Subtraction, 0.0, NumericValue)
BUILTIN_BINARY_OP(builtins::div, /, Division, 1.0, NumericValue)
BUILTIN_BINARY_OP(builtins::eq, ==, Equality, 0.0, BooleanValue)
BUILTIN_BINARY_OP(builtins::lt, <, Less than, 0.0, BooleanValue)
BUILTIN_BINARY_OP(builtins::gt, >, Greater than, 0.0, BooleanValue)
BUILTIN_BINARY_OP(builtins::le, <=, Greater than, 0.0, BooleanValue)
BUILTIN_BINARY_OP(builtins::ge, >=, Greater than, 0.0, BooleanValue)

ValuePtr builtins::apply(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(apply, 2);
    CHECK_TYPE(params[1], PAIR, apply, list);
    auto param = dynamic_cast<PairValue*>(params[1]);
    auto paramList = param->toVector();
    removeTrailingNil(paramList);
    return EvalEnv::apply(params[0], paramList);
}

ValuePtr builtins::display(const std::vector<ValuePtr>& params) {
    for (const auto& i : params) {
        std::cout << i->toString() << ' ';
    }
    return LISP_NIL;
}

ValuePtr builtins::displayln(const std::vector<ValuePtr>& params) {
    display(params);
    std::cout << std::endl;
    return LISP_NIL;
}

ValuePtr builtins::newline(const std::vector<ValuePtr>& params) {
    if (!params.empty()) {
        throw ValueError("newline requires no argument");
    }
    std::cout << std::endl;
    return LISP_NIL;
}

ValuePtr builtins::error(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        std::exit(1);
    }
    if (params.size() != 1) {
        throw ValueError("error requires exactly one argument or none");
    }
    if (const auto value = params[0]->asNumber()) {
        std::exit(static_cast<int>(*value));
    }
    throw ValueError("error requires a number as its argument");
}

ValuePtr builtins::print(const std::vector<ValuePtr>& params) {
    for (const auto& i : params) {
        std::cout << i->toString() << ' ';
    }
    std::cout << std::endl;
    return LISP_NIL;
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
    CHECK_PARAM_NUM(length, 1);
    if (params[0]->getType() == ValueType::NIL) {
        return LISP_NUM(0);
    }
    CHECK_TYPE(params[0], PAIR, length, list);
    const auto first = dynamic_cast<PairValue*>(params[0]);
    const auto values = first->toVector();
    size_t size = values.size();
    if (values.back()->getType() == ValueType::NIL) {
        size--;
    }
    return LISP_NUM(size);
}

ValuePtr builtins::car(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(car, 1);
    CHECK_TYPE(params[0], PAIR, car, list);
    const auto first = dynamic_cast<PairValue*>(params[0]);
    return first->getCar();
}

ValuePtr builtins::cdr(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(cdr, 1);
    CHECK_TYPE(params[0], PAIR, cdr, list);
    const auto first = dynamic_cast<PairValue*>(params[0]);
    return first->getCdr();
}

ValuePtr builtins::atom(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(atom?, 1);
    return LISP_BOOL(params[0]->isAtom());
}

ValuePtr builtins::integer(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(integer?, 1);
    if (!params[0]->isNumber()) {
        return LISP_BOOL(false);
    }
    const double value = *params[0]->asNumber();
    return LISP_BOOL(std::abs(static_cast<int>(value) - value) < 1e-7);
}

ValuePtr builtins::isList(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(list?, 1);
    if (params[0]->getType() == ValueType::NIL) {
        return LISP_BOOL(true);
    }
    if (params[0]->getType() != ValueType::PAIR) {
        return LISP_BOOL(false);
    }
    const auto arg = dynamic_cast<PairValue*>(params[0]);
    if (arg == nullptr) {
        throw ValueError("list? requires a pair as its argument");
    }
    const auto vec = arg->toVector();
    return LISP_BOOL(vec.back()->getType() == ValueType::NIL);
}

BUILT_IN_IS_TYPE(boolean, BOOLEAN)
BUILT_IN_IS_TYPE(number, NUMBER)
BUILT_IN_IS_TYPE(null, NIL)

ValuePtr builtins::procedure(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(procedure?, 1);
    return LISP_BOOL(params[0]->getType() == ValueType::LAMBDA ||
                     params[0]->getType() == ValueType::BUILTIN);
}

BUILT_IN_IS_TYPE(string, STRING)
BUILT_IN_IS_TYPE(symbol, SYMBOL)
BUILT_IN_IS_TYPE(pair, PAIR)

ValuePtr builtins::append(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        return LISP_NIL;
    }
    std::vector<ValuePtr> result = {};
    for (const auto& i : params) {
        if (i->getType() == ValueType::NIL) {
            continue;
        }
        CHECK_TYPE(i, PAIR, append, list);
        const auto pair = dynamic_cast<PairValue*>(i);
        auto vec = pair->toVector();
        CHECK_LIST(vec, append);
        result.insert(result.end(), vec.begin(), vec.end());
    }
    return LISP_PAIR(PairValue::fromVector(result));
}

ValuePtr builtins::cons(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(cons, 2);
    return LISP_PAIR(params[0], params[1]);
}

ValuePtr builtins::makeList(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        return LISP_NIL;
    }
    return LISP_PAIR(PairValue::fromVector(params));
}

ValuePtr builtins::map(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(map, 2);
    CHECK_TYPE(params[1], PAIR, map, list);
    const auto& func = params[0];
    const auto arg = dynamic_cast<PairValue*>(params[1]);
    auto vec = arg->toVector();
    CHECK_LIST(vec, map);
    std::vector<ValuePtr> result;
    for (const auto& i : vec) {
        result.push_back(EvalEnv::apply(func, {i}));
    }
    return LISP_PAIR(PairValue::fromVector(result));
}

ValuePtr builtins::filter(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(filter, 2);
    CHECK_TYPE(params[1], PAIR, filter, list);
    const auto& func = params[0];
    const auto arg = dynamic_cast<PairValue*>(params[1]);
    auto vec = arg->toVector();
    CHECK_LIST(vec, filter);
    std::vector<ValuePtr> result;
    for (const auto& i : vec) {
        auto res = EvalEnv::apply(func, {i});
        if (res->getType() != ValueType::BOOLEAN) {
            throw ValueError("filter function must return a boolean value");
        }
        if (dynamic_cast<BooleanValue*>(res)->getValue()) {
            result.push_back(i);
        }
    }
    return LISP_PAIR(PairValue::fromVector(result));
}

ValuePtr builtins::reduce(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(reduce, 2);
    CHECK_TYPE(params[1], PAIR, reduce, list);
    const auto& func = params[0];
    const auto arg = dynamic_cast<PairValue*>(params[1]);
    auto cdr = arg->getCdr();
    if (cdr->getType() == ValueType::NIL) {
        return arg->getCar();
    }
    return EvalEnv::apply(func, {arg->getCar(), reduce({func, cdr})});
}

ValuePtr builtins::abs(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(abs, 1);
    CHECK_TYPE(params[0], NUMBER, abs, number);
    return LISP_NUM(std::abs(*params[0]->asNumber()));
}

ValuePtr builtins::expt(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(expt, 2);
    if (!params[0]->isNumber() || !params[1]->isNumber()) {
        throw ValueError("expt requires two numbers as its arguments");
    }
    return LISP_NUM(std::pow(*params[0]->asNumber(), *params[1]->asNumber()));
}

ValuePtr builtins::quotient(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(quotient, 2);
    if (!params[0]->isNumber() || !params[1]->isNumber()) {
        throw ValueError("quotient requires two numbers as its arguments");
    }
    const double res = *params[0]->asNumber() / *params[1]->asNumber();
    return LISP_NUM(res >= 0 ? std::floor(res) : std::ceil(res));
}

ValuePtr builtins::modulo(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(modulo, 2);
    if (!params[0]->isNumber() || !params[1]->isNumber()) {
        throw ValueError("modulo requires two numbers as its arguments");
    }
    const double a = *params[0]->asNumber();
    const double b = *params[1]->asNumber();
    if (a * b > 0) {
        return LISP_NUM(std::fmod(a, b));
    }
    return LISP_NUM(b + std::fmod(a, b));
}

ValuePtr builtins::remainder(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(remainder, 2);
    if (!params[0]->isNumber() || !params[1]->isNumber()) {
        throw ValueError("remainder requires two numbers as its arguments");
    }
    const double a = *params[0]->asNumber();
    const double b = *params[1]->asNumber();
    return LISP_NUM(a - std::trunc(a / b) * b);
}

ValuePtr builtins::loceq(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(loceq, 2);
    if (params[0]->getType() != params[1]->getType()) {
        return LISP_BOOL(false);
    }
    if (LITERAL_TYPE.contains(params[0]->getType())) {
        return LISP_BOOL(params[0]->equals(params[1]));
    }

    return LISP_BOOL(params[0] == params[1]);
}

ValuePtr builtins::dataeq(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(dataeq, 2);
    return LISP_BOOL(params[0]->equals(params[1]));
}

ValuePtr builtins::logicalNot(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(logicalNot, 1);
    // All non-boolean values are considered true
    if (params[0]->getType() != ValueType::BOOLEAN) {
        return LISP_BOOL(false);
    }
    return LISP_BOOL(!dynamic_cast<BooleanValue*>(params[0])->getValue());
}

ValuePtr builtins::even(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(even, 1);
    CHECK_TYPE(params[0], NUMBER, even, number);
    return LISP_BOOL(std::abs(std::remainder(*params[0]->asNumber(), 2)) <= 1e-7);
}

ValuePtr builtins::odd(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(odd, 1);
    CHECK_TYPE(params[0], NUMBER, odd, number);
    return LISP_BOOL(std::abs(std::abs(std::remainder(*params[0]->asNumber(), 2)) - 1) <= 1e-7);
}

ValuePtr builtins::zero(const std::vector<ValuePtr>& params) {
    CHECK_PARAM_NUM(zero, 1);
    CHECK_TYPE(params[0], NUMBER, zero, number);
    return LISP_BOOL(std::abs(*params[0]->asNumber()) <= 1e-7);
}
