#include "api/lisp_api.h"

#include <string>

#include "eval_env.h"
#include "value.h"

void registerProc(const std::string& name, BuiltinFuncType& func) {
    pool.root()->addVariable(name, pool.makeValue<BuiltinProcValue>(func));
}