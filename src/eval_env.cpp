#include "eval_env.h"

#include <algorithm>
#include <format>
#include <iterator>
#include <set>
#include <utility>

#include "builtins.h"
#include "error.h"
#include "forms.h"
#include "pool.h"
#include "utils.h"
#include "value.h"

#define BUILTIN_PAIR(procName, builtinName) \
    {#builtinName, ValuePool::instance()->makeValue<BuiltinProcValue>(&builtins::procName)}

void EvalEnv::addBuiltins() {
    const std::unordered_map<std::string, ValuePtr> builtins = {
        BUILTIN_PAIR(add, +),
        BUILTIN_PAIR(sub, -),
        BUILTIN_PAIR(mul, *),
        BUILTIN_PAIR(div, /),
        BUILTIN_PAIR(eq, =),
        BUILTIN_PAIR(lt, <),
        BUILTIN_PAIR(gt, >),
        BUILTIN_PAIR(le, <=),
        BUILTIN_PAIR(ge, >=),
        BUILTIN_PAIR(apply, apply),
        BUILTIN_PAIR(display, display),
        BUILTIN_PAIR(print, print),
        BUILTIN_PAIR(exit, exit),
        BUILTIN_PAIR(length, length),
        BUILTIN_PAIR(car, car),
        BUILTIN_PAIR(cdr, cdr),
        BUILTIN_PAIR(displayln, displayln),
        BUILTIN_PAIR(newline, newline),
        BUILTIN_PAIR(error, error),
        BUILTIN_PAIR(atom, atom?),
        BUILTIN_PAIR(boolean, boolean?),
        BUILTIN_PAIR(integer, integer?),
        BUILTIN_PAIR(isList, list?),
        BUILTIN_PAIR(number, number?),
        BUILTIN_PAIR(null, null?),
        BUILTIN_PAIR(pair, pair?),
        BUILTIN_PAIR(string, string?),
        BUILTIN_PAIR(symbol, symbol?),
        BUILTIN_PAIR(procedure, procedure?),
        BUILTIN_PAIR(append, append),
        BUILTIN_PAIR(cons, cons),
        BUILTIN_PAIR(makeList, list),
        BUILTIN_PAIR(map, map),
        BUILTIN_PAIR(filter, filter),
        BUILTIN_PAIR(reduce, reduce),
        BUILTIN_PAIR(abs, abs),
        BUILTIN_PAIR(expt, expt),
        BUILTIN_PAIR(quotient, quotient),
        BUILTIN_PAIR(modulo, modulo),
        BUILTIN_PAIR(remainder, remainder),
        BUILTIN_PAIR(loceq, eq?),
        BUILTIN_PAIR(dataeq, equal?),
        BUILTIN_PAIR(logicalNot, not),
        BUILTIN_PAIR(even, even?),
        BUILTIN_PAIR(odd, odd?),
        BUILTIN_PAIR(zero, zero?),
        BUILTIN_PAIR(require, require),
    };

    for (const auto& i : builtins) {
        this->symbolTable.insert(i);
    }
}

EvalEnv::EvalEnv() {
    parent = nullptr;
}

EvalEnv::EvalEnv(const EvalEnv* parent) : parent{parent} {}

ValuePtr EvalEnv::eval(ValuePtr expr) {
    using OptStr = std::optional<std::string>;

    const auto ty = expr->getType();
    if (SELF_EVAL_VALUES.contains(ty)) {  // self-evaluating types
        return expr;
    }
    if (ty == ValueType::NIL) {  // nil
        throw ValueError("Cannot evaluate an empty list (nil value)", expr->getLocation());
    }
    if (ty == ValueType::SYMBOL) {  // symbol, lookup in symbol table
        const auto symbolName =
            expr->asSymbolName()
                .or_else([&] -> OptStr {
                    throw ValueError("Expected symbol, found keyword", expr->getLocation());
                })
                .value_or("");
        try {
            return this->lookupBinding(symbolName);
        } catch (const ValueError& e) {
            if (e.location()) {
                throw;
            }
            throw ValueError(std::format("Undefined variable: {}", symbolName),
                             expr->getLocation());
        }
    }
    if (ty == ValueType::PAIR) {  // pair
        const auto pair = dynamic_cast<PairValue*>(expr);
        const std::vector<ValuePtr> values = pair->toVector();

        if (values.empty()) {
            throw ValueError("Cannot evaluate an empty list", expr->getLocation());
        }

        auto first = values[0];
        if (first->getType() != ValueType::SYMBOL) {
            first = this->eval(first);
        }

        const auto firstName = first->asSymbolName();
        // Check if it's a special form
        if (firstName && SPECIAL_FORMS.contains(firstName.value())) {
            const auto form = SPECIAL_FORMS.at(first->asSymbolName().value());
            auto params = std::vector(values.begin() + 1, values.end());
            removeTrailingNil(params);
            return form(params, this);
        }

        // It's a function call
        const auto proc = this->eval(first);
        const std::vector<ValuePtr> args = this->evalList(pair->getCdr());
        return apply(proc, args);
    }
    if (ty == ValueType::BUILTIN || ty == ValueType::LAMBDA) {
        return expr;
    }
    throw InternalError("This is a bug, not your fault, please report it", std::nullopt);
}

void EvalEnv::reset() {
    symbolTable.clear();
    addBuiltins();
}

ValuePtr EvalEnv::apply(const ValuePtr& proc, const std::vector<ValuePtr>& args) {
    if (proc->getType() == ValueType::BUILTIN) {
        const auto builtin = dynamic_cast<BuiltinProcValue*>(proc);
        return builtin->apply(args);
    }
    if (proc->getType() == ValueType::LAMBDA) {
        const auto lambda = dynamic_cast<LambdaValue*>(proc);
        return lambda->apply(args);
    }
    throw ValueError("Only functions can be applied", proc->getLocation());
}

std::vector<ValuePtr> EvalEnv::evalList(const ValuePtr& expr) {
    std::vector<ValuePtr> result;
    const auto* pair = dynamic_cast<PairValue*>(expr);
    if (pair == nullptr) {
        if (const auto* nil = dynamic_cast<NilValue*>(expr); nil != nullptr) {
            return result;
        }
        throw ValueError("Expected a list", expr->getLocation());
    }
    auto vector = pair->toVector();
    removeTrailingNil(vector);
    std::ranges::transform(vector, std::back_inserter(result),
                           [this](const ValuePtr& v) { return this->eval(v); });
    return result;
}

std::optional<ValuePtr> EvalEnv::addVariable(const std::string& name, const ValuePtr& value) {
    if (symbolTable.contains(name)) {
        auto old = symbolTable[name];
        symbolTable[name] = value;
        return old;
    }
    symbolTable[name] = value;
    return std::nullopt;
}

bool EvalEnv::removeVariable(const std::string& name) {
    if (symbolTable.contains(name)) {
        symbolTable.erase(name);
        return true;
    }
    return false;
}

ValuePtr EvalEnv::lookupBinding(const std::string& name) const {
    try {
        return symbolTable.at(name);
    } catch (const std::out_of_range&) {
        if (parent) {
            return parent->lookupBinding(name);
        }
        throw ValueError(std::format("Undefined variable: {}", name), std::nullopt);
    }
}

std::vector<std::string> EvalEnv::loadStack = {};