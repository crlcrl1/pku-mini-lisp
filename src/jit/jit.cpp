#include "jit.h"

#include <llvm/IR/Function.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

#include <ranges>

#include "../error.h"
#include "../eval_env.h"
#include "../pool.h"
#include "type_infer.h"

void jit::initializeLLVM(EvalEnv* env) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
}

void jit::finalizeLLVM() {
    llvm::llvm_shutdown();
}

bool jit::JITFunctionValue::checkParamType(const std::vector<ValuePtr>& args) const {
    if (!paramTypes.has_value() || !returnType.has_value() || func == nullptr ||
        args.size() != paramTypes->size()) {
        return false;
    }
    return std::ranges::all_of(std::views::zip(*paramTypes, args),
                               [](const std::tuple<JITValueType, Value*>& t) static {
                                   auto [expected, actual] = t;
                                   return expected == TypeInfer::toJITType(actual->getType());
                               });
}

jit::JITValueType jit::JITFunctionValue::inferReturnType(const std::vector<ValuePtr>& args,
                                                         EvalEnv* env) const {
    TypeInfer infer(this, env);
    return infer.inferReturnType();
}

jit::JITFunctionValue::JITFunctionValue(std::vector<std::string> params, std::vector<ValuePtr> body,
                                        EvalEnv* env)
    : LambdaValue(std::move(params), std::move(body), env), func(nullptr) {}

jit::JITFunctionValue::JITFunctionValue(const LambdaValue& lambda)
    : LambdaValue(lambda), func(nullptr) {}

jit::JITFunctionValue::~JITFunctionValue() {
    if (func != nullptr) {
        this->func->eraseFromParent();
    }
}

void jit::JITFunctionValue::compile(const std::vector<ValuePtr>& args) const {
    if (func != nullptr) {
        return;
    }
    const auto newEnv = pool.makeEnv(env);
    for (size_t i = 0; i < params.size(); i++) {
        newEnv->addVariable(params[i], args[i]);
    }
    paramTypes = args | std::views::transform([](const ValuePtr& v) static {
                     return TypeInfer::toJITType(v->getType());
                 }) |
                 std::ranges::to<std::vector>();

    returnType = inferReturnType(args, newEnv);
    // TODO: IR generation
}

bool jit::JITFunctionValue::compiled() const {
    return func != nullptr;
}

std::optional<jit::JITValueType> jit::JITFunctionValue::getReturnType() const {
    return returnType;
}

ValuePtr jit::JITFunctionValue::apply(const std::vector<ValuePtr>& args) const {
    if (args.size() != params.size()) {
        throw ValueError(
            std::format("Expected {} arguments, but got {}", params.size(), args.size()));
    }
    // not compiled yet
    if (func == nullptr) {
        compile(args);
    }
    throw UnimplementedError("JITFunctionValue::apply");
}
