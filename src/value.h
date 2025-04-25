#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <set>
#include <string>
#include <vector>

class Value;

using ValuePtr = Value*;

class EvalEnv;
class SymbolValue;

enum class ValueType {
    BOOLEAN,
    NUMBER,
    STRING,
    NIL,
    SYMBOL,
    PAIR,
    BUILTIN,
    LAMBDA,
};

class Value {
    ValueType ty;

public:
    explicit Value(const ValueType ty) : ty{ty} {}
    virtual ~Value() = default;

    virtual std::string toString() const = 0;
    ValueType getType() const;
    std::optional<std::string> asSymbolName() const;
    bool isNumber() const;
    std::optional<double> asNumber() const;
    bool isAtom() const;
    virtual bool equals(const ValuePtr& other) const = 0;

    static std::vector<ValuePtr> children(ValuePtr value);
};

using BuiltinFuncType = ValuePtr(const std::vector<ValuePtr>&);

class BooleanValue : public Value {
    bool value;

    explicit BooleanValue(const bool value) : Value(ValueType::BOOLEAN), value{value} {}

public:
    std::string toString() const override;
    bool getValue() const;
    bool equals(const ValuePtr& other) const override;

    friend class ValuePool;
};

class NumericValue : public Value {
    double value;

    explicit NumericValue(const double value) : Value(ValueType::NUMBER), value{value} {}

public:
    std::string toString() const override;
    double getValue() const;
    bool equals(const ValuePtr& other) const override;

    friend class ValuePool;
};

class StringValue : public Value {
    std::string value;

    explicit StringValue(const std::string& value) : Value(ValueType::STRING), value{value} {}

public:
    std::string toString() const override;
    std::string getValue() const;
    bool equals(const ValuePtr& other) const override;

    friend class ValuePool;
};

class NilValue : public Value {
    NilValue() : Value(ValueType::NIL) {}

public:
    std::string toString() const override;
    bool equals(const ValuePtr& other) const override;

    friend class ValuePool;
};

class SymbolValue : public Value {
    std::string value;

    explicit SymbolValue(const std::string& value) : Value(ValueType::SYMBOL), value{value} {}

public:
    std::string toString() const override;
    std::string getValue() const;
    bool equals(const ValuePtr& other) const override;

    friend class ValuePool;
};

class PairValue : public Value {
    ValuePtr car;
    ValuePtr cdr;

    PairValue(ValuePtr car, ValuePtr cdr) : Value(ValueType::PAIR), car{car}, cdr{cdr} {}

public:
    std::string toString() const override;

    /**
     * Convert the pair to a vector
     *
     * @return a vector of values
     */
    std::vector<ValuePtr> toVector() const;

    static PairValue fromVector(const std::vector<ValuePtr>& vec);

    ValuePtr getCar() const;
    ValuePtr getCdr() const;
    bool equals(const ValuePtr& other) const override;

    friend class ValuePool;
};

class BuiltinProcValue : public Value {
    BuiltinFuncType* func;

    explicit BuiltinProcValue(BuiltinFuncType* func) : Value(ValueType::BUILTIN), func(func) {}

public:
    std::string toString() const override;
    ValuePtr apply(const std::vector<ValuePtr>& args) const;
    bool equals(const ValuePtr& other) const override;

    friend class ValuePool;
};

class LambdaValue : public Value {
protected:
    std::vector<std::string> params;
    std::vector<ValuePtr> body;
    EvalEnv* env;

public:
    LambdaValue(std::vector<std::string> params, std::vector<ValuePtr> body, EvalEnv* env);

    std::string toString() const override;
    virtual ValuePtr apply(const std::vector<ValuePtr>& args) const;
    bool equals(const ValuePtr& other) const override;
    const std::vector<ValuePtr>& getBody() const;

    friend class ValuePool;
};

extern const std::set<ValueType> SELF_EVAL_VALUES;

extern const std::set<ValueType> ATOMIC_VALUES;

#endif  // VALUE_H
