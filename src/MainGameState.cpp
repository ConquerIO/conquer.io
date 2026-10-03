#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
constexpr int NEUTRAL = -1;
constexpr int PLAYER = 0;
constexpr int BOT = 1;
constexpr float TROOP_GROWTH_PER_SECOND = 0.9f;
constexpr float ATTACK_INTERVAL = 0.05f;
constexpr float CAPTURE_PROTECTION_DURATION = 0.1f;
constexpr float COMBAT_STALE_DURATION = 0.25f;

Color contrastingColor(Color color)
{
    const Color red{235, 65, 75, 255};
    const Color cyan{45, 205, 220, 255};
    const auto distance = [color](Color other)
    {
        const int redDifference = static_cast<int>(color.r) - other.r;
        const int greenDifference = static_cast<int>(color.g) - other.g;
        const int blueDifference = static_cast<int>(color.b) - other.b;
        return redDifference * redDifference
            + greenDifference * greenDifference
            + blueDifference * blueDifference;
    };
    return distance(red) > distance(cyan) ? red : cyan;
}

Color cellColor(int owner, Color playerColor, Color botColor)
{
    if (owner == PLAYER) return playerColor;
    if (owner == BOT) return botColor;
    return Color{55, 63, 73, 255};
}
}

MainGameState::MainGameState(std::string playerName, Color playerColor)
    : player_name(std::move(playerName)),
      player_color(playerColor),
      bot_color(contrastingColor(playerColor)),
      attack_timer(0.0f),
      bot_timer(0.0f),
      game_time(0.0f),
      player_target_index(-1),
      bot_move_count(0),
      game_over(false),
      winner(NEUTRAL)
{
}

void MainGameState::init()
{
    resetGame();
}

void MainGameState::handleInput()
{
    if (game_over)
    {
        if (IsKeyPressed(KEY_R)) resetGame();
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ESCAPE))
    {
        player_target_index = -1;
        attack_timer = 0.0f;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const int targetIndex = getCellIndex(GetMousePosition());
        if (targetIndex >= 0 && map[targetIndex].owner != PLAYER)
        {
            player_target_index = targetIndex;
            attack_timer = 0.0f;
        }
    }
}

