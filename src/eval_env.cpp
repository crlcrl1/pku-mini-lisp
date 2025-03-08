#include "eval_env.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>

#include "builtins.h"
#include "error.h"
#include "forms.h"
#include "utils.h"
#include "value.h"

void EvalEnv::addBuiltins() {
    for (const auto& i : BUILTINS) {
        this->symbolTable.insert(i);
    }
}

EvalEnv::EvalEnv() {
    addBuiltins();
}

ValuePtr EvalEnv::eval(ValuePtr expr) {
    using OptStr = std::optional<std::string>;

    const auto ty = expr->getType();
    if (SELF_EVAL_VALUES.contains(ty)) {  // self-evaluating types
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

        // Check if it's a special form
        if (SPECIAL_FORMS.contains(first->asSymbolName().value())) {
            const auto form = SPECIAL_FORMS.at(first->asSymbolName().value());
            auto params = std::vector(values.begin() + 1, values.end());
            removeTrailingNil(params);
            return form(params, *this);
        }

        // It's a function call
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
    removeTrailingNil(vector);
    std::ranges::transform(vector, std::back_inserter(result),
                           [this](const ValuePtr& v) { return this->eval(v); });
    return result;
}

void EvalEnv::addVariable(const std::string& name, const ValuePtr& value) {
    this->symbolTable[name] = value;
}
