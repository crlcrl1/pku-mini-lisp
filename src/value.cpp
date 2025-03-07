#include "value.h"

#include <iomanip>
#include <ranges>
#include <sstream>
#include <unordered_map>

const std::unordered_map<std::string, Keyword> keywordMap = {
    {"define", Keyword::DEFINE},
    {"lambda", Keyword::LAMBDA},
};

ValueType Value::getType() const {
    return ty;
}

std::optional<Keyword> Value::asKeyword() const {
    if (ty == ValueType::SYMBOL) {
        const auto symbolName = dynamic_cast<const SymbolValue*>(this)->getValue();
        if (const auto it = keywordMap.find(symbolName); it != keywordMap.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}

std::optional<std::string> Value::asSymbolName() const {
    if (ty == ValueType::SYMBOL) {
        auto symbolName = dynamic_cast<const SymbolValue*>(this)->getValue();
        if (const auto it = keywordMap.find(symbolName); it != keywordMap.end()) {
            return std::nullopt;
        }
        return symbolName;
    }
    return std::nullopt;
}

std::string BooleanValue::toString() const {
    return value ? "#t" : "#f";
}

std::string NumericValue::toString() const {
    return std::to_string(value);
}

std::string StringValue::toString() const {
    std::stringstream ss;  // Reuse the same stringstream to avoid runtime overhead
    ss << std::quoted(value);
    return ss.str();
}

std::string SymbolValue::toString() const {
    return value;
}

std::string SymbolValue::getValue() const {
    return value;
}

std::string PairValue::toString() const {
    std::stringstream ss;  // Reuse the same stringstream to avoid runtime overhead
    ss << '(';
    const auto valueList = this->toVector();
    const auto valueNum = valueList.size();
    for (auto [i, v] : std::views::enumerate(valueList)) {
        if (i == valueNum - 2) {
            const auto& last = valueList.back();
            if (last->getType() == ValueType::NIL) {
                ss << v->toString();
            } else {
                ss << v->toString() << " . " << last->toString();
            }
            break;
        }
        ss << v->toString() << ' ';
    }
    ss << ')';
    return ss.str();
}

std::vector<ValuePtr> PairValue::toVector() const {
    std::vector<ValuePtr> result;
    auto current = this;
    while (true) {
        result.push_back(current->car);
        if (current->cdr->getType() == ValueType::PAIR) {
            current = dynamic_cast<PairValue*>(current->cdr.get());
        } else {
            result.push_back(current->cdr);
            break;
        }
    }
    return result;
}

std::string NilValue::toString() const {
    return "()";
}
