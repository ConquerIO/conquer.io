#include <Map.hpp>

Map::Map(size_t rows, size_t columns): rows(rows), columns(columns), cells(std::vector<TerritoryCell>(rows * columns)) {}

TerritoryCell& Map::getCell(size_t x, size_t y) const {
    return (TerritoryCell&)cells[y * columns + x];
}

TerritoryCell& Map::getCellFromIndex(size_t index) const {
    return (TerritoryCell&)cells[index];
}