void MainGameState::update(float deltaTime)
{
    if (game_over) return;

    game_time += deltaTime;
    for (Cell& cell : map)
    {
        cell.combat_timer = std::max(0.0f, cell.combat_timer - deltaTime);
        cell.capture_protection = std::max(0.0f, cell.capture_protection - deltaTime);
        if (cell.capture_protection == 0.0f) cell.protected_from = NEUTRAL;
        if (cell.owner != NEUTRAL && cell.combat_timer == 0.0f)
        {
            cell.troops += TROOP_GROWTH_PER_SECOND * deltaTime;
        }
    }

    int playerCapturedTarget = -1;
    if (player_target_index >= 0)
    {
        if (map[player_target_index].owner == PLAYER)
        {
            player_target_index = -1;
        }
        else
        {
            attack_timer += deltaTime;
            if (attack_timer >= ATTACK_INTERVAL)
            {
                attack_timer -= ATTACK_INTERVAL;
                const int nextTarget = getNextPlayerTarget();
                if (nextTarget >= 0)
                {
                    const int previousOwner = map[nextTarget].owner;
                    attack(nextTarget, PLAYER);
                    if (previousOwner != PLAYER && map[nextTarget].owner == PLAYER)
                    {
                        playerCapturedTarget = nextTarget;
                    }
                }
            }
        }
    }

    bot_timer += deltaTime;
    if (bot_timer >= ATTACK_INTERVAL)
    {
        bot_timer = 0.0f;
        updateBot(playerCapturedTarget);
    }
}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(Color{24, 29, 36, 255});

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    int playerCells = 0;
    int botCells = 0;
    float playerTroops = 0.0f;
    float botTroops = 0.0f;
    for (const Cell& cell : map)
    {
        if (cell.owner == PLAYER)
        {
            ++playerCells;
            playerTroops += cell.troops;
        }
        else if (cell.owner == BOT)
        {
            ++botCells;
            botTroops += cell.troops;
        }
    }
    const int playerPercent = playerCells * 100 / static_cast<int>(map.size());
    const int botPercent = botCells * 100 / static_cast<int>(map.size());

    DrawRectangle(0, 0, screenWidth, 108, Color{31, 38, 47, 255});
    DrawText(player_name.c_str(), 24, 12, 32, player_color);
    DrawText(TextFormat("%d%% territorio", playerPercent), 24, 51, 24, WHITE);
    DrawText(TextFormat("%d tropas", static_cast<int>(playerTroops)),
             24, 78, 22, WHITE);

    const char* title = "CONQUISTA EL MAPA";
    DrawText(title, (screenWidth - MeasureText(title, 30)) / 2, 10, 30, RAYWHITE);
    const int totalSeconds = static_cast<int>(game_time);
    const std::string clock = TextFormat("%02d:%02d", totalSeconds / 60, totalSeconds % 60);
    DrawText(clock.c_str(), (screenWidth - MeasureText(clock.c_str(), 24)) / 2, 55, 24, LIGHTGRAY);

    const char* botName = "BOT";
    const int botNameWidth = MeasureText(botName, 32);
    DrawText(botName, screenWidth - botNameWidth - 24, 12, 32, bot_color);
    const std::string botStats = TextFormat("%d%% territorio", botPercent);
    DrawText(botStats.c_str(), screenWidth - MeasureText(botStats.c_str(), 24) - 24,
             51, 24, WHITE);
    const std::string botTroopStats = TextFormat("%d tropas", static_cast<int>(botTroops));
    DrawText(botTroopStats.c_str(),
             screenWidth - MeasureText(botTroopStats.c_str(), 22) - 24,
             78, 22, WHITE);

    const Rectangle bounds = getMapBounds();
    DrawRectangleRec(bounds, Color{38, 45, 54, 255});
    const float cellWidth = bounds.width / MAP_COLUMNS;
    const float cellHeight = bounds.height / MAP_ROWS;
    const Vector2 mousePosition = GetMousePosition();
    const int hoveredCell = getCellIndex(mousePosition);

    for (int i = 0; i < static_cast<int>(map.size()); ++i)
    {
        const int column = i % MAP_COLUMNS;
        const int row = i / MAP_COLUMNS;
        const Rectangle cellBounds{
            bounds.x + column * cellWidth,
            bounds.y + row * cellHeight,
            cellWidth,
            cellHeight
        };
        const Cell& cell = map[i];
        DrawRectangleRec(cellBounds, cellColor(cell.owner, player_color, bot_color));
        const Color ownershipColor = cell.owner == PLAYER ? player_color
            : cell.owner == BOT ? bot_color : Color{31, 37, 45, 255};
        const float borderThickness = cell.capture_protection > 0.0f ? 3.0f
            : cell.owner == NEUTRAL ? 0.7f : 1.5f;
        DrawRectangleLinesEx(cellBounds, borderThickness,
                             ownershipColor);

        if (cell.is_base)
        {
            DrawRectangleLinesEx(cellBounds, 2.0f, RAYWHITE);
        }
        else if (i == player_target_index && !game_over)
        {
            DrawRectangleLinesEx(cellBounds, 3.0f, GOLD);
        }
        else if (i == hoveredCell && cell.owner != PLAYER && !game_over)
        {
            DrawRectangleLinesEx(cellBounds, 2.0f, GOLD);
        }

        if (cell.owner != NEUTRAL && cell.troops >= 10.0f && cellWidth >= 20.0f)
        {
            const int troops = static_cast<int>(cell.troops);
            DrawText(TextFormat("%d", troops),
                     static_cast<int>(cellBounds.x + 2),
                     static_cast<int>(cellBounds.y + 3),
                     14, RAYWHITE);
        }
    }

    DrawRectangle(0, screenHeight - 64, screenWidth, 64, Color{31, 38, 47, 255});
    const char* instructions = "Clic para avanzar hasta el territorio  |  Clic derecho o ESC para cancelar";
    DrawText(instructions, 24, screenHeight - 41, 21, LIGHTGRAY);
    const char* objective = "Captura la base rival para ganar";
    DrawText(objective, screenWidth - MeasureText(objective, 21) - 24,
             screenHeight - 41, 21, GOLD);

    if (game_over)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.68f));
        const char* result = winner == PLAYER ? "¡VICTORIA!" : "DERROTA";
        const Color resultColor = winner == PLAYER ? GOLD : LIGHTGRAY;
        DrawText(result, (screenWidth - MeasureText(result, 54)) / 2,
                 screenHeight / 2 - 60, 54, resultColor);
        const char* restart = "Pulsa R para jugar otra vez";
        DrawText(restart, (screenWidth - MeasureText(restart, 22)) / 2,
                 screenHeight / 2 + 14, 22, RAYWHITE);
    }

    EndDrawing();
}

