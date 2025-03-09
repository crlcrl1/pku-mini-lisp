#include "value.h"

#include <iomanip>
#include <memory>
#include <ranges>
#include <sstream>
#include <unordered_map>

#include "error.h"
#include "eval_env.h"

const std::set<ValueType> SELF_EVAL_VALUES = {
    ValueType::BOOLEAN,
    ValueType::NUMBER,
    ValueType::STRING,
};

const std::set<ValueType> ATOMIC_VALUES = {
    ValueType::BOOLEAN, ValueType::NUMBER, ValueType::STRING, ValueType::NIL, ValueType::SYMBOL,
};

const std::unordered_map<std::string, Keyword> KEYWORD_MAP = {
    {"define", Keyword::DEFINE},
    {"lambda", Keyword::LAMBDA},
};

ValueType Value::getType() const {
    return ty;
}

std::optional<Keyword> Value::asKeyword() const {
    if (ty == ValueType::SYMBOL) {
        const auto symbolName = dynamic_cast<const SymbolValue*>(this)->getValue();
        if (const auto it = KEYWORD_MAP.find(symbolName); it != KEYWORD_MAP.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}

std::optional<std::string> Value::asSymbolName() const {
    if (ty == ValueType::SYMBOL) {
        auto symbolName = dynamic_cast<const SymbolValue*>(this)->getValue();
        return symbolName;
    }
    return std::nullopt;
}

bool Value::isNumber() const {
    return ty == ValueType::NUMBER;
}

std::optional<double> Value::asNumber() const {
    if (this->isNumber()) {
        return dynamic_cast<const NumericValue*>(this)->getValue();
    }
    return std::nullopt;
}

bool Value::isAtom() const {
    return ATOMIC_VALUES.contains(ty);
}

std::string BooleanValue::toString() const {
    return value ? "#t" : "#f";
}

bool BooleanValue::getValue() const {
    return value;
}

bool BooleanValue::equals(const ValuePtr& other) const {
    if (other->getType() != ValueType::BOOLEAN) {
        return false;
    }
    return value == dynamic_cast<BooleanValue*>(other.get())->getValue();
}

std::string NumericValue::toString() const {
    auto result = std::to_string(value);
    if (result.find('.') != std::string::npos) {
        result.erase(result.find_last_not_of('0') + 1);
        if (result.back() == '.') {
            result.pop_back();
        }
    }
    return result;
}

double NumericValue::getValue() const {
    return value;
}

bool NumericValue::equals(const ValuePtr& other) const {
    if (other->getType() != ValueType::NUMBER) {
        return false;
    }
    return value == dynamic_cast<NumericValue*>(other.get())->getValue();
}

std::string StringValue::toString() const {
    std::stringstream ss;
    ss << std::quoted(value);
    return ss.str();
}

std::string StringValue::getValue() const {
    return value;
}

bool StringValue::equals(const ValuePtr& other) const {
    if (other->getType() != ValueType::STRING) {
        return false;
    }
    return value == dynamic_cast<StringValue*>(other.get())->getValue();
}

std::string SymbolValue::toString() const {
    return value;
}

std::string SymbolValue::getValue() const {
    return value;
}

bool SymbolValue::equals(const ValuePtr& other) const {
    if (other->getType() != ValueType::SYMBOL) {
        return false;
    }
    return value == dynamic_cast<SymbolValue*>(other.get())->getValue();
}

std::string PairValue::toString() const {
    std::stringstream ss;
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

PairValue PairValue::fromVector(const std::vector<ValuePtr>& vec) {
    if (vec.empty()) {
        return {std::make_shared<NilValue>(), std::make_shared<NilValue>()};
    }
    auto current = PairValue(vec.back(), std::make_shared<NilValue>());
    for (auto it = vec.rbegin() + 1; it != vec.rend(); ++it) {
        current = PairValue(*it, std::make_shared<PairValue>(current));
    }
    return current;
}

ValuePtr PairValue::getCar() const {
    return car;
}

ValuePtr PairValue::getCdr() const {
    return cdr;
}

bool PairValue::equals(const ValuePtr& other) const {
    if (other->getType() != ValueType::PAIR) {
        return false;
    }
    const auto otherPair = dynamic_cast<PairValue*>(other.get());
    return car->equals(otherPair->car) && cdr->equals(otherPair->cdr);
}

std::string BuiltinProcValue::toString() const {
    return "#<proc>";
}

ValuePtr BuiltinProcValue::apply(const std::vector<ValuePtr>& args) const {
    return func(args);
}

bool BuiltinProcValue::equals(const ValuePtr& other) const {
    if (other->getType() != ValueType::BUILTIN) {
        return false;
    }
    return func == dynamic_cast<BuiltinProcValue*>(other.get())->func;
}

std::string LambdaValue::toString() const {
    return "#<proc>";
}

ValuePtr LambdaValue::apply(const std::vector<ValuePtr>& args) const {
    if (args.size() != params.size()) {
        throw ValueError(
            std::format("Expected {} arguments, but got {}", params.size(), args.size()));
    }
    for (size_t i = 0; i < params.size(); i++) {
        env->addVariable(params[i], args[i]);
    }
    ValuePtr result = std::make_shared<NilValue>();
    for (const auto& expr : body) {
        result = env->eval(expr);
    }
    return result;
}

bool LambdaValue::equals(const ValuePtr& other) const {
    if (other->getType() != ValueType::LAMBDA) {
        return false;
    }
    const auto otherLambda = dynamic_cast<LambdaValue*>(other.get());
    return params == otherLambda->params && body == otherLambda->body;
}

std::string NilValue::toString() const {
    return "()";
}

bool NilValue::equals(const ValuePtr& other) const {
    return other->getType() == ValueType::NIL;
}
