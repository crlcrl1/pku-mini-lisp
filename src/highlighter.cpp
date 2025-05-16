#include "highlighter.h"

#include <iostream>
#include <ranges>
#include <sstream>

#include "value.h"

std::string colored(const std::string& str, Color color, Style style) {
    std::stringstream ss;
    if (color != Color::NONE) {
        ss << "\033[" << static_cast<int>(color) << "m";
    }
    if (style != Style::NONE) {
        ss << "\033[" << static_cast<int>(style) << "m";
    }
    ss << str << "\033[0m";
    return ss.str();
}

void displayError(const LispError& err) {
    std::cerr << std::format("{}:\n {}", colored(err.name(), Color::RED, Style::BOLD), err.what())
              << std::endl;
    if (auto& location = err.location()) {
        std::cerr << std::format("  {}:{}:{}", location->file, location->row + 1, location->col)
                  << std::endl;
    }
}

std::string colorfulString(ValuePtr value) {
    switch (value->getType()) {
        case ValueType::NUMBER:
            return colored(value->toString(), Color::YELLOW, Style::NONE);
        case ValueType::STRING:
            return colored(value->toString(), Color::GREEN, Style::NONE);
        case ValueType::BOOLEAN:
            return colored(value->toString(), Color::MAGENTA, Style::NONE);
        case ValueType::SYMBOL:
            return colored(value->toString(), Color::CYAN, Style::NONE);
        case ValueType::PAIR: {
            std::stringstream ss;
            ss << '(';
            const auto valueList = dynamic_cast<PairValue*>(value)->toVector();
            const size_t valueNum = valueList.size();
            for (auto [i, v] : std::views::enumerate(valueList)) {
                if (i == valueNum - 2) {
                    const auto& last = valueList.back();
                    if (last->getType() == ValueType::NIL) {
                        ss << colorfulString(v);
                    } else {
                        ss << colorfulString(v) << " . " << colorfulString(last);
                    }
                    break;
                }
                ss << colorfulString(v) << ' ';
            }
            ss << ')';
            return ss.str();
        }
        case ValueType::NIL:
            return colored(value->toString(), Color::LIGHTGRAY, Style::NONE);
        default:
            return value->toString();
    }
}

void displayValue(ValuePtr value) {
    std::cout << colorfulString(value) << std::endl;
}