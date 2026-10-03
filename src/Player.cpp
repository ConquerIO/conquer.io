#include <Player.hpp>
#include <cmath>
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

    attack_timer -= interval;
    return true;
}

int Player::getNextTarget(const std::vector<TerritoryCell>& map,
                          int columns, int rows) const
{
    if (target_index < 0 || map[target_index].owner == Owner::Player) return -1;

    const int targetColumn = target_index % columns;
    const int targetRow = target_index / columns;
    int bestDistance = columns + rows;
    int bestTroops = -1;
    int bestTarget = -1;

    for (int sourceIndex = 0; sourceIndex < static_cast<int>(map.size()); ++sourceIndex)
    {
        const TerritoryCell& source = map[sourceIndex];
        if (source.owner != Owner::Player || source.troops < 2.0f) continue;

        const int column = sourceIndex % columns;
        const int row = sourceIndex / columns;
        const int neighbors[] = {
            row > 0 ? sourceIndex - columns : -1,
            row + 1 < rows ? sourceIndex + columns : -1,
            column > 0 ? sourceIndex - 1 : -1,
            column + 1 < columns ? sourceIndex + 1 : -1
        };

        for (const int neighbor : neighbors)
        {
            if (neighbor < 0 || map[neighbor].owner == Owner::Player
                || (map[neighbor].protected_from == Owner::Player
                    && map[neighbor].capture_protection > 0.0f)) continue;

            const int neighborColumn = neighbor % columns;
            const int neighborRow = neighbor / columns;
            const int distance = std::abs(neighborColumn - targetColumn)
                               + std::abs(neighborRow - targetRow);
            const int availableTroops = static_cast<int>(source.troops);
            if (distance < bestDistance
                || (distance == bestDistance && availableTroops > bestTroops))
            {
                bestDistance = distance;
                bestTroops = availableTroops;
                bestTarget = neighbor;
            }
        }
    }

    return bestTarget;
}
