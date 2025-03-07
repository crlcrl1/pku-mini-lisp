#include "test_util.h"

#include <algorithm>
#include <stdexcept>

bool isClose(const double a, const double b) {
    double err = std::max(1e-6, 1e-4 * (a + b) / 2);
    return std::abs(a - b) < err;
}

bool testEqual(const std::string& a, const std::string& b) {
    try {
        const double aNum = std::stod(a);
        const double bNum = std::stod(b);
        return isClose(aNum, bNum);
    } catch (std::invalid_argument&) {
        return a == b;
    }
}