Rectangle MainGameState::getMapBounds() const
{
    return Rectangle{
        24.0f,
        124.0f,
        static_cast<float>(std::max(1, GetScreenWidth() - 48)),
        static_cast<float>(std::max(1, GetScreenHeight() - 212))
    };
}

int MainGameState::getCellIndex(Vector2 position) const
{
    const Rectangle bounds = getMapBounds();
    if (position.x < bounds.x || position.y < bounds.y
        || position.x >= bounds.x + bounds.width
        || position.y >= bounds.y + bounds.height)
    {
        return -1;
    }

    const int column = static_cast<int>((position.x - bounds.x) / bounds.width * MAP_COLUMNS);
    const int row = static_cast<int>((position.y - bounds.y) / bounds.height * MAP_ROWS);
    return row * MAP_COLUMNS + column;
}

int MainGameState::getNextPlayerTarget() const
{
    if (player_target_index < 0 || map[player_target_index].owner == PLAYER) return -1;

    const int targetColumn = player_target_index % MAP_COLUMNS;
    const int targetRow = player_target_index / MAP_COLUMNS;
    int bestDistance = MAP_COLUMNS + MAP_ROWS;
    int bestTroops = -1;
    int bestTarget = -1;

    for (int sourceIndex = 0; sourceIndex < static_cast<int>(map.size()); ++sourceIndex)
    {
        const Cell& source = map[sourceIndex];
        if (source.owner != PLAYER || source.troops < 2.0f) continue;

        const int column = sourceIndex % MAP_COLUMNS;
        const int row = sourceIndex / MAP_COLUMNS;
        const int neighbors[] = {
            row > 0 ? sourceIndex - MAP_COLUMNS : -1,
            row + 1 < MAP_ROWS ? sourceIndex + MAP_COLUMNS : -1,
            column > 0 ? sourceIndex - 1 : -1,
            column + 1 < MAP_COLUMNS ? sourceIndex + 1 : -1
        };

        for (const int neighbor : neighbors)
        {
            if (neighbor < 0 || map[neighbor].owner == PLAYER
                || (map[neighbor].protected_from == PLAYER
                    && map[neighbor].capture_protection > 0.0f)) continue;

            const int neighborColumn = neighbor % MAP_COLUMNS;
            const int neighborRow = neighbor / MAP_COLUMNS;
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

void MainGameState::attack(int targetIndex, int attacker)
{
    Cell& target = map[targetIndex];
    if (target.protected_from == attacker && target.capture_protection > 0.0f) return;

    const int column = targetIndex % MAP_COLUMNS;
    const int row = targetIndex / MAP_COLUMNS;
    const int neighbors[] = {
        row > 0 ? targetIndex - MAP_COLUMNS : -1,
        row + 1 < MAP_ROWS ? targetIndex + MAP_COLUMNS : -1,
        column > 0 ? targetIndex - 1 : -1,
        column + 1 < MAP_COLUMNS ? targetIndex + 1 : -1
    };

    int sourceIndex = -1;
    for (const int neighbor : neighbors)
    {
        if (neighbor >= 0 && map[neighbor].owner == attacker
            && (sourceIndex < 0 || map[neighbor].troops > map[sourceIndex].troops))
        {
            sourceIndex = neighbor;
        }
    }
    if (sourceIndex < 0) return;

    Cell& source = map[sourceIndex];
    if (source.troops < 2.0f) return;
    const int force = static_cast<int>(source.troops * 0.5f);
    source.troops -= force;
    target.combat_timer = COMBAT_STALE_DURATION;

    if (force >= target.troops)
    {
        const bool capturedFromOpponent =
            target.owner != NEUTRAL && target.owner != attacker;
        const bool capturedBase = target.is_base;
        target.protected_from = capturedFromOpponent ? target.owner : NEUTRAL;
        target.owner = attacker;
        target.troops = force - target.troops;
        target.capture_protection = capturedFromOpponent
            ? CAPTURE_PROTECTION_DURATION : 0.0f;
        if (capturedBase)
        {
            game_over = true;
            winner = attacker;
        }
    }
    else
    {
        target.troops -= force;
    }
}

void MainGameState::updateBot(int excludedTarget)
{
    std::vector<std::pair<int, int>> playerTargets;
    std::vector<std::pair<int, int>> expansionTargets;
    int closestColumn = MAP_COLUMNS;

    for (int sourceIndex = 0; sourceIndex < static_cast<int>(map.size()); ++sourceIndex)
    {
        const Cell& source = map[sourceIndex];
        if (source.owner != BOT || source.troops < 2.0f) continue;

        const int column = sourceIndex % MAP_COLUMNS;
        const int row = sourceIndex / MAP_COLUMNS;
        const int neighbors[] = {
            row > 0 ? sourceIndex - MAP_COLUMNS : -1,
            row + 1 < MAP_ROWS ? sourceIndex + MAP_COLUMNS : -1,
            column > 0 ? sourceIndex - 1 : -1,
            column + 1 < MAP_COLUMNS ? sourceIndex + 1 : -1
        };
        for (const int targetIndex : neighbors)
        {
            if (targetIndex < 0 || targetIndex == excludedTarget
                || (map[targetIndex].protected_from == BOT
                    && map[targetIndex].capture_protection > 0.0f)) continue;
            if (map[targetIndex].owner == PLAYER)
            {
                playerTargets.emplace_back(sourceIndex, targetIndex);
            }
            else if (map[targetIndex].owner == NEUTRAL)
            {
                const int targetColumn = targetIndex % MAP_COLUMNS;
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
    if (targets.empty()) return;
    const auto [sourceIndex, targetIndex] =
        targets[bot_move_count++ % targets.size()];
    attack(targetIndex, BOT);
}

void MainGameState::resetGame()
{
    map.assign(MAP_COLUMNS * MAP_ROWS, Cell{});
    game_time = 0.0f;
    attack_timer = 0.0f;
    bot_timer = 0.0f;
    player_target_index = -1;
    bot_move_count = 0;
    game_over = false;
    winner = NEUTRAL;

    const int baseRow = MAP_ROWS / 2;
    const int playerBaseColumn = 5;
    const int botBaseColumn = MAP_COLUMNS - 6;
    for (int rowOffset = -1; rowOffset <= 1; ++rowOffset)
    {
        for (int columnOffset = -1; columnOffset <= 1; ++columnOffset)
        {
            Cell& playerCell = map[(baseRow + rowOffset) * MAP_COLUMNS
                                   + playerBaseColumn + columnOffset];
            playerCell.owner = PLAYER;
            playerCell.troops = rowOffset == 0 && columnOffset == 0 ? 48.0f : 8.0f;

            Cell& botCell = map[(baseRow + rowOffset) * MAP_COLUMNS
                                + botBaseColumn + columnOffset];
            botCell.owner = BOT;
            botCell.troops = rowOffset == 0 && columnOffset == 0 ? 48.0f : 8.0f;
        }
    }

    map[baseRow * MAP_COLUMNS + playerBaseColumn].is_base = true;
    map[baseRow * MAP_COLUMNS + botBaseColumn].is_base = true;
}