#ifndef LOCATION_H
#define LOCATION_H

#include <optional>
#include <string>
#include <vector>

using ValuePtr = class Value*;

struct Location {
    std::string file;
    int row;
    int col;
    int len;

    static std::optional<Location> fromRange(const std::vector<ValuePtr>& range);
};

#endif  // LOCATION_H
