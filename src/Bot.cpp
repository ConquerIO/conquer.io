#include <Bot.hpp>
#include <cmath>

Bot::Bot(Color color)
    : color(color),
      move_timer(0.0f),
      move_count(0)
{
}

bool Bot::shouldMove(float deltaTime, float interval)
{
    move_timer += deltaTime;
    if (move_timer < interval) return false;

    move_timer = 0.0f;
    return true;
}

int Bot::getNextTarget(const std::vector<TerritoryCell>& map,
                       int columns, int rows, int excludedTarget)
{
    std::vector<std::pair<int, int>> playerTargets;
    std::vector<std::pair<int, int>> expansionTargets;
    int closestColumn = columns;

    for (int sourceIndex = 0; sourceIndex < static_cast<int>(map.size()); ++sourceIndex)
    {
        const TerritoryCell& source = map[sourceIndex];
        if (source.owner != Owner::Bot || source.troops < 2.0f) continue;

        const int column = sourceIndex % columns;
        const int row = sourceIndex / columns;
        const int neighbors[] = {
            row > 0 ? sourceIndex - columns : -1,
            row + 1 < rows ? sourceIndex + columns : -1,
            column > 0 ? sourceIndex - 1 : -1,
            column + 1 < columns ? sourceIndex + 1 : -1
        };
        for (const int targetIndex : neighbors)
        {
            if (targetIndex < 0 || targetIndex == excludedTarget
                || (map[targetIndex].protected_from == Owner::Bot
                    && map[targetIndex].capture_protection > 0.0f)) continue;
            if (map[targetIndex].owner == Owner::Player)
            {
                playerTargets.emplace_back(sourceIndex, targetIndex);
            }
            else if (map[targetIndex].owner == Owner::Neutral)
            {
                const int targetColumn = targetIndex % columns;
                const int distanceToPlayer = std::abs(targetColumn - 5);
                if (distanceToPlayer < closestColumn)
                {
                    closestColumn = distanceToPlayer;
                    expansionTargets.clear();
                }
                if (distanceToPlayer == closestColumn)
                {
                    expansionTargets.emplace_back(sourceIndex, targetIndex);
                }
            }
        }
    }

    const auto& targets = playerTargets.empty() ? expansionTargets : playerTargets;
    if (targets.empty()) return -1;

    return targets[move_count++ % targets.size()].second;
}

void Bot::reset()
{
    move_timer = 0.0f;
    move_count = 0;
}
