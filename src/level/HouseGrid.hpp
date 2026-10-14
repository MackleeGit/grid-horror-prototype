#ifndef HOUSE_GRID_HPP
#define HOUSE_GRID_HPP

#include <vector>
#include <string>

namespace cp1 {

using Grid = std::vector<std::string>;

// Symbolic map encoding:
// '1' = solid wall block
// '0' = corridor floor
// 'H' = closet / hiding spot
// 'F' = fuse box objective
inline Grid defaultHouse() {
    return {
        "1111111111111111",
        "1000000100000001",
        "10F00000100H0001",
        "1000111100000001",
        "1000100000011101",
        "1H00100000010001",
        "1000100111010F01",
        "1000000100010001",
        "1011110100000001",
        "1000000000111101",
        "1F0000100000H001",
        "1111111111111111",
    };
}

inline char cellAt(const Grid& g, int col, int row) {
    if (row < 0 || row >= static_cast<int>(g.size()) ||
        col < 0 || col >= static_cast<int>(g[row].size())) {
        return '1'; // boundary defaults to solid wall
    }
    return g[row][col];
}

} // namespace cp1

#endif // HOUSE_GRID_HPP
