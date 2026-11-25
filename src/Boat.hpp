#pragma once
#include "Entity.hpp"

struct Vector2 {
    int x;
    int y;
}

class Boat {
    Entity* owner;
    Vector2 position;
    Vector2 target;
    unsigned long troops;
}