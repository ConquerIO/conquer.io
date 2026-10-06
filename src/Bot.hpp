#pragma once

#include <raylib.h>

class Bot
{
public:
    explicit Bot(Color color);

    Color getColor() const { return color; }
    bool shouldMove(float deltaTime, float interval);
    void reset();

private:
    Color color;
    float move_timer;
};
