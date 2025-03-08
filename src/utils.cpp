#include "utils.h"

#include <vector>

#include "value.h"

void removeTrailingNil(std::vector<ValuePtr>& values) {
    if (values.empty()) {
        return;
    }
    if (values.back()->getType() == ValueType::NIL) {
        values.pop_back();
    }
}