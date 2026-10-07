#pragma once
#include <Entity.hpp>

struct TerritoryCell
{
    Entity* owner = nullptr;
    bool isWater;
};
