#include <Game.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

using namespace std;

namespace
{
    // Ritmo de crecimiento y expansión territorial.
    constexpr float ATTACK_INTERVAL = 0.12f;

    // Constantes para luego el calculo de los puntos e interes
    constexpr float INITIAL_TROOPS = 500.0f;
    constexpr float INITIAL_INTEREST = 0.10f;
    constexpr float TROOPS_PER_PIXEL = 200.0f;
    constexpr float TICK_INTERVAL = 1.0f;


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

bool isConquerable(const TerritoryCell& cell, const Target& target)
{
    if (cell.isWater) return false;

    if (target.isLand()) return cell.owner == nullptr;
    if (target.isEnemy()) return cell.owner == target.getEntity();
    return false;
}
}

Game::Game(std::string playerName, Color playerColor):
      game_time(0.0f),
      tick_timer(0.0f),
      game_over(false),
      winner(nullptr),
      map(Map::generateMap())
{
    this->entities.push_back(new Player(std::move(playerName), playerColor));
    this->entities.push_back(new Bot(contrastingColor(playerColor)));
}

void Game::update(float deltaTime)
{
    if (game_over) return;

    game_time += deltaTime;

    //Aqui se ajusta es sistema de los ticks para ajustar el crecimiento
    tick_timer += deltaTime;

    const auto player = getPlayer();
    const auto bot = (Bot*)entities[1];

    while(tick_timer>=TICK_INTERVAL){
        tick_timer-=TICK_INTERVAL;
        updateTroopGrowth(player);
        updateTroopGrowth(bot);
    }

    if (!player->getTarget().isNone())
    {
        if (player->shouldAttack(deltaTime, ATTACK_INTERVAL)
            && !expandTerritory(player))
        {
            player->cancelAttack();
        }
    }

    if (bot->shouldMove(deltaTime, ATTACK_INTERVAL))
    {
        // El bot destina todas sus tropas al ataque en curso antes de expandirse.
        bot->beginAttack(1.0f);
        expandTerritory(bot);
    }
}

bool Game::expandTerritory(Entity* owner)
{
    const Target& target = owner->getTarget();
    if (target.isNone()) return false;

    std::vector<std::size_t> nextWave;
    const std::size_t columns = map.getWidth();
    const std::size_t rows = map.getHeight();

    for (std::size_t row = 0; row < rows; ++row)
    {
        for (std::size_t column = 0; column < columns; ++column)
        {
            const std::size_t index = row * columns + column;
            if (!isConquerable(map.getCellFromIndex(index), target)) continue;

            const bool touchesOwner =
                (row > 0 && map.getCellFromIndex(index - columns).owner == owner)
                || (row + 1 < rows
                    && map.getCellFromIndex(index + columns).owner == owner)
                || (column > 0
                    && map.getCellFromIndex(index - 1).owner == owner)
                || (column + 1 < columns
                    && map.getCellFromIndex(index + 1).owner == owner);
            if (touchesOwner) nextWave.push_back(index);
        }
    }


    const std::size_t fuel = owner->getAttackFuel();
    if (fuel == 0) return false;

    if (nextWave.size() > fuel)
    {
        nextWave.resize(fuel);
    }

    for (const std::size_t index : nextWave)
    {
        map.getCellFromIndex(index).owner = owner;
    }

    owner->setAttackFuel(fuel - nextWave.size());

    return !nextWave.empty();
}

std::size_t Game::getPlayerPixels(Entity* owner) const {
    std::size_t pixels = 0;
    for (const TerritoryCell& cell : map.getCells()){
        if(cell.owner == owner) pixels++;
    }
    return pixels;
}

float Game::calculateTroopLimit(float pixels) const {
    return pixels*TROOPS_PER_PIXEL;
}

