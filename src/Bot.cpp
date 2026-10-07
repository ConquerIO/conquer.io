#include <Bot.hpp>

Bot::Bot(Color color) : Entity("Bot", color), move_timer(0.0f)
{
    target = Target::land();
}

bool Bot::shouldMove(float deltaTime, float interval)
{
    move_timer += deltaTime;
    if (move_timer < interval) return false;
    move_timer = 0.0f;
    return true;
}

void Bot::reset()
{
    move_timer = 0.0f;
    target = Target::land();
    attack_fuel = 0;
}
