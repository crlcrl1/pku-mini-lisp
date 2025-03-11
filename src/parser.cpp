#include "parser.h"

#include "error.h"
#include "pool.h"
#include "token.h"
#include "utils.h"
#include "value.h"

#define RETURN_VALUE_IF_MATCH(token, tokenTy, valueTy)                \
    if (auto t = dynamic_cast<tokenTy*>(token.get()); t != nullptr) { \
        return pool.makeValue<valueTy>(t->getValue());                \
    }

#define RETURN_QUOTE_IF_MATCH(token, tokenTy, symbolName)                          \
    if (token->getType() == TokenType::tokenTy) {                                  \
        return pool.makeValue<PairValue>(                                          \
            pool.makeValue<SymbolValue>(#symbolName),                              \
            pool.makeValue<PairValue>(this->parse(), pool.makeValue<NilValue>())); \
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
