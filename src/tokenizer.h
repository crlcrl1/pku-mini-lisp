#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <deque>
#include <string>
#include <utility>

#include "./token.h"

class Tokenizer {
private:
    TokenPtr nextToken(int& pos) const;
    std::deque<TokenPtr> tokenize() const;

    std::string input;
    explicit Tokenizer(const std::string& input) : input{input} {}

public:
    static std::deque<TokenPtr> tokenize(const std::string& input);
};

#endif