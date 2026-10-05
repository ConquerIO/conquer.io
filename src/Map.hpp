#pragma once
#include <vector>
#include <TerritoryCell.hpp>

class Map {
    const size_t rows;
    const size_t columns;
    std::vector<TerritoryCell> cells;

public:
    Map(size_t rows, size_t columns);
    static Map fromImage(const char* filename);

    TerritoryCell& getCell(size_t x, size_t y);
    const TerritoryCell& getCell(size_t x, size_t y) const;
    TerritoryCell& getCellFromIndex(size_t index);
    const TerritoryCell& getCellFromIndex(size_t index) const;
    std::vector<TerritoryCell>& getCells() {return cells;}
    const std::vector<TerritoryCell>& getCells() const {return cells;}
    size_t getWidth() const {return columns;}
    size_t getHeight() const {return rows;}

};