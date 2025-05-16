#include "tokenizer.h"

#include <cctype>
#include <iostream>
#include <set>
#include <stdexcept>

#include "error.h"
#include "utils.h"

const std::set TOKEN_END{'(', ')', '\'', '`', ',', '"'};

TokenPtr Tokenizer::nextToken(int& pos) const {
    while (pos < input.size()) {
        if (const auto c = input[pos]; c == ';') {  // Comment
            while (pos < input.size() && input[pos] != '\n') {
                pos++;
            }
        } else if (std::isspace(c)) {  // Skip whitespace
            pos++;
        } else if (auto token = Token::fromChar(c, file, row, pos)) {  // Special character
            pos++;
            return token;
        } else if (c == '#') {  // Boolean literal
            if (auto result = BooleanLiteralToken::fromChar(input[pos + 1], file, row, pos)) {
                pos += 2;
                return result;
            }
            throw SyntaxError("Unexpected character after #", Location{file, row, pos, 1});
        } else if (c == '"') {  // String literal
            std::string string;
            int start = pos;
            pos++;
            while (pos < input.size()) {
                if (input[pos] == '"') {  // End of string
                    pos++;
                    return std::make_unique<StringLiteralToken>(string, file, row, start,
                                                                pos - start);
                }
                if (input[pos] == '\\') {  // Escape character
                    if (pos + 1 >= input.size()) {
                        throw SyntaxError("Unexpected end of string literal",
                                          Location{file, row, pos, 1});
                    }
                    if (const auto next = input[pos + 1]; next == 'n') {
                        string += '\n';
                    } else {
                        string += next;
                    }
                    pos += 2;
                } else {
                    string += input[pos];
                    pos++;
                }
            }
            throw SyntaxError("Unexpected end of string literal",
                              Location{file, row, start, pos - start});
        } else {  // Numeric literal or identifier
            const int start = pos;
            do {
                pos++;
            } while (pos < input.size() && !std::isspace(input[pos]) &&
                     !TOKEN_END.contains(input[pos]));
            auto text = input.substr(start, pos - start);
            if (text == ".") {
                return Token::dot(file, row, pos);
            }
            if (std::isdigit(text[0]) || text[0] == '+' || text[0] == '-' || text[0] == '.') {
                try {
                    return std::make_unique<NumericLiteralToken>(std::stod(text), file, row, start,
                                                                 pos - start);
                } catch (std::invalid_argument&) {
                }
            }
            return std::make_unique<IdentifierToken>(text, file, row, start, pos - start);
        }
    }
    return nullptr;
}

std::deque<TokenPtr> Tokenizer::tokenize() const {
    std::deque<TokenPtr> tokens;
    int pos = 0;
    while (true) {
        auto token = nextToken(pos);
        if (!token) {
            break;
        }
        tokens.push_back(std::move(token));
    }
    return tokens;
}

Tokenizer::Tokenizer(const std::string& input, const std::optional<std::string>& file, int row)
    : input{input}, file{file.value_or("<stdin>")}, row{row} {}

std::deque<TokenPtr> Tokenizer::tokenize(const std::string& input,
                                         const std::optional<std::string>& file, int row) {
    return Tokenizer(input, file, row).tokenize();
}

bool Tokenizer::checkEnd(const std::deque<TokenPtr>& tokens) {
    int parenCount = 0;
    for (const auto& token : tokens) {
        if (token->getType() == TokenType::LEFT_PAREN) {
            parenCount++;
        } else if (token->getType() == TokenType::RIGHT_PAREN) {
            parenCount--;
        }
        if (parenCount < 0) {
            throw SyntaxError("Unexpected ')'", token->getLocation());
        }
    }
    return parenCount == 0;
}

std::deque<TokenPtr> Tokenizer::fromStream(std::istream* stream, std::optional<Repl>& repl,
                                           const std::optional<std::string>& file, int& row) {
    if (repl) {
        repl->updateCompletion();
        repl->readLine(">>> ");
    }
    std::string line;
    std::getline(*stream, line);

    auto tokens = tokenize(line, file, row);
    row++;
    while (!checkEnd(tokens)) {
        if (repl) {
            repl->readLine("... ");
        }
        if (!repl && stream->eof()) {
            throw SyntaxError("Unexpected end of file",
                              Location{file.value_or("<stdin>"), row, 0, 0});
        }
        std::getline(*stream, line);
        auto newTokens = tokenize(line, file, row);
        row++;
        tokens = merge(std::move(tokens), std::move(newTokens));
    }
    return tokens;
}
