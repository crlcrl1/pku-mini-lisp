#ifndef UTILS_H
#define UTILS_H

#include "value.h"

void removeTrailingNil(std::vector<ValuePtr>& values);

#define CHECK_LIST(vec, name)                                                 \
    if (vec.empty()) {                                                        \
        throw ValueError(#name " requires a non-empty list as its argument"); \
    }                                                                         \
    if (vec.back()->getType() != ValueType::NIL) {                            \
        throw ValueError(#name " requires a list as its argument");           \
    }                                                                         \
    vec.pop_back();

#endif  // UTILS_H
