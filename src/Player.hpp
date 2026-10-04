#pragma once

#include <TerritoryCell.hpp>
#include <raylib.h>
#include <string>
#include <vector>
#include <Map.hpp>

class Player
{
public:
    Player(std::string name, Color color);

    const std::string& getName() const { return name; }
    Color getColor() const { return color; }
    void cancelAttack();
    void setTarget(int targetIndex);
    int getTargetIndex() const { return target_index; }
    bool shouldAttack(float deltaTime, float interval);
    int getNextTarget(Map& map, int columns, int rows) const;

private:
    std::string name;
    Color color;
    int target_index;
    float attack_timer;
};
