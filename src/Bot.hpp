#pragma once

#include <raylib.h>
#include <Entity.hpp>

class Bot : public Entity
{
public:
    explicit Bot(Color color);

    bool shouldMove(float deltaTime, float interval);
    void reset();

private:
    float move_timer;
};
