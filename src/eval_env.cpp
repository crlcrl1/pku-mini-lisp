#include "eval_env.h"

#include <memory>
#include <set>

#include "error.h"
#include "value.h"

const std::set selfEvaluatingTypes = {
    ValueType::BOOLEAN,
    ValueType::NUMBER,
    ValueType::STRING,
};

ValuePtr EvalEnv::eval(ValuePtr expr) {
    using OptStr = std::optional<std::string>;

    auto ty = expr->getType();
    if (selfEvaluatingTypes.contains(ty)) {
        return expr;
    }
    if (ty == ValueType::NIL) {
        throw ValueError("Cannot evaluate an empty list");
    }
    if (ty == ValueType::SYMBOL) {
        const auto symbolName =
            expr->asSymbolName()
                .or_else([] -> OptStr { throw ValueError("Expected symbol, found keyword"); })
                .value_or("");
        try {
            return symbolTable.at(symbolName);
        } catch (const std::out_of_range&) {
            throw ValueError("Undefined variable: " + symbolName);
        }
    }
    if (ty == ValueType::PAIR) {
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
            symbolTable[symbolName] = eval(values[2]);
            return std::make_shared<NilValue>();
        }
    }
    throw UnimplementedError("EvalEnv::eval");
}

void EvalEnv::reset() {
    symbolTable.clear();
}