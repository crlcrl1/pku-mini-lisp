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

#define CHECK_LIST(vec, name)                                                 \
    if (vec.empty()) {                                                        \
        throw ValueError(#name " requires a non-empty list as its argument"); \
    }                                                                         \
    if (vec.back()->getType() != ValueType::NIL) {                            \
        throw ValueError(#name " requires a list as its argument");           \
    }                                                                         \
    vec.pop_back();

#define CHECK_PARAM_NUM(name, num)                                      \
    if (params.size() != num) {                                         \
        throw ValueError(#name " requires exactly " #num " arguments"); \
    }

#define CHECK_EMPTY_PARAMS(name)                                   \
    if (params.empty()) {                                          \
        throw ValueError(#name " requires at least one argument"); \
    }

#define LISP_BOOL(value) std::make_shared<BooleanValue>(value)
#define LISP_NUM(value) std::make_shared<NumericValue>(value)
#define LISP_NIL std::make_shared<NilValue>()
#define LISP_PAIR(car, cdr) std::make_shared<PairValue>(car, cdr)

#endif  // UTILS_H
