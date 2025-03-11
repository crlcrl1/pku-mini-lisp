#ifndef PARSER_H
#define PARSER_H

#include <deque>
#include <memory>

class Value;
class Token;

using ValuePtr = Value*;
using TokenPtr = std::unique_ptr<Token>;

class Parser {
    std::deque<TokenPtr> tokens;

    ValuePtr parseTails();

public:
    explicit Parser(std::deque<TokenPtr> tokens);

    ValuePtr parse();
    bool empty() const;
};

#endif  // PARSER_H
