#include "jit.h"

#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

#include <utility>

void jit::initializeLLVM() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
}

void jit::finalizeLLVM() {
    llvm::llvm_shutdown();
}

jit::JITFunctionValue::JITFunctionValue(std::vector<std::string> params, std::vector<ValuePtr> body,
                                        EvalEnv* env)
    : LambdaValue(std::move(params), std::move(body), env) {}
