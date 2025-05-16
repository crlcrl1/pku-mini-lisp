#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <string>

#include "error.h"

enum class Color {
    NONE = 0,
    RED = 31,
    GREEN = 32,
    YELLOW = 93,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    LIGHTGRAY = 90,
};

enum class Style {
    NONE = 0,
    BOLD = 1,
    ITALIC = 3,
    UNDERLINE = 4,
};

std::string colored(const std::string& str, Color color, Style style);

void displayError(const LispError& err);

void displayValue(ValuePtr value);

#endif  // HIGHLIGHTER_H
