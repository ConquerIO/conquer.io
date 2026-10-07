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
    constexpr float TROOPS_PER_PIXEL = 10.0f;
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
}

Game::Game(std::string playerName, Color playerColor)
    : player(std::move(playerName), playerColor),
      bot(contrastingColor(playerColor)),
      game_time(0.0f),
      tick_timer(0.0f),
      game_over(false),
      winner(Owner::Land),
      map(Map::fromImage("./maps/map1.png"))
{
    terrain_owners.reserve(map.getCells().size());
    for (const TerritoryCell& cell : map.getCells())
    {
        terrain_owners.push_back(cell.owner);
    }
}

void Game::update(float deltaTime)
{
    if (game_over) return;

    game_time += deltaTime;

    //Aqui se ajusta es sistema de los ticks para ajustar el crecimiento
    tick_timer += deltaTime;

    while(tick_timer>=TICK_INTERVAL){
        tick_timer-=TICK_INTERVAL;
        updateTroopGrowth(Owner::Player);
        updateTroopGrowth(Owner::Bot);
    }

    if (player.getTargetIndex() >= 0)
    {
        if (player.shouldAttack(deltaTime, ATTACK_INTERVAL)
            && !expandTerritory(Owner::Player))
        {
            player.cancelAttack();
        }
    }

    if (bot.shouldMove(deltaTime, ATTACK_INTERVAL))
    {
        expandTerritory(Owner::Bot);
    }
}

bool Game::expandTerritory(Owner owner)
{
    std::vector<std::size_t> nextWave;
    const std::size_t columns = map.getWidth();
    const std::size_t rows = map.getHeight();

    for (std::size_t row = 0; row < rows; ++row)
    {
        for (std::size_t column = 0; column < columns; ++column)
        {
            const std::size_t index = row * columns + column;
            if (map.getCellFromIndex(index).owner != Owner::Land) continue;

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

    for (const std::size_t index : nextWave)
    {
        TerritoryCell& cell = map.getCellFromIndex(index);
        cell.owner = owner;
    }

    return !nextWave.empty();
}

std::size_t Game::getPlayerPixels(Owner owner) const {
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

void Game::updateTroopGrowth(Owner owner){
    //si no tenemos pixeles no podemos expandirnos
    const float pixels=getPlayerPixels(owner);
    if(pixels<=0.0){
        return;
    }

    const float currentTroops= 0; //getPlayerTroops(owner);
    const float limit=calculateTroopLimit(pixels);

    //cuando lleguemos al limite no hay que crecer
    if(currentTroops>=limit){
        return;
    }

    const float interest=calculateInterest(currentTroops,pixels);

    //siguiendo este interés hay que sacar las tropas que se van creciendo con la formula, pero sin superar el limit, y luego añadimos
    float newTroops=currentTroops*(1.0 + interest);
    newTroops=min(newTroops, limit);
    const float troopsToAdd=newTroops-currentTroops;
    if(troopsToAdd<=0.0){
        return;
    }

    //ahora hay que repartir estas tropas por las celdas que tienee este usuario
    int availableCells=0;
    for (TerritoryCell& cell : map.getCells()){
        if(cell.owner==owner){
            availableCells++;
        }
    }
    if(availableCells==0){
        return;
    }

    //se reparten por igual por las celdas del mapa 
    // PENDIENTE DE REFACTORIZACION
    /*
    const float troopsPerCell= troopsToAdd/static_cast<float>(availableCells);
    for (TerritoryCell& cell : map.getCells()){
        if(cell.owner == owner && cell.combat_timer == 0.0){
            cell.troops += troopsPerCell;
        }
    }*/

}

void Game::reset()
{
    game_time = 0.0f;
    tick_timer=0.0;
    player.cancelAttack();
    bot.reset();
    game_over = false;
    winner = Owner::Land;

    for (std::size_t index = 0; index < map.getCells().size(); ++index)
    {
        TerritoryCell& cell = map.getCellFromIndex(index);
        cell.owner = terrain_owners[index];
    }

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
                    if (map.getCell(column + columnOffset, row + rowOffset).owner != Owner::Land)
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

    for (int rowOffset = -1; rowOffset <= 1; ++rowOffset)
    {
        for (int columnOffset = -1; columnOffset <= 1; ++columnOffset)
        {
            TerritoryCell& playerCell =
                map.getCell(playerBase.column + columnOffset, playerBase.row + rowOffset);
            playerCell.owner = Owner::Player;

            TerritoryCell& botCell =
                map.getCell(botBase.column + columnOffset, botBase.row + rowOffset);
            botCell.owner = Owner::Bot;
        }
    }
}

void Game::setPlayerTarget(int targetIndex)
{
    if (targetIndex < 0
        || static_cast<std::size_t>(targetIndex) >= map.getCells().size())
    {
        return;
    }

    const Owner targetOwner = map.getCellFromIndex(targetIndex).owner;
    if (targetOwner == Owner::Land)
    {
        player.setTarget(targetIndex);
    }
    else if (targetOwner != Owner::Player)
    {
        player.cancelAttack();
    }
}

void Game::cancelPlayerAttack()
{
    player.cancelAttack();
}
