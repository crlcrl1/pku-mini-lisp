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
        const auto c = input[pos];
        if (c == ';') {  // Comment
            while (pos < input.size() && input[pos] != '\n') {
                pos++;
            }
        } else if (std::isspace(c)) {  // Skip whitespace
            pos++;
        } else if (auto token = Token::fromChar(c)) {  // Special character
            pos++;
            return token;
        } else if (c == '#') {  // Boolean literal
            if (auto result = BooleanLiteralToken::fromChar(input[pos + 1])) {
                pos += 2;
                return result;
            }
            throw SyntaxError("Unexpected character after #");
        } else if (c == '"') {  // String literal
            std::string string;
            pos++;
            while (pos < input.size()) {
                if (input[pos] == '"') {  // End of string
                    pos++;
                    return std::make_unique<StringLiteralToken>(string);
                } else if (input[pos] == '\\') {  // Escape character
                    if (pos + 1 >= input.size()) {
                        throw SyntaxError("Unexpected end of string literal");
                    }
                    const auto next = input[pos + 1];
                    if (next == 'n') {
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
            throw SyntaxError("Unexpected end of string literal");
        } else {  // Numeric literal or identifier
            const int start = pos;
            do {
                pos++;
            } while (pos < input.size() && !std::isspace(input[pos]) &&
                     !TOKEN_END.contains(input[pos]));
            auto text = input.substr(start, pos - start);
            if (text == ".") {
                return Token::dot();
            }
            if (std::isdigit(text[0]) || text[0] == '+' || text[0] == '-' || text[0] == '.') {
                try {
                    return std::make_unique<NumericLiteralToken>(std::stod(text));
                } catch (std::invalid_argument&) {
                }
            }
            return std::make_unique<IdentifierToken>(text);
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

std::deque<TokenPtr> Tokenizer::tokenize(const std::string& input) {
    return Tokenizer(input).tokenize();
}

bool Tokenizer::checkEnd(const std::deque<TokenPtr>& tokens) {
    ssize_t parenCount = 0;
    for (const auto& token : tokens) {
        if (token->getType() == TokenType::LEFT_PAREN) {
            parenCount++;
        } else if (token->getType() == TokenType::RIGHT_PAREN) {
            parenCount--;
        }
        if (parenCount < 0) {
            throw SyntaxError("Unexpected ')'");
        }
    }
    return parenCount == 0;
}

std::deque<TokenPtr> Tokenizer::fromStream(std::istream* stream, const bool isRepl) {
    if (isRepl) {
        std::cout << ">>> ";
    }
    std::string line;
    std::getline(*stream, line);

    auto tokens = tokenize(line);
    while (!checkEnd(tokens)) {
        if (isRepl) {
            std::cout << "... ";
        }
        std::getline(*stream, line);
        auto newTokens = tokenize(line);
        tokens = merge(std::move(tokens), std::move(newTokens));
    }
    return tokens;
}
