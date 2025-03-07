#ifndef PARSER_H
#define PARSER_H

#include <deque>
#include <memory>

class Value;
class Token;

using ValuePtr = std::shared_ptr<Value>;
using TokenPtr = std::unique_ptr<Token>;

class Parser {
    std::deque<TokenPtr> tokens;

    ValuePtr parseTails();

public:
    explicit Parser(std::deque<TokenPtr> tokens);

    ValuePtr parse();
};

#endif  // PARSER_H
