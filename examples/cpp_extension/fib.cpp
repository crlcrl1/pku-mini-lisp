#include <lisp_api.h>

LISP_API ValuePtr fib(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw ValueError("fib: expected one argument");
    }
    const auto n = params[0];
    if (n->getType() != ValueType::NUMBER) {
        throw ValueError("fib: expected a number as the argument");
    }
    const int num = static_cast<int>(*n->asNumber());
    if (num < 0) {
        throw ValueError("fib: expected a non-negative number as the argument");
    }
    if (num == 0) {
        return LISP_NUM(0);
    }
    if (num == 1) {
        return LISP_NUM(1);
    }
    double a = 0;
    double b = 1;
    for (int i = 2; i <= num; ++i) {
        const auto temp = b;
        b += a;
        a = temp;
    }
    return LISP_NUM(b);
}

LISP_API void LISP_EXT_INIT() {
    LISP_REGISTER_PROC(fib, fib);
}
