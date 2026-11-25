#pragma once
#include "Entity.hpp"

class Boat {
    Entity* owner;
    Vector2 position;
    Vector2 target;
    unsigned long troops;
};