#pragma once

#include <TerritoryCell.hpp>
#include <raylib.h>
#include <vector>

class Bot
{
public:
    explicit Bot(Color color);

    Color getColor() const { return color; }
    bool shouldMove(float deltaTime, float interval);
    int getNextTarget(const std::vector<TerritoryCell>& map,
                      int columns, int rows, int excludedTarget);
    void reset();

private:
    Color color;
    float move_timer;
    std::size_t move_count;
};
