#ifndef UTILS_H
#define UTILS_H

#include <deque>

#include "value.h"

void removeTrailingNil(std::vector<ValuePtr>& values);

template <typename T>
std::deque<T> merge(std::deque<T> a, std::deque<T> b) {
    std::deque<T> result;
    for (auto& i : a) {
        result.push_back(std::move(i));
    }
    for (auto& i : b) {
        result.push_back(std::move(i));
    }
    return result;
}

#define CHECK_LIST(vec, name)                                                                 \
    if (vec.empty()) {                                                                        \
        throw ValueError(#name " requires a non-empty list as its argument", std::nullopt);   \
    }                                                                                         \
    if (vec.back()->getType() != ValueType::NIL) {                                            \
        throw ValueError(#name " requires a list as its argument", Location::fromRange(vec)); \
    }                                                                                         \
    vec.pop_back();

#define CHECK_PARAM_NUM(name, num)                                     \
    if (params.size() != num) {                                        \
        throw ValueError(#name " requires exactly " #num " arguments", \
                         Location::fromRange(params));                 \
    }

#define CHECK_EMPTY_PARAMS(name)                                                 \
    if (params.empty()) {                                                        \
        throw ValueError(#name " requires at least one argument", std::nullopt); \
    }

#define CHECK_TYPE(var, type, name, typeName)                                                   \
    if (var->getType() != ValueType::type) {                                                    \
        throw TypeError(#name " requires a " #typeName " as its argument", var->getLocation()); \
    }

#define LISP_BOOL(value) ValuePool::instance()->makeValue<BooleanValue>(value)
#define LISP_NUM(value) ValuePool::instance()->makeValue<NumericValue>(value)
#define LISP_NIL ValuePool::instance()->makeValue<NilValue>()
#define LISP_PAIR(...) ValuePool::instance()->makeValue<PairValue>(__VA_ARGS__)

#endif  // UTILS_H
