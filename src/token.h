#ifndef TOKEN_H
#define TOKEN_H

#include <memory>
#include <string>

#include "location.h"

enum class TokenType {
    LEFT_PAREN,
    RIGHT_PAREN,
    QUOTE,
    QUASIQUOTE,
    UNQUOTE,
    DOT,
    BOOLEAN_LITERAL,
    NUMERIC_LITERAL,
    STRING_LITERAL,
    IDENTIFIER,
};

class Token;
using TokenPtr = std::unique_ptr<Token>;

class Token {
    TokenType type;
    Location location;

protected:
    explicit Token(TokenType type, const std::string& file, int row, int col, int len)
        : type{type}, location{.file = file, .row = row, .col = col, .len = len} {}

public:
    virtual ~Token() = default;

    /**
     * @brief Try to generate a token from a special character, i.e. '(', ')', '\`',
     * '`', ','.
     * @param c The character to generate token from.
     * @param file The file name of the token.
     * @param row The row number of the token in the file.
     * @param col The column number of the token in the file.
     * @return A token if the character is a special character, nullptr otherwise.
     */
    static TokenPtr fromChar(char c, const std::string& file, int row, int col);
    static TokenPtr dot(const std::string& file, int row, int col);

    TokenType getType() const {
        return type;
    }
    virtual std::string toString() const;

    const Location& getLocation() const {
        return location;
    }
};

class BooleanLiteralToken : public Token {
    bool value;

public:
    explicit BooleanLiteralToken(bool value, const std::string& file, int row, int col)
        : Token(TokenType::BOOLEAN_LITERAL, file, row, col, 2), value{value} {}

    static std::unique_ptr<BooleanLiteralToken> fromChar(char c, const std::string& file, int row,
                                                         int col);

    bool getValue() const {
        return value;
    }
    std::string toString() const override;
};

class NumericLiteralToken : public Token {
    double value;

public:
    explicit NumericLiteralToken(double value, const std::string& file, int row, int col, int len)
        : Token(TokenType::NUMERIC_LITERAL, file, row, col, len), value{value} {}

    double getValue() const {
        return value;
    }
    std::string toString() const override;
};

class StringLiteralToken : public Token {
    std::string value;

public:
    explicit StringLiteralToken(const std::string& value, const std::string& file, int row, int col,
                                int len)
        : Token(TokenType::STRING_LITERAL, file, row, col, len), value{value} {}

    const std::string& getValue() const {
        return value;
    }
    std::string toString() const override;
};

class IdentifierToken : public Token {
    std::string name;

public:
    explicit IdentifierToken(const std::string& name, const std::string& file, int row, int col,
                             int len)
        : Token(TokenType::IDENTIFIER, file, row, col, len), name{name} {}

    const std::string& getName() const {
        return name;
    }
    const std::string& getValue() const {
        return name;
    }
    std::string toString() const override;
};

std::ostream& operator<<(std::ostream& os, const Token& token);

#endif
