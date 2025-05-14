#include <lisp_api.h>

#include <algorithm>
#include <ranges>

LISP_API ValuePtr quick_sort(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw ValueError("quick_sort: expected one argument", Location::fromRange(params));
    }
    const auto list = params[0];
    if (list->getType() != ValueType::PAIR) {
        throw ValueError("quick_sort: expected a list as the argument",
                         Location::fromRange(params));
    }
    const auto pair = dynamic_cast<PairValue*>(list);
    auto inputVec = pair->toVector();
    if (inputVec.empty()) {
        return LISP_NIL;
    }
    if (inputVec.back()->getType() != ValueType::NIL) {
        throw ValueError("quick_sort: expected a list", Location::fromRange(params));
    }
    inputVec.pop_back();
    auto vec =
        inputVec | std::views::transform([](ValuePtr v) static {
            if (v->getType() != ValueType::NUMBER) {
                throw ValueError("quick_sort: expected a number in the list", v->getLocation());
            }
            return *v->asNumber();
        }) |
        std::ranges::to<std::vector>();
    std::ranges::sort(vec);
    const auto res =
        vec |
        std::views::transform([](double v) static { return static_cast<ValuePtr>(LISP_NUM(v)); }) |
        std::ranges::to<std::vector>();
    return LISP_PAIR(PairValue::fromVector(res));
}

LISP_API void LISP_EXT_INIT() {
    LISP_REGISTER_PROC(quick_sort, quick_sort);
}