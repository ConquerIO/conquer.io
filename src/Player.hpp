#pragma once

#include <TerritoryCell.hpp>
#include <raylib.h>
#include <string>
#include <Entity.hpp>

class Player: public Entity
{
public:
    Player(std::string name, Color color);

    void cancelAttack();
    bool shouldAttack(float deltaTime, float interval);

private:
    float attack_timer;
};
