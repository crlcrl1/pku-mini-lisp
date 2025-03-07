#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <string>
#include <vector>

enum class ValueType {
    BOOLEAN,
    NUMBER,
    STRING,
    NIL,
    SYMBOL,
    PAIR,
};

class Value {
    ValueType ty;

public:
    explicit Value(ValueType ty) : ty{ty} {}

    virtual ~Value() = default;
    virtual std::string toString() const = 0;
    ValueType getType() const;
};

using ValuePtr = std::shared_ptr<Value>;

class BooleanValue : public Value {
    bool value;

public:
    explicit BooleanValue(bool value) : Value(ValueType::BOOLEAN), value{value} {}
    std::string toString() const override;
};

class NumericValue : public Value {
    double value;

public:
    explicit NumericValue(double value) : Value(ValueType::NUMBER), value{value} {}
    std::string toString() const override;
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
};

class PairValue : public Value {
    ValuePtr car;
    ValuePtr cdr;

public:
    PairValue(ValuePtr car, ValuePtr cdr)
        : Value(ValueType::PAIR), car{std::move(car)}, cdr{std::move(cdr)} {}

    std::string toString() const override;

    std::vector<ValuePtr> toList() const;
};

#endif  // VALUE_H
