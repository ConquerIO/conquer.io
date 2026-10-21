#include <Bot.hpp>
#include <cstdlib>

#define BOT_INITIAL_TROOPS 250

namespace
{
// Valor aleatorio uniforme en [min, max].
float randomRange(float min, float max)
{
    const float t = static_cast<float>(std::rand())
        / static_cast<float>(RAND_MAX);
    return min + t * (max - min);
}
}

Bot::Bot(const std::string &name, Color color) : Entity(name, color)
{
    this->troops = BOT_INITIAL_TROOPS;
    reset();
}

bool Bot::shouldStartAttack(float deltaTime)
{
    attack_decision_timer += deltaTime;
    if (attack_decision_timer < next_attack_delay) return false;

    attack_decision_timer = 0.0f;
    next_attack_delay = randomRange(6.0f, 10.0f);
    attack_ratio = randomRange(0.30f, 0.70f);
    expand_timer = 0.0f;
    return true;
}

bool Bot::shouldExpand(float deltaTime, float interval)
{
    expand_timer += deltaTime;
    if (expand_timer < interval) return false;
    expand_timer -= interval;
    return true;
}

void Bot::reset()
{
    attack_decision_timer = 0.0f;
    next_attack_delay = randomRange(6.0f, 10.0f);
    expand_timer = 0.0f;
    attack_ratio = randomRange(0.30f, 0.70f);
    target = Target::none();
    attack_fuel = 0;
    attack_resistence=1;
}
