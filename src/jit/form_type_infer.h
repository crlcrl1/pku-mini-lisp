#ifndef FORM_TYPE_INFER_H
#define FORM_TYPE_INFER_H

#include <string>
#include <unordered_map>
#include <vector>

class Value;
class EvalEnv;

namespace jit {

enum class JITValueType;

using ValuePtr = Value*;
using FormInferFunc = JITValueType(const std::vector<ValuePtr>&, EvalEnv*);

extern const std::unordered_map<std::string, FormInferFunc*> FORM_INFER;

JITValueType defineForm(const std::vector<ValuePtr>& params, EvalEnv* env);

JITValueType quoteForm(const std::vector<ValuePtr>& params, EvalEnv* env);

JITValueType ifForm(const std::vector<ValuePtr>& params, EvalEnv* env);

JITValueType logicForm(const std::vector<ValuePtr>& params, EvalEnv* env);

JITValueType lambdaForm(const std::vector<ValuePtr>& params, EvalEnv* env);

JITValueType evalForm(const std::vector<ValuePtr>& params, EvalEnv* env);

JITValueType condForm(const std::vector<ValuePtr>& params, EvalEnv* env);

}  // namespace jit

#endif  // FORM_TYPE_INFER_H
