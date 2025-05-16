#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <deque>
#include <optional>
#include <string>

#include "repl.h"
#include "token.h"

class Tokenizer {
    TokenPtr nextToken(int& pos) const;
    std::deque<TokenPtr> tokenize() const;

    std::string input;
    std::string file;

    int row = 0;

    explicit Tokenizer(const std::string& input, const std::optional<std::string>& file, int row);

public:
    static std::deque<TokenPtr> tokenize(const std::string& input,
                                         const std::optional<std::string>& file, int row);
    static bool checkEnd(const std::deque<TokenPtr>& tokens);
    static std::deque<TokenPtr> fromStream(std::istream* stream, std::optional<Repl>& repl,
                                           const std::optional<std::string>& file, int& row);
};

#endif