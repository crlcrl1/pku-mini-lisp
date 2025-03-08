#ifndef BUILTINS_H
#define BUILTINS_H

#include <vector>

#include "value.h"

namespace builtins {

ValuePtr add(const std::vector<ValuePtr>& params);

ValuePtr sub(const std::vector<ValuePtr>& params);

ValuePtr mul(const std::vector<ValuePtr>& params);

ValuePtr div(const std::vector<ValuePtr>& params);

ValuePtr eq(const std::vector<ValuePtr>& params);

ValuePtr lt(const std::vector<ValuePtr>& params);

ValuePtr gt(const std::vector<ValuePtr>& params);

ValuePtr le(const std::vector<ValuePtr>& params);

ValuePtr ge(const std::vector<ValuePtr>& params);

ValuePtr apply(const std::vector<ValuePtr>& params);

ValuePtr display(const std::vector<ValuePtr>& params);

ValuePtr print(const std::vector<ValuePtr>& params);

ValuePtr exit(const std::vector<ValuePtr>& params);

// TODO: Add more built-in functions here

}  // namespace builtins

#endif  // BUILTINS_H
