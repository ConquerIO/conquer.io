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

    // Reinicia el temporizador al habilitar un movimiento del bot.
    move_timer = 0.0f;
    return true;
}

int Bot::getNextTarget(Map& map, int columns, int rows, int excludedTarget)
{
    // Separa ataques al jugador de la expansión para dar prioridad al combate directo.
    std::vector<std::pair<int, int>> playerTargets;
    std::vector<std::pair<int, int>> expansionTargets;
    int closestColumn = columns;

    // Reúne objetivos vecinos de todas las celdas del bot que pueden atacar.
    for (size_t sourceIndex = 0; sourceIndex < map.getCells().size(); ++sourceIndex)
    {
        const TerritoryCell& source = map.getCellFromIndex(sourceIndex);
        if (source.owner != Owner::Bot || source.troops < 2.0f) continue;

        const size_t column = sourceIndex % columns;
        const size_t row = sourceIndex / columns;
        const size_t neighbors[] = {
            row > 0 ? sourceIndex - columns : -1,
            row + 1 < rows ? sourceIndex + columns : -1,
            column > 0 ? sourceIndex - 1 : -1,
            column + 1 < columns ? sourceIndex + 1 : -1
        };
        for (const size_t targetIndex : neighbors)
        {
            const auto target = map.getCellFromIndex(targetIndex);

            if (targetIndex < 0 || targetIndex == excludedTarget
                || (target.protected_from == Owner::Bot && target.capture_protection > 0.0f)) continue;
            if (target.owner == Owner::Player)
            {
                playerTargets.emplace_back(sourceIndex, targetIndex);
            }
            else if (target.owner == Owner::Land)
            {
                // Heurística voraz de expansión: elige columnas más cercanas a la base
                // del jugador (columna 5), sin calcular una ruta completa.
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

    // No es un algoritmo de IA estándar: prioriza jugadores y reparte las elecciones
    // restantes en ciclo (round-robin) para alternar entre objetivos equivalentes.
    const auto& targets = playerTargets.empty() ? expansionTargets : playerTargets;
    if (targets.empty()) return -1;

    return targets[move_count++ % targets.size()].second;
}

void Bot::reset()
{
    // Restablece el ritmo y el orden cíclico de selección para una nueva partida.
    move_timer = 0.0f;
    move_count = 0;
}
