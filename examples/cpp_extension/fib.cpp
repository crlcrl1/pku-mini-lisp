#include <lisp_api.h>

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

LISP_API ValuePtr fib(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw ValueError("fib: expected one argument", Location::fromRange(params));
    }
    const auto n = params[0];
    if (n->getType() != ValueType::NUMBER) {
        throw ValueError("fib: expected a number as the argument", n->getLocation());
    }
    const int num = static_cast<int>(*n->asNumber());
    if (num < 0) {
        throw ValueError("fib: expected a non-negative number as the argument", n->getLocation());
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

LISP_API ValuePtr fib_recursive(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw ValueError("fib: expected one argument", Location::fromRange(params));
    }
    const auto n = params[0];
    if (n->getType() != ValueType::NUMBER) {
        throw ValueError("fib: expected a number as the argument", n->getLocation());
    }
    const int num = static_cast<int>(*n->asNumber());
    if (num < 0) {
        throw ValueError("fib: expected a non-negative number as the argument", n->getLocation());
    }
    return LISP_NUM(fibonacci(num));
}

LISP_API void LISP_EXT_INIT() {
    LISP_REGISTER_PROC(fib, fib);
    LISP_REGISTER_PROC(fib_recursive, fib_recursive);
}
