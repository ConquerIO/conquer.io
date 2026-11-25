#include <Bot.hpp>
#include <cstdlib>

#define BOT_INITIAL_TROOPS 350

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

Bot::Bot(const std::string &name, Color color, const BotTuning &tuning) : Entity(name, color), tuning(tuning)
{
    this->troops = BOT_INITIAL_TROOPS;
    reset();
}

bool Bot::shouldStartAttack(float deltaTime)
{
    attack_decision_timer += deltaTime;
    if (attack_decision_timer < next_attack_delay) return false;

    attack_decision_timer = 0.0f;
    next_attack_delay = randomRange(tuning.attack_delay_min, tuning.attack_delay_max);
    attack_ratio = randomRange(tuning.min_attack_ratio, tuning.max_attack_ratio);
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

bool Bot::shouldRequestAlliance(float deltaTime){
    if (!tuning.allow_player_alliances && !tuning.allow_bot_alliances) return false;

    alliance_timer -= deltaTime;
    if (alliance_timer > 0.0f) {
        return false;
    }
    //un bot intenta iniciar una alianza dependiendo de la dificultad (GameConfig.hpp)
    alliance_timer = randomRange(tuning.alliance_request_interval_min, tuning.alliance_request_interval_max);
    return true;
}

bool Bot::shouldDonate(float deltaTime){
    if (!tuning.allow_bot_donations) return false;

    donation_timer -= deltaTime;
    if (donation_timer > 0.0f) return false;

    donation_timer = randomRange(tuning.donation_interval_min, tuning.donation_interval_max);
    return true;
}

void Bot::reset()
{
    attack_decision_timer = 0.0f;
    next_attack_delay = randomRange(
        tuning.first_attack_delay_min,
        tuning.first_attack_delay_max);

    expand_timer = 0.0f;
    attack_ratio = randomRange(
        tuning.min_attack_ratio,
        tuning.max_attack_ratio);

    alliance_timer = randomRange(
        tuning.alliance_request_interval_min,
        tuning.alliance_request_interval_max);

    donation_timer = tuning.allow_bot_donations
        ? randomRange(
              tuning.donation_interval_min,
              tuning.donation_interval_max)
        : 0.0f;

    target = Target::none();
    attack_fuel = 0;
    attack_resistence = 1;
}
