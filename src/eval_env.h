#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <memory>
#include <unordered_map>
#include <vector>

class Value;
using ValuePtr = std::shared_ptr<Value>;

class EvalEnv : public std::enable_shared_from_this<EvalEnv> {
    std::unordered_map<std::string, ValuePtr> symbolTable;
    std::shared_ptr<EvalEnv> parent;
    void addBuiltins();
    ValuePtr lookupBinding(const std::string& name);

    EvalEnv();

    explicit EvalEnv(std::shared_ptr<EvalEnv> parent);

public:
    static std::shared_ptr<EvalEnv> createEnv();
    static std::shared_ptr<EvalEnv> createEnv(std::shared_ptr<EvalEnv> parent);

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

    std::shared_ptr<EvalEnv> createChild(const std::vector<std::string>& params,
                                         const std::vector<ValuePtr>& args);
};

#endif  // EVAL_ENV_H
