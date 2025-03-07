#ifndef UTIL_H
#define UTIL_H

#include <string>

bool isClose(double a, double b);

bool testEqual(const std::string& a, const std::string& b);

#define RUN_TEST(testCase, procFunc)                                                             \
    for (const auto& [input, output] : testCase.cases) {                                         \
        if (!output) {                                                                           \
            procFunc(input);                                                                     \
            continue;                                                                            \
        }                                                                                        \
        auto o = procFunc(input);                                                                \
        ASSERT_TRUE(testEqual(o, *output))                                                       \
            << "Input: " << input << "\nExpected output: " << *output << "\nReal output: " << o; \
    }

#endif  // UTIL_H