float Game::calculateInterest(float troops, float pixels) const{
    const float limit=calculateTroopLimit(pixels);

    //Si el limite es menor o igual
    if (limit <=INITIAL_TROOPS){
        return 0.0f;
    }

    //si es superior al limite no se puede crecer
    if (troops>=limit){
        return 0.0f;
    }

    const float interest= INITIAL_INTEREST * ((limit-troops) / (limit-INITIAL_TROOPS));

    return max(0.0f, interest);
}

void Game::updateTroopGrowth(Entity* owner){
    const float pixels=getPlayerPixels(owner);
    const float currentTroops= owner->getTroops();
    const float limit=calculateTroopLimit(pixels);
    const float interest=calculateInterest(currentTroops,pixels);

    const float newTroops = currentTroops < 1.0f ? 1.0f : currentTroops * (1.0 + interest);
    owner->setTroops(newTroops);
}

void Game::reset()
{
    game_time = 0.0f;
    tick_timer=0.0;
    game_over = false;
    winner = nullptr;
    getPlayer()->cancelAttack();
    static_cast<Bot*>(entities[1])->reset();

    const int mapWidth = static_cast<int>(map.getWidth());
    const int mapHeight = static_cast<int>(map.getHeight());
    if (mapWidth < 6 || mapHeight < 3)
    {
        throw std::runtime_error("El mapa es demasiado pequeño para colocar las bases.");
    }

    struct BasePosition
    {
        int column;
        int row;
        int playerDistance;
        int botDistance;
    };
    std::vector<BasePosition> candidates;
    const int targetRow = mapHeight / 2;
    const int targetPlayerColumn = mapWidth / 4;
    const int targetBotColumn = mapWidth * 3 / 4;

    for (int row = 1; row < mapHeight - 1; ++row)
    {
        for (int column = 1; column < mapWidth - 1; ++column)
        {
            bool allLand = true;
            for (int rowOffset = -1; rowOffset <= 1 && allLand; ++rowOffset)
            {
                for (int columnOffset = -1; columnOffset <= 1; ++columnOffset)
                {
                    const TerritoryCell& cell =
                        map.getCell(column + columnOffset, row + rowOffset);
                    if (cell.isWater || cell.owner != nullptr)
                    {
                        allLand = false;
                        break;
                    }
                }
            }
            if (!allLand) continue;

            candidates.push_back({
                column,
                row,
                std::abs(column - targetPlayerColumn) + std::abs(row - targetRow),
                std::abs(column - targetBotColumn) + std::abs(row - targetRow)
            });
        }
    }

    bool foundBasePositions = false;
    BasePosition playerBase{};
    BasePosition botBase{};
    int bestDistance = std::numeric_limits<int>::max();
    for (const BasePosition& playerCandidate : candidates)
    {
        for (const BasePosition& botCandidate : candidates)
        {
            if (playerCandidate.column + 2 >= botCandidate.column) continue;

            const int distance = playerCandidate.playerDistance + botCandidate.botDistance;
            if (distance >= bestDistance) continue;

            bestDistance = distance;
            playerBase = playerCandidate;
            botBase = botCandidate;
            foundBasePositions = true;
        }
    }

    if (!foundBasePositions)
    {
        throw std::runtime_error("No hay espacio suficiente en tierra para colocar ambas bases.");
    }

    spawnEntity(getPlayer(), playerBase.column, playerBase.row);
    spawnEntity(entities[1], botBase.column, botBase.row);
}

void Game::spawnEntity(Entity* entity, int x, int y){
    for (int rowOffset = -1; rowOffset <= 1; ++rowOffset)
        for (int columnOffset = -1; columnOffset <= 1; ++columnOffset)
            map.getCell(x + columnOffset, y + rowOffset).owner = entity;
}

void Game::setPlayerTarget(const Target& target, float ratio)
{
    Player* player = getPlayer();
    player->setTarget(target);
    // Destina al ataque la fraccion de tropas elegida (porcentaje del slider).
    player->beginAttack(ratio);
}

void Game::cancelPlayerAttack()
{
    getPlayer()->cancelAttack();
}
