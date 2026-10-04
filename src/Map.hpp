#pragma once
#include <vector>
#include <TerritoryCell.hpp>

class Map {
    const unsigned int rows;
    const unsigned int columns;
    const std::vector<TerritoryCell>& cells;

public:
    Map(unsigned int rows, unsigned int columns);
    static Map& fromImage(const char* filename);

    TerritoryCell& getCell(unsigned int x, unsigned int y) const;

};