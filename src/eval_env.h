#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#ifdef USE_LLVM
namespace llvm {
class Module;
class LLVMContext;
class ExecutionEngine;
}  // namespace llvm
#endif

#include <memory>
#include <unordered_map>
#include <vector>

class Value;
using ValuePtr = Value*;
using ConstValuePtr = const Value*;

class EvalEnv {
    std::unordered_map<std::string, ValuePtr> symbolTable;
    const EvalEnv* parent;

#ifdef USE_LLVM
    std::shared_ptr<llvm::LLVMContext> context;
    llvm::Module* module;
    llvm::ExecutionEngine* engine;
#endif

    void addBuiltins();

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
    ValuePtr lookupBinding(const std::string& name) const;

    friend class ValuePool;
};

#endif  // EVAL_ENV_H
