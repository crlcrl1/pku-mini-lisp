#ifndef TYPE_INFER_H
#define TYPE_INFER_H

#include "../value.h"
#include "jit.h"

namespace jit {
class TypeInfer {
    const Value* value;
    EvalEnv* env;

    JITValueType inferForm(const std::string& formName, const std::vector<ValuePtr>& args) const;
    JITValueType inferLambda(const JITFunctionValue* lambda,
                             const std::vector<ValuePtr>& args) const;

public:
    explicit TypeInfer(const Value* value, EvalEnv* env);

    static JITValueType toJITType(ValueType type);
    JITValueType infer() const;
    JITValueType inferReturnType() const;
};
}  // namespace jit

#endif  // TYPE_INFER_H
