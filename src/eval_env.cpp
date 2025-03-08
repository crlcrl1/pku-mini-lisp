#include "eval_env.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>

#include "builtins.h"
#include "error.h"
#include "value.h"

#define BUILTIN_PAIR(procName, builtinName) \
    std::make_pair(#builtinName, std::make_shared<BuiltinProcValue>(&procName))

const std::set selfEvaluatingTypes = {
    ValueType::BOOLEAN,
    ValueType::NUMBER,
    ValueType::STRING,
};

// clang-format off
const std::unordered_map<std::string, ValuePtr> builtinFunctions = {
    BUILTIN_PAIR(builtins::add, +),
    BUILTIN_PAIR(builtins::sub, -),
    BUILTIN_PAIR(builtins::mul, *),
    BUILTIN_PAIR(builtins::div, /),
    BUILTIN_PAIR(builtins::eq, =),
    BUILTIN_PAIR(builtins::lt, <),
    BUILTIN_PAIR(builtins::gt, >),
    BUILTIN_PAIR(builtins::le, <=),
    BUILTIN_PAIR(builtins::ge, >=),
    BUILTIN_PAIR(builtins::apply, apply),
    BUILTIN_PAIR(builtins::display, display),
    BUILTIN_PAIR(builtins::print, print),
    BUILTIN_PAIR(builtins::exit, exit),
};
// clang-format on

void EvalEnv::addBuiltins() {
    for (const auto& i : builtinFunctions) {
        this->symbolTable.insert(i);
    }
}

EvalEnv::EvalEnv() {
    addBuiltins();
}

ValuePtr EvalEnv::eval(ValuePtr expr) {
    using OptStr = std::optional<std::string>;

    const auto ty = expr->getType();
    if (selfEvaluatingTypes.contains(ty)) {  // self-evaluating types
        return expr;
    }
    if (ty == ValueType::NIL) {  // nil
        throw ValueError("Cannot evaluate an empty list");
    }
    if (ty == ValueType::SYMBOL) {  // symbol, lookup in symbol table
        const auto symbolName =
            expr->asSymbolName()
                .or_else([] -> OptStr { throw ValueError("Expected symbol, found keyword"); })
                .value_or("");
        try {
            return symbolTable.at(symbolName);
        } catch (const std::out_of_range&) {
            throw ValueError(std::format("Undefined variable: {}", symbolName));
        }
    }
    if (ty == ValueType::PAIR) {  // pair
        const auto pair = dynamic_cast<PairValue*>(expr.get());
        const std::vector<ValuePtr> values = pair->toVector();

        if (values.empty()) {
            throw ValueError("Cannot evaluate an empty list");
        }

        const auto& first = values[0];
        if (first->getType() != ValueType::SYMBOL) {
            throw UnimplementedError("Only symbols can be evaluated");
        }
        const auto keyword = first->asKeyword().value_or(Keyword::INVALID);
        if (keyword == Keyword::DEFINE) {
            if (values.size() == 4 && values[3]->getType() != ValueType::NIL) {
                throw ValueError(
                    std::format("define: expected 2 arguments, but got {}", values.size()));
            }
            const auto& symbol = values[1];
            if (symbol->getType() != ValueType::SYMBOL) {
                throw ValueError("define: first argument must be a symbol");
            }
            const auto symbolName =
                symbol->asSymbolName()
                    .or_else([] -> OptStr { throw ValueError("Expected symbol, found keyword"); })
                    .value_or("");
            symbolTable[symbolName] = this->eval(values[2]);
            return std::make_shared<NilValue>();
        }
        const auto proc = this->eval(first);
        const std::vector<ValuePtr> args = this->evalList(pair->getCdr());
        return apply(proc, args);
    }
    throw UnimplementedError("EvalEnv::eval");
}

void EvalEnv::reset() {
    symbolTable.clear();
    addBuiltins();
}

ValuePtr EvalEnv::apply(const ValuePtr& proc, const std::vector<ValuePtr>& args) {
    if (proc->getType() == ValueType::BUILTIN) {
        const auto builtin = dynamic_cast<BuiltinProcValue*>(proc.get());
        return builtin->apply(args);
    }
    throw UnimplementedError("EvalEnv::apply");
}

std::vector<ValuePtr> EvalEnv::evalList(const ValuePtr& expr) {
    std::vector<ValuePtr> result;
    const auto* pair = dynamic_cast<PairValue*>(expr.get());
    if (pair == nullptr) {
        if (const auto* nil = dynamic_cast<NilValue*>(expr.get()); nil != nullptr) {
            return result;
        }
        throw ValueError("Expected a list");
    }
    auto vector = pair->toVector();
    // check if the last element is a nil
    if (vector.back()->getType() != ValueType::NIL) {
        throw ValueError("Expected a proper list");
    }
    vector.pop_back();
    std::ranges::transform(vector, std::back_inserter(result),
                           [this](const ValuePtr& v) { return this->eval(v); });
    return result;
}