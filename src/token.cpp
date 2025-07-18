#include "token.h"

#include <iomanip>
#include <sstream>

using namespace std::literals;

TokenPtr Token::fromChar(char c, const std::string& file, int row, int col) {
    TokenType type;
    switch (c) {
        case '(':
            type = TokenType::LEFT_PAREN;
            break;
        case ')':
            type = TokenType::RIGHT_PAREN;
            break;
        case '\'':
            type = TokenType::QUOTE;
            break;
        case '`':
            type = TokenType::QUASIQUOTE;
            break;
        case ',':
            type = TokenType::UNQUOTE;
            break;
        // DOT not listed here, because it can be part of identifier/literal.
        default:
            return nullptr;
    }
    return TokenPtr(new Token(type, file, row, col, 1));
}

TokenPtr Token::dot(const std::string& file, int row, int col) {
    return TokenPtr(new Token(TokenType::DOT, file, row, col, 1));
}

std::string Token::toString() const {
    switch (type) {
        case TokenType::LEFT_PAREN:
            return "(LEFT_PAREN)";
        case TokenType::RIGHT_PAREN:
            return "(RIGHT_PAREN)";
        case TokenType::QUOTE:
            return "(QUOTE)";
        case TokenType::QUASIQUOTE:
            return "(QUASIQUOTE)";
        case TokenType::UNQUOTE:
            return "(UNQUOTE)";
        case TokenType::DOT:
            return "(DOT)";
        default:
            return "(UNKNOWN)";
    }
}

std::unique_ptr<BooleanLiteralToken> BooleanLiteralToken::fromChar(char c, const std::string& file,
                                                                   int row, int col) {
    if (c == 't') {
        return std::make_unique<BooleanLiteralToken>(true, file, row, col);
    }
    if (c == 'f') {
        return std::make_unique<BooleanLiteralToken>(false, file, row, col);
    }
    return nullptr;
}

std::string BooleanLiteralToken::toString() const {
    return "(BOOLEAN_LITERAL "s + (value ? "true" : "false") + ")";
}

std::string NumericLiteralToken::toString() const {
    return "(NUMERIC_LITERAL " + std::to_string(value) + ")";
}

std::string StringLiteralToken::toString() const {
    std::ostringstream ss;
    ss << "(STRING_LITERAL " << std::quoted(value) << ")";
    return ss.str();
}

std::string IdentifierToken::toString() const {
    return "(IDENTIFIER " + name + ")";
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    return os << token.toString();
}
