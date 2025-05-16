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

    static void hookColor(const std::string& context, Replxx::colors_t& colors);

    static syntax_highlight_t syntaxHighlight;
    static keyword_highlight_t keywordHighlight;

public:
    Repl();

    std::stringstream& getInput() {
        return inputStream;
    }

    void updateCompletion();
    void readLine(const char* prompt);
};

#endif  // REPL_H
