#include <Player.hpp>
#include <utility>

Player::Player(std::string name, Color color)
    : name(std::move(name)),
      color(color),
      target_index(-1),
      attack_timer(0.0f)
{
}

void Player::cancelAttack()
{
    target_index = -1;
    attack_timer = 0.0f;
}

void Player::setTarget(int targetIndex)
{
    target_index = targetIndex;
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
