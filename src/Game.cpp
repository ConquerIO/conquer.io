#include <Game.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
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
}

Game::Game(std::string playerName, Color playerColor)
    : player(std::move(playerName), playerColor),
      bot(contrastingColor(playerColor)),
      game_time(0.0f),
      game_over(false),
      winner(Owner::Neutral)
{
}

void Game::update(float deltaTime)
{
    if (game_over) return;

    game_time += deltaTime;
    for (TerritoryCell& cell : map)
    {
        cell.combat_timer = std::max(0.0f, cell.combat_timer - deltaTime);
        cell.capture_protection = std::max(0.0f, cell.capture_protection - deltaTime);
        if (cell.capture_protection == 0.0f) cell.protected_from = Owner::Neutral;
        if (cell.owner != Owner::Neutral && cell.combat_timer == 0.0f)
        {
            cell.troops += TROOP_GROWTH_PER_SECOND * deltaTime;
        }
    }

    int playerCapturedTarget = -1;
    if (player.getTargetIndex() >= 0)
    {
        if (map[player.getTargetIndex()].owner == Owner::Player)
        {
            player.cancelAttack();
        }
        else if (player.shouldAttack(deltaTime, ATTACK_INTERVAL))
        {
            const int nextTarget = player.getNextTarget(map, MAP_COLUMNS, MAP_ROWS);
            if (nextTarget >= 0)
            {
                const Owner previousOwner = map[nextTarget].owner;
                attack(nextTarget, Owner::Player);
                if (previousOwner != Owner::Player
                    && map[nextTarget].owner == Owner::Player)
                {
                    playerCapturedTarget = nextTarget;
                }
            }
        }
    }

    if (bot.shouldMove(deltaTime, ATTACK_INTERVAL))
    {
        const int targetIndex =
            bot.getNextTarget(map, MAP_COLUMNS, MAP_ROWS, playerCapturedTarget);
        if (targetIndex >= 0) attack(targetIndex, Owner::Bot);
    }
}

void Game::reset()
{
    map.assign(MAP_COLUMNS * MAP_ROWS, TerritoryCell{});
    game_time = 0.0f;
    player.cancelAttack();
    bot.reset();
    game_over = false;
    winner = Owner::Neutral;

    const int baseRow = MAP_ROWS / 2;
    const int playerBaseColumn = 5;
    const int botBaseColumn = MAP_COLUMNS - 6;
    for (int rowOffset = -1; rowOffset <= 1; ++rowOffset)
    {
        for (int columnOffset = -1; columnOffset <= 1; ++columnOffset)
        {
            TerritoryCell& playerCell = map[(baseRow + rowOffset) * MAP_COLUMNS
                                            + playerBaseColumn + columnOffset];
            playerCell.owner = Owner::Player;
            playerCell.troops = rowOffset == 0 && columnOffset == 0 ? 48.0f : 8.0f;

            TerritoryCell& botCell = map[(baseRow + rowOffset) * MAP_COLUMNS
                                         + botBaseColumn + columnOffset];
            botCell.owner = Owner::Bot;
            botCell.troops = rowOffset == 0 && columnOffset == 0 ? 48.0f : 8.0f;
        }
    }

    map[baseRow * MAP_COLUMNS + playerBaseColumn].is_base = true;
    map[baseRow * MAP_COLUMNS + botBaseColumn].is_base = true;
}

void Game::setPlayerTarget(int targetIndex)
{
    if (targetIndex >= 0 && targetIndex < static_cast<int>(map.size())
        && map[targetIndex].owner != Owner::Player)
    {
        player.setTarget(targetIndex);
    }
}

void Game::cancelPlayerAttack()
{
    player.cancelAttack();
}

void Game::attack(int targetIndex, Owner attacker)
{
    TerritoryCell& target = map[targetIndex];
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

    TerritoryCell& source = map[sourceIndex];
    if (source.troops < 2.0f) return;
    const int force = static_cast<int>(source.troops * 0.5f);
    source.troops -= force;
    target.combat_timer = COMBAT_STALE_DURATION;

    if (force >= target.troops)
    {
        const bool capturedFromOpponent =
            target.owner != Owner::Neutral && target.owner != attacker;
        const bool capturedBase = target.is_base;
        target.protected_from = capturedFromOpponent ? target.owner : Owner::Neutral;
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
