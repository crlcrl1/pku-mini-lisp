#include "value.h"

#include <iomanip>
#include <memory>
#include <ranges>
#include <sstream>

#include "error.h"
#include "eval_env.h"
#include "pool.h"
#include "utils.h"

const std::set<ValueType> SELF_EVAL_VALUES = {
    ValueType::BOOLEAN,
    ValueType::NUMBER,
    ValueType::STRING,
};

const std::set<ValueType> ATOMIC_VALUES = {
    ValueType::BOOLEAN, ValueType::NUMBER, ValueType::STRING, ValueType::NIL, ValueType::SYMBOL,
};

ValueType Value::getType() const {
    return ty;
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

std::vector<ValuePtr> Value::children(ValuePtr value) {
    if (value->getType() != ValueType::PAIR) {
        return {};
    }
    std::vector<ValuePtr> children;
    auto pair = dynamic_cast<PairValue*>(value);
    children.push_back(pair->getCar());
    children.push_back(pair->getCdr());

    auto carChildren = Value::children(pair->getCar());
    auto cdrChildren = Value::children(pair->getCdr());
    children.insert(children.end(), carChildren.begin(), carChildren.end());
    children.insert(children.end(), cdrChildren.begin(), cdrChildren.end());
    return children;
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
    return value == dynamic_cast<BooleanValue*>(other)->getValue();
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
    return value == dynamic_cast<NumericValue*>(other)->getValue();
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
    return value == dynamic_cast<StringValue*>(other)->getValue();
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
    return value == dynamic_cast<SymbolValue*>(other)->getValue();
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
            current = dynamic_cast<PairValue*>(current->cdr);
        } else {
            result.push_back(current->cdr);
            break;
        }
    }
    return result;
}

PairValue PairValue::fromVector(const std::vector<ValuePtr>& vec) {
    if (vec.empty()) {
        return {LISP_NIL, LISP_NIL};
    }
    auto current = PairValue(vec.back(), LISP_NIL);
    for (auto it = vec.rbegin() + 1; it != vec.rend(); ++it) {
        current = PairValue(*it, LISP_PAIR(current));
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
    const auto otherPair = dynamic_cast<PairValue*>(other);
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
    return func == dynamic_cast<BuiltinProcValue*>(other)->func;
}

LambdaValue::LambdaValue(std::vector<std::string> params, std::vector<ValuePtr> body, EvalEnv* env)
    : Value(ValueType::LAMBDA), params(std::move(params)), body(std::move(body)), env(env) {}

std::string LambdaValue::toString() const {
    return "#<proc>";
}

ValuePtr LambdaValue::apply(const std::vector<ValuePtr>& args) const {
    if (args.size() != params.size()) {
        throw ValueError(
            std::format("Expected {} arguments, but got {}", params.size(), args.size()));
    }
    // Create a new environment
    const auto newEnv = pool.makeEnv(env);
    for (size_t i = 0; i < params.size(); i++) {
        newEnv->addVariable(params[i], args[i]);
    }
    ValuePtr result = LISP_NIL;
    for (const auto& expr : body) {
        result = newEnv->eval(expr);
    }
    return result;
}

bool LambdaValue::equals(const ValuePtr& other) const {
    if (other->getType() != ValueType::LAMBDA) {
        return false;
    }
    const auto otherLambda = dynamic_cast<LambdaValue*>(other);
    return params == otherLambda->params && body == otherLambda->body;
}

const std::vector<ValuePtr>& LambdaValue::getBody() const {
    return body;
}

std::string NilValue::toString() const {
    return "()";
}

bool NilValue::equals(const ValuePtr& other) const {
    return other->getType() == ValueType::NIL;
}
