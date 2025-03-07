#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <memory>
#include <unordered_map>

class Value;
using ValuePtr = std::shared_ptr<Value>;

class EvalEnv {
    std::unordered_map<std::string, ValuePtr> symbolTable;

public:
    EvalEnv() = default;
    ValuePtr eval(ValuePtr expr);
    void reset();
};

#endif  // EVAL_ENV_H
