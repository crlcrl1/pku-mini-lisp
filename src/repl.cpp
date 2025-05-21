#include "repl.h"

#include <iostream>
#include <ranges>
#include <regex>

#include "forms.h"
#include "pool.h"

int Repl::utf8strLength(const std::string& s) {
    const size_t len = s.length();
    int codepointLen = 0;
    constexpr unsigned char m4 = 128 + 64 + 32 + 16;
    constexpr unsigned char m3 = 128 + 64 + 32;
    constexpr unsigned char m2 = 128 + 64;
    for (int i = 0; i < len; ++i, ++codepointLen) {
        if (const char c = s[i]; (c & m4) == m4) {
            i += 3;
        } else if ((c & m3) == m3) {
            i += 2;
        } else if ((c & m2) == m2) {
            i += 1;
        }
    }
    return codepointLen;
}

bool Repl::isSymbolSplit(const char ch) {
    return isspace(ch) || ch == '(' || ch == ')' || ch == '\'' || ch == '`' || ch == ',';
}

void Repl::hookColor(const std::string& context, Replxx::colors_t& colors) {
    const size_t totalLen = context.length();

    // highlight matching regex sequences
    for (const auto& [regex, color, checkSplit] : syntaxHighlight) {
        size_t pos = 0;
        std::string str = context;
        std::smatch match;

        while (std::regex_search(str, match, std::regex(regex))) {
            std::string c = match[0];
            std::string prefix = match.prefix();
            pos += utf8strLength(prefix);
            const int len = utf8strLength(c);

            bool shouldHighlight = true;
            if (checkSplit) {
                if (pos > 0 && !isSymbolSplit(context[pos - 1])) {
                    shouldHighlight = false;
                }
                if (pos + len < totalLen && !isSymbolSplit(context[pos + len])) {
                    shouldHighlight = false;
                }
            }

            if (shouldHighlight) {
                for (int i = 0; i < len; ++i) {
                    colors.at(pos + i) = color;
                }
            }

            pos += len;
            str = match.suffix();
        }
    }

    // match keywords
    size_t wordStart = 0;
    for (size_t i = 0; i <= totalLen; ++i) {
        if (const char ch = context[i]; i == totalLen || isSymbolSplit(ch)) {
            if (wordStart != i) {
                std::string word = context.substr(wordStart, i - wordStart);
                if (const auto it = keywordHighlight.find(word); it != keywordHighlight.end()) {
                    const auto color = it->second;
                    for (size_t j = wordStart; j < i; ++j) {
                        colors.at(j) = color;
                    }
                }
            }
            wordStart = i + 1;
        }
    }
}

Repl::Replxx::completions_t Repl::hookCompletion(std::string const& context, int& contextLen) {
    Replxx::completions_t completions;
    const size_t totalLen = context.length();
    size_t suffix = totalLen - 1;
    while (suffix > 0 && !isSymbolSplit(context[suffix])) {
        --suffix;
    }
    if (isSymbolSplit(context[suffix])) {
        ++suffix;
    }
    std::string suffixStr = context.substr(suffix, totalLen - suffix);
    if (suffixStr.empty()) {
        contextLen = 0;
        return completions;
    }
    contextLen = utf8strLength(suffixStr);
    const auto filterPrefix = [&](const std::string& str) { return str.starts_with(suffixStr); };
    const auto keywordCompletions = std::views::keys(keywordHighlight) |
                                    std::views::filter(filterPrefix) |
                                    std::ranges::to<std::vector>();
    std::ranges::copy(keywordCompletions, std::back_inserter(completions));
    const auto syntaxCompletions =
        this->syntaxCompletions | std::views::filter(filterPrefix) | std::ranges::to<std::vector>();
    std::ranges::copy(syntaxCompletions, std::back_inserter(completions));
    return completions;
}

Repl::Repl() {
    syntaxHighlight = {
        // symbols
        {R"([a-zA-Z_+\-!$%&*./:<>=?@~][a-zA-Z0-9_+\-!$%&*./:<>=?@~]*)", Replxx::Color::BLUE, true},
        // numbers
        {R"([+-]?(\d+(\.\d*)?|\.\d+))", Replxx::Color::YELLOW, true},
        // strings
        {R"("([^"\\]|\\.)*")", Replxx::Color::GREEN, true},
        // comments
        {R"(\s*;.*)", Replxx::Color::GRAY, false},
    };
    keywordHighlight = {
        {"#t", Replxx::Color::MAGENTA},
        {"#f", Replxx::Color::MAGENTA},
    };
    for (const auto& formName : std::views::keys(SPECIAL_FORMS)) {
        keywordHighlight[formName] = Replxx::Color::CYAN;
    }

    replxx.bind_key_internal(Replxx::KEY::control('D'), "abort_line");

    // here, we capture this pointer, so we should not copy or move the class
    replxx.set_highlighter_callback([this](const std::string& context, Replxx::colors_t& colors) {
        hookColor(context, colors);
    });
    replxx.set_completion_callback([this](std::string const& context, int& contextLen) {
        return hookCompletion(context, contextLen);
    });
}

void Repl::updateCompletion() {
    syntaxCompletions = ValuePool::instance()->root()->variables();
}

void Repl::readLine(const char* prompt) {
    auto input = replxx.input(prompt);
    if (input == nullptr) {
        ValuePool::dispose();
        std::exit(0);
    }
    inputStream << input << '\n';
}
