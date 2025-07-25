#ifndef REPL_H
#define REPL_H

#include <replxx.hxx>
#include <sstream>

class Repl {
    using Replxx = replxx::Replxx;
    using syntax_highlight_t = std::vector<std::tuple<std::string, Replxx::Color, bool>>;
    using keyword_highlight_t = std::unordered_map<std::string, Replxx::Color>;

    std::stringstream inputStream;
    Replxx replxx;

    static int utf8strLength(const std::string& s);
    static bool isSymbolSplit(char ch);

    void hookColor(const std::string& context, Replxx::colors_t& colors);
    Replxx::completions_t hookCompletion(std::string const& context, int& contextLen);

    syntax_highlight_t syntaxHighlight;
    keyword_highlight_t keywordHighlight;
    std::vector<std::string> syntaxCompletions;

public:
    Repl();
    // Disable copy constructor and move constructor because we need to pin the class
    // in the same memory. Refer to the constructor for more details.
    Repl(const Repl&) = delete;
    Repl(Repl&&) = delete;

    std::stringstream& getInput() {
        return inputStream;
    }

    void updateCompletion();
    void readLine(const char* prompt);
};

#endif  // REPL_H
