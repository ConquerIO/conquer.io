#pragma once
#include <cstddef>
#include <vector>
#include <TerritoryCell.hpp>

class Map {
    const std::size_t rows;
    const std::size_t columns;
    std::vector<TerritoryCell> cells;

public:
    Map(std::size_t rows, std::size_t columns);
    static Map fromImage(const char* filename);

    TerritoryCell& getCell(std::size_t x, std::size_t y);
    const TerritoryCell& getCell(std::size_t x, std::size_t y) const;
    TerritoryCell& getCellFromIndex(std::size_t index);
    const TerritoryCell& getCellFromIndex(std::size_t index) const;
    std::vector<TerritoryCell>& getCells() {return cells;}
    const std::vector<TerritoryCell>& getCells() const {return cells;}
    std::size_t getWidth() const {return columns;}
    std::size_t getHeight() const {return rows;}

};