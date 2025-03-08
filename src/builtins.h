#ifndef BUILTINS_H
#define BUILTINS_H

#include <unordered_map>
#include <vector>

#include "value.h"

extern const std::unordered_map<std::string, ValuePtr> BUILTINS;

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

ValuePtr length(const std::vector<ValuePtr>& params);

ValuePtr car(const std::vector<ValuePtr>& params);

ValuePtr cdr(const std::vector<ValuePtr>& params);

// TODO: Add more built-in functions here

}  // namespace builtins

#endif  // BUILTINS_H
