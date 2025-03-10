#include "eval_env.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <utility>

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
    parent = std::nullopt;
}

EvalEnv::EvalEnv(const std::shared_ptr<EvalEnv>& parent) : parent{parent} {}

std::shared_ptr<EvalEnv> EvalEnv::createEnv() {
    EvalEnv env;
    return std::make_shared<EvalEnv>(env);
}

ValuePtr EvalEnv::eval(ValuePtr expr) {
    using OptStr = std::optional<std::string>;

    const auto ty = expr->getType();
    if (SELF_EVAL_VALUES.contains(ty)) {  // self-evaluating types
        return expr;
    }
    if (ty == ValueType::NIL) {  // nil
        throw ValueError("Cannot evaluate an empty list (nil value)");
    }
    if (ty == ValueType::SYMBOL) {  // symbol, lookup in symbol table
        const auto symbolName =
            expr->asSymbolName()
                .or_else([] -> OptStr { throw ValueError("Expected symbol, found keyword"); })
                .value_or("");
        try {
            return this->lookupBinding(symbolName);
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
            return form(params, *this);
        }

        // It's a function call
        const auto proc = this->eval(first);
        const std::vector<ValuePtr> args = this->evalList(pair->getCdr());
        return apply(proc, args);
    }
    if (ty == ValueType::BUILTIN || ty == ValueType::LAMBDA) {
        return expr;
    }
    throw InternalError("This is a bug, not your fault, please report it");
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
    if (proc->getType() == ValueType::LAMBDA) {
        const auto lambda = dynamic_cast<LambdaValue*>(proc.get());
        return lambda->apply(args);
    }
    throw ValueError("Only functions can be applied");
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

std::optional<ValuePtr> EvalEnv::addVariable(const std::string& name, const ValuePtr& value) {
    if (symbolTable.contains(name)) {
        auto old = symbolTable[name];
        symbolTable[name] = value;
        return std::move(old);
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

std::shared_ptr<EvalEnv> EvalEnv::createChild(const std::vector<std::string>& params,
                                              const std::vector<ValuePtr>& args) {
    EvalEnv child = EvalEnv(this->shared_from_this());
    if (params.size() != args.size()) {
        throw ValueError(
            std::format("Expected {} arguments, but got {}", params.size(), args.size()));
    }
    for (size_t i = 0; i < params.size(); i++) {
        child.addVariable(params[i], args[i]);
    }
    return std::make_shared<EvalEnv>(child);
}

ValuePtr EvalEnv::lookupBinding(const std::string& name) {
    try {
        return symbolTable.at(name);
    } catch (const std::out_of_range&) {
        if (parent.has_value()) {
            return parent.value().lock()->lookupBinding(name);
        }
        throw ValueError(std::format("Undefined variable: {}", name));
    }
}
