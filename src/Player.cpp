#include <Player.hpp>
#include <utility>

Player::Player(std::string name, Color color)
    : Entity(std::move(name), color), attack_timer(0.0f)
{
}

void Player::cancelAttack()
{
    // Devuelve al territorio el combustible que no se haya gastado.
    troops += attack_fuel;
    attack_fuel = 0;
    target = Target::none();
    attack_timer = 0.0f;
}

bool Player::shouldAttack(float deltaTime, float interval)
{
    attack_timer += deltaTime;
    if (attack_timer < interval) return false;

    // Conserva el tiempo sobrante para mantener el intervalo estable entre frames.
    attack_timer -= interval;
    return true;
}
