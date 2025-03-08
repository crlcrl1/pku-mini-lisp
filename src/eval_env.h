#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <memory>
#include <unordered_map>
#include <vector>

class Value;
using ValuePtr = std::shared_ptr<Value>;

class EvalEnv {
    std::unordered_map<std::string, ValuePtr> symbolTable;
    void addBuiltins();

public:
    EvalEnv();

    /**
     * Evaluate an expression
     * @param expr expression to evaluate
     * @return result of the evaluation
     */
    ValuePtr eval(ValuePtr expr);

    /**
     * Reset the environment
     */
    void reset();

    static ValuePtr apply(const ValuePtr& proc, const std::vector<ValuePtr>& args);

    std::vector<ValuePtr> evalList(const ValuePtr& expr);

    void addVariable(const std::string& name, const ValuePtr& value);
};

#endif  // EVAL_ENV_H
