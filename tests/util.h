#ifndef UTIL_H
#define UTIL_H

#include <string>

bool isClose(double a, double b);

bool testEqual(const std::string& a, const std::string& b);

#define RUN_TEST(testCase, procFunc)                      \
    for (const auto& [input, output] : testCase.cases) {  \
        if (!output) {                                    \
            continue;                                     \
        }                                                 \
        ASSERT_TRUE(testEqual(procFunc(input), *output)); \
    }

#endif  // UTIL_H
