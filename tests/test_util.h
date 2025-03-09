#ifndef UTIL_H
#define UTIL_H

#include <string>

bool isClose(double a, double b);

bool testEqual(const std::string& a, const std::string& b);

#define RUN_TEST(testCase, procFunc)                                                   \
    for (const auto& [input, output] : testCase.cases) {                               \
        try {                                                                          \
            if (!output) {                                                             \
                procFunc(input);                                                       \
                continue;                                                              \
            }                                                                          \
            auto o = procFunc(input);                                                  \
            ASSERT_TRUE(testEqual(o, *output))                                         \
                << "Input: " << input << "\nExpected output: " << *output              \
                << "\nReal output: " << o;                                             \
        } catch (std::runtime_error & e) {                                             \
            ASSERT_TRUE(false) << "Input: " << input << "\nCatch error: " << e.what(); \
        }                                                                              \
    }

#endif  // UTIL_H
