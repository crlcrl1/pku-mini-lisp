#ifndef JIT_H
#define JIT_H

#include <string>
#include <vector>

#include "../value.h"

namespace llvm {
class Function;
}

class ValuePool;

namespace jit {

void initializeLLVM(EvalEnv* env);
void finalizeLLVM();

enum class JITValueType {
    NUMBER,
    LIST,
    FUNCTION,
    BOOLEAN,
    NIL,
};

class JITFunctionValue final : public LambdaValue {
    mutable std::optional<std::vector<JITValueType>> paramTypes = std::nullopt;
    mutable std::optional<JITValueType> returnType = std::nullopt;
    mutable llvm::Function* func;

    bool checkParamType(const std::vector<ValuePtr>& args) const;
    JITValueType inferReturnType(const std::vector<ValuePtr>& args, EvalEnv* env) const;

    JITFunctionValue(std::vector<std::string> params, std::vector<ValuePtr> body, EvalEnv* env);
    explicit JITFunctionValue(const LambdaValue& lambda);
    ~JITFunctionValue() override;

public:
    void compile(const std::vector<ValuePtr>& args) const;
    bool compiled() const;
    std::optional<JITValueType> getReturnType() const;
    ValuePtr apply(const std::vector<ValuePtr>& args) const override;

    friend class ::ValuePool;
};

}  // namespace jit

#endif  // JIT_H
