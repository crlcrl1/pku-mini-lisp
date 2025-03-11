#ifndef TYPES_H
#define TYPES_H

namespace jit {

struct Value;

enum class ValueType {
    BOOLEAN,
    NUMBER,
    STRING,
    NIL,
    PAIR,
    FUNC,
};

struct Pair {
    Value *car;
    Value *cdr;
};

union Data {
    bool boolean;
    double number;
    char *string;
    Pair pair;
    void *func;
};

struct Value {
    ValueType ty;
    Data data;
};

}  // namespace jit

#endif  // TYPES_H
