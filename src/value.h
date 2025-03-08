#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <string>
#include <vector>

class SymbolValue;

enum class Keyword {
    DEFINE,
    LAMBDA,
    INVALID,
};

enum class ValueType {
    BOOLEAN,
    NUMBER,
    STRING,
    NIL,
    SYMBOL,
    PAIR,
    BUILTIN,
};

class Value {
    ValueType ty;

public:
    explicit Value(const ValueType ty) : ty{ty} {}

    virtual ~Value() = default;
    virtual std::string toString() const = 0;
    ValueType getType() const;
    std::optional<Keyword> asKeyword() const;
    std::optional<std::string> asSymbolName() const;
    bool isNumber() const;
    std::optional<double> asNumber() const;
};

using ValuePtr = std::shared_ptr<Value>;
using BuiltinFuncType = ValuePtr(const std::vector<ValuePtr>&);

class BooleanValue : public Value {
    bool value;

public:
    explicit BooleanValue(const bool value) : Value(ValueType::BOOLEAN), value{value} {}
    std::string toString() const override;
};

class NumericValue : public Value {
    double value;

public:
    explicit NumericValue(const double value) : Value(ValueType::NUMBER), value{value} {}
    std::string toString() const override;
    double getValue() const;
};

class StringValue : public Value {
    std::string value;

public:
    explicit StringValue(const std::string& value) : Value(ValueType::STRING), value{value} {}
    std::string toString() const override;
};

class NilValue : public Value {
public:
    NilValue() : Value(ValueType::NIL) {}
    std::string toString() const override;
};

class SymbolValue : public Value {
    std::string value;

public:
    explicit SymbolValue(const std::string& value) : Value(ValueType::SYMBOL), value{value} {}
    std::string toString() const override;
    std::string getValue() const;
};

class PairValue : public Value {
    ValuePtr car;
    ValuePtr cdr;

public:
    PairValue(ValuePtr car, ValuePtr cdr)
        : Value(ValueType::PAIR), car{std::move(car)}, cdr{std::move(cdr)} {}

    std::string toString() const override;

    /**
     * Convert the pair to a vector
     * @return
     */
    std::vector<ValuePtr> toVector() const;

    ValuePtr getCar() const;
    ValuePtr getCdr() const;
};

class BuiltinProcValue : public Value {
    BuiltinFuncType* func;

public:
    explicit BuiltinProcValue(BuiltinFuncType* func) : Value(ValueType::BUILTIN), func(func) {}
    std::string toString() const override;
    ValuePtr apply(const std::vector<ValuePtr>& args) const;
};

#endif  // VALUE_H
