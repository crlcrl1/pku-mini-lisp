#ifndef JIT_H
#define JIT_H

#include <llvm/IR/Module.h>

#include "../value.h"

namespace jit {

void initializeLLVM();
void finalizeLLVM();

class JITFunctionValue : public LambdaValue {
    std::shared_ptr<llvm::Module> module;
    std::vector<std::string> cachedVariables;

public:
    explicit JITFunctionValue(std::vector<std::string> params, std::vector<ValuePtr> body,
                              EvalEnv* env);
    // ValuePtr apply(const std::vector<ValuePtr>& args) const override;
};

}  // namespace jit

#endif  // JIT_H
