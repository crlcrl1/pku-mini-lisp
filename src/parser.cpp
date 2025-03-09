#include "parser.h"

#include "error.h"
#include "token.h"
#include "utils.h"
#include "value.h"

/**
 * Return a value if the token matches the given type.
 *
 * @param token token to parse
 * @param tokenTy the type of token to match
 * @param valueTy the type of value to return
 */
#define RETURN_VALUE_IF_MATCH(token, tokenTy, valueTy)                \
    if (auto t = dynamic_cast<tokenTy*>(token.get()); t != nullptr) { \
        return std::make_shared<valueTy>(t->getValue());              \
    }

/**
 * Return a quote value if the token matches the given type.
 *
 * @param token token to parse
 * @param tokenTy enum member of TokenType to match
 * @param symbolName the name of the quote token
 */
#define RETURN_QUOTE_IF_MATCH(token, tokenTy, symbolName)                              \
    if (token->getType() == TokenType::tokenTy) {                                      \
        return std::make_shared<PairValue>(                                            \
            std::make_shared<SymbolValue>(#symbolName),                                \
            std::make_shared<PairValue>(this->parse(), std::make_shared<NilValue>())); \
    }

Parser::Parser(std::deque<TokenPtr> tokens) : tokens(std::move(tokens)) {}

ValuePtr Parser::parse() {
    const TokenPtr token = std::move(tokens.front());
    tokens.pop_front();

    RETURN_VALUE_IF_MATCH(token, NumericLiteralToken, NumericValue);
    RETURN_VALUE_IF_MATCH(token, BooleanLiteralToken, BooleanValue);
    RETURN_VALUE_IF_MATCH(token, StringLiteralToken, StringValue);
    RETURN_VALUE_IF_MATCH(token, IdentifierToken, SymbolValue);

    RETURN_QUOTE_IF_MATCH(token, QUOTE, quote);
    RETURN_QUOTE_IF_MATCH(token, QUASIQUOTE, quasiquote);
    RETURN_QUOTE_IF_MATCH(token, UNQUOTE, unquote);

    if (token->getType() == TokenType::LEFT_PAREN) {
        return this->parseTails();
    }

    throw SyntaxError("Unexpected token: " + token->toString());
}

bool Parser::empty() const {
    return tokens.empty();
}

ValuePtr Parser::parseTails() {
    if (tokens.empty()) {
        throw SyntaxError("Expected ')'");
    }

    auto frontTy = tokens.front()->getType();

    // find right parenthesis, return nil
    if (frontTy == TokenType::RIGHT_PAREN) {
        tokens.pop_front();
        return LISP_NIL;
    }

    auto car = this->parse();

    if (tokens.empty()) {
        throw SyntaxError("Expected ')'");
    }

    frontTy = tokens.front()->getType();

    // check if the next token is a dot, if it is, parse the cdr
    if (frontTy == TokenType::DOT) {
        tokens.pop_front();
        auto cdr = this->parse();

        // check the end token, if it is not a right parenthesis, throw an error
        const TokenPtr endToken = std::move(tokens.front());
        tokens.pop_front();
        if (endToken->getType() != TokenType::RIGHT_PAREN) {
            throw SyntaxError("Expected ')' at the end of the list");
        }
        return LISP_PAIR(car, cdr);
    }

    auto cdr = this->parseTails();
    return LISP_PAIR(car, cdr);
}
