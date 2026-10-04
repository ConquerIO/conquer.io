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

    // Conserva el tiempo sobrante para mantener el intervalo estable entre frames.
    attack_timer -= interval;
    return true;
}

int Player::getNextTarget(Map& map,
                          int columns, int rows) const
{
    if (target_index < 0 || map.getCellFromIndex(target_index).owner == Owner::Player) return -1;

    const int targetColumn = target_index % columns;
    const int targetRow = target_index / columns;
    int bestDistance = columns + rows;
    int bestTroops = -1;
    int bestTarget = -1;

    // Examina los territorios propios y propone solo los vecinos atacables.
    for (size_t sourceIndex = 0; sourceIndex < map.getCells().size(); ++sourceIndex)
    {
        const TerritoryCell& source = map.getCellFromIndex(sourceIndex);
        if (source.owner != Owner::Player || source.troops < 2.0f) continue;

        const size_t column = sourceIndex % columns;
        const size_t row = sourceIndex / columns;
        const size_t neighbors[] = {
            row > 0 ? sourceIndex - columns : -1,
            row + 1 < rows ? sourceIndex + columns : -1,
            column > 0 ? sourceIndex - 1 : -1,
            column + 1 < columns ? sourceIndex + 1 : -1
        };

        for (const size_t neighbor : neighbors)
        {
            const auto neighborCell = map.getCellFromIndex(neighbor);
            if (neighbor < 0 || neighborCell.owner == Owner::Player || (neighborCell.protected_from == Owner::Player && neighborCell.capture_protection > 0.0f)) continue;

            const int neighborColumn = neighbor % columns;
            const int neighborRow = neighbor / columns;
            // Distancia Manhattan: cuenta pasos horizontales y verticales en la cuadrícula.
            // Es una heurística voraz para acercarse al destino, no una búsqueda de ruta
            // como A* o BFS; se usa aquí porque basta con elegir el siguiente vecino.
            const int distance = std::abs(neighborColumn - targetColumn)
                               + std::abs(neighborRow - targetRow);
            const int availableTroops = static_cast<int>(source.troops);
            // Prioriza el vecino más cercano al objetivo; en empate, el de más tropas.
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
