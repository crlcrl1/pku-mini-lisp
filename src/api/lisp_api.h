#ifndef LISP_API_H
#define LISP_API_H

#include "../error.h"
#include "../pool.h"
#include "../value.h"

#ifdef _WIN32
#define LISP_DLL_EXPORT __declspec(dllexport)
#else
#define LISP_DLL_EXPORT
#endif

#define LISP_API extern "C" LISP_DLL_EXPORT

#define LISP_BOOL(value) ValuePool::instance()->makeValue<BooleanValue>(value)
#define LISP_NUM(value) ValuePool::instance()->makeValue<NumericValue>(value)
#define LISP_NIL ValuePool::instance()->makeValue<NilValue>()
#define LISP_PAIR(...) ValuePool::instance()->makeValue<PairValue>(__VA_ARGS__)
#define LISP_STRING(str) ValuePool::instance()->makeValue<StringValue>(str)

#define LISP_EXT_INIT init_ext
#define LISP_REGISTER_PROC(name, func) registerProc(#name, func)

void registerProc(const std::string& name, BuiltinFuncType& func);

#endif  // LISP_API_H
