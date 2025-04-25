#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Value;
using ValuePtr = Value*;

class EvalEnv {
    std::unordered_map<std::string, ValuePtr> symbolTable;
    const EvalEnv* parent;

    void addBuiltins();
    ValuePtr lookupBinding(const std::string& name) const;

    EvalEnv();
    explicit EvalEnv(const EvalEnv* parent);

public:
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

    /**
     * Add a variable to the environment and return the old value if it exists
     *
     * @param name variable name
     * @param value variable value
     * @return old value if it exists, std::nullopt otherwise
     */
    std::optional<ValuePtr> addVariable(const std::string& name, const ValuePtr& value);

    bool removeVariable(const std::string& name);

    friend class ValuePool;
};

extern std::vector<std::string> loadStack;

#endif  // EVAL_ENV_H
