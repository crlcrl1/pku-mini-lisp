#include "location.h"

#include "value.h"

std::optional<Location> Location::fromRange(const std::vector<ValuePtr>& range) {
    if (range.empty()) {
        return std::nullopt;
    }
    int len = 0;
    std::string file;
    int row = -1;
    int col = -1;
    for (const auto& i : range) {
        if (auto& loc = i->getLocation()) {
            len += loc.value().len;
            if (file.empty() || row == -1 || col == -1) {
                file = loc.value().file;
                row = loc.value().row;
                col = loc.value().col;
            }
        }
    }
    if (file.empty()) {
        return std::nullopt;
    }
    return Location{file, row, col, len};
}