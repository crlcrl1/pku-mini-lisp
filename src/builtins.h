#ifndef BUILTINS_H
#define BUILTINS_H

#include <unordered_map>
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

ValuePtr displayln(const std::vector<ValuePtr>& params);

ValuePtr newline(const std::vector<ValuePtr>& params);

ValuePtr error(const std::vector<ValuePtr>& params);

ValuePtr print(const std::vector<ValuePtr>& params);

ValuePtr exit(const std::vector<ValuePtr>& params);

ValuePtr length(const std::vector<ValuePtr>& params);

ValuePtr car(const std::vector<ValuePtr>& params);

ValuePtr cdr(const std::vector<ValuePtr>& params);

ValuePtr atom(const std::vector<ValuePtr>& params);

ValuePtr boolean(const std::vector<ValuePtr>& params);

ValuePtr integer(const std::vector<ValuePtr>& params);

ValuePtr isList(const std::vector<ValuePtr>& params);

ValuePtr number(const std::vector<ValuePtr>& params);

ValuePtr null(const std::vector<ValuePtr>& params);

ValuePtr procedure(const std::vector<ValuePtr>& params);

ValuePtr string(const std::vector<ValuePtr>& params);

ValuePtr symbol(const std::vector<ValuePtr>& params);

ValuePtr pair(const std::vector<ValuePtr>& params);

ValuePtr append(const std::vector<ValuePtr>& params);

ValuePtr cons(const std::vector<ValuePtr>& params);

ValuePtr makeList(const std::vector<ValuePtr>& params);

ValuePtr map(const std::vector<ValuePtr>& params);

ValuePtr filter(const std::vector<ValuePtr>& params);

ValuePtr reduce(const std::vector<ValuePtr>& params);

ValuePtr abs(const std::vector<ValuePtr>& params);

ValuePtr expt(const std::vector<ValuePtr>& params);

ValuePtr quotient(const std::vector<ValuePtr>& params);

ValuePtr modulo(const std::vector<ValuePtr>& params);

ValuePtr remainder(const std::vector<ValuePtr>& params);

ValuePtr loceq(const std::vector<ValuePtr>&
                   params);  /// Whether two values are stored at the same memory location

ValuePtr dataeq(const std::vector<ValuePtr>& params);  /// Whether two values are equal

ValuePtr logicalNot(const std::vector<ValuePtr>& params);

ValuePtr even(const std::vector<ValuePtr>& params);

ValuePtr odd(const std::vector<ValuePtr>& params);

ValuePtr zero(const std::vector<ValuePtr>& params);

ValuePtr require(const std::vector<ValuePtr>& params);

}  // namespace builtins

#endif  // BUILTINS_H
