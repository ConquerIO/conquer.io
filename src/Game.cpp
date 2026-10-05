#include <Game.hpp>
#include <algorithm>
#include <cmath>
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

// constexpr float TROOP_GROWTH_PER_SECOND = 0.9f;
// constexpr float ATTACK_INTERVAL = 0.02f;
// constexpr float CAPTURE_PROTECTION_DURATION = 0.1f;
// constexpr float COMBAT_STALE_DURATION = 0.05f;
// constexpr float ATTACK_FORCE = 0.65f;

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
}

void Game::update(float deltaTime)
{
    if (game_over) return;

    game_time += deltaTime;
    for (TerritoryCell& cell : map.getCells())
    {
        // Los temporizadores se reducen usando deltaTime para que el ritmo no dependa de los FPS.
        cell.combat_timer = std::max(0.0f, cell.combat_timer - deltaTime);
        cell.capture_protection = std::max(0.0f, cell.capture_protection - deltaTime);
        if (cell.capture_protection == 0.0f) cell.protected_from = Owner::Land;
        // Las tropas crecen solo en territorios propios que ya no están en combate.
        /*if (cell.owner != Owner::Land && cell.combat_timer == 0.0f)
        {
            cell.troops += TROOP_GROWTH_PER_SECOND * deltaTime;
        }*/
    }


    //Aqui se ajusta es sistema de los ticks para ajustar el crecimiento
    tick_timer += deltaTime;

    while(tick_timer>=TICK_INTERVAL){
        tick_timer-=TICK_INTERVAL;
        updateTroopGrowth(deltaTime, Owner::Player);
        updateTroopGrowth(deltaTime, Owner::Bot);
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
    std::vector<int> nextWave;

    for (int row = 0; row < MAP_ROWS; ++row)
    {
        for (int column = 0; column < MAP_COLUMNS; ++column)
        {
            const int index = row * MAP_COLUMNS + column;
            if (map.getCellFromIndex(index).owner != Owner::Land) continue;

            const bool touchesOwner =
                (row > 0 && map.getCellFromIndex(index - MAP_COLUMNS).owner == owner)
                || (row + 1 < MAP_ROWS
                    && map.getCellFromIndex(index + MAP_COLUMNS).owner == owner)
                || (column > 0
                    && map.getCellFromIndex(index - 1).owner == owner)
                || (column + 1 < MAP_COLUMNS
                    && map.getCellFromIndex(index + 1).owner == owner);
            if (touchesOwner) nextWave.push_back(index);
        }
    }

    for (const int index : nextWave)
    {
        TerritoryCell& cell = map.getCellFromIndex(index);
        cell.owner = owner;
        cell.troops = 0.0f;
        cell.combat_timer = 0.0f;
        cell.capture_protection = 0.0f;
        cell.protected_from = Owner::Land;
    }

    return !nextWave.empty();
}

size_t Game::getPlayerPixels(Owner owner) const {
    size_t pixels = 0;
    for (const TerritoryCell& cell : map.getCells()){
        if(cell.owner == owner) pixels++;
    }
    return pixels;
}

size_t Game::getPlayerTroops(Owner owner) const{
    size_t troops= 0;
    for (const TerritoryCell& cell : map.getCells()){
        if(cell.owner == owner) troops += cell.troops;
    }
    return troops;
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

void Game::updateTroopGrowth(float deltaTime, Owner owner){
    //si no tenemos pixeles no podemos expandirnos
    const float pixels=getPlayerPixels(owner);
    if(pixels<=0.0){
        return;
    }

    const float currentTroops=getPlayerTroops(owner);
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
        if(cell.owner==owner && cell.combat_timer==0.0){
            availableCells++;
        }
    }
    if(availableCells==0){
        return;
    }

    //se reparten por igual por las celdas del mapa 
    const float troopsPerCell= troopsToAdd/static_cast<float>(availableCells);
    for (TerritoryCell& cell : map.getCells()){
        if(cell.owner == owner && cell.combat_timer == 0.0){
            cell.troops += troopsPerCell;
        }
    }

}

void Game::reset()
{
    game_time = 0.0f;
    tick_timer=0.0;
    player.cancelAttack();
    bot.reset();
    game_over = false;
    winner = Owner::Land;

    // Las bases empiezan sobre tierra transitable; cada una ocupa un bloque de 3x3.
    const int baseRow = MAP_ROWS / 2;
    const int playerBaseColumn = 14;
    const int botBaseColumn = 50;
    for (int rowOffset = -1; rowOffset <= 1; ++rowOffset)
    {
        for (int columnOffset = -1; columnOffset <= 1; ++columnOffset)
        {
            TerritoryCell& playerCell = map.getCellFromIndex((baseRow + rowOffset) * MAP_COLUMNS
                                            + playerBaseColumn + columnOffset);
            playerCell.owner = Owner::Player;
            playerCell.troops = rowOffset == 0 && columnOffset == 0 ? 48.0f : 8.0f;

            TerritoryCell& botCell = map.getCellFromIndex((baseRow + rowOffset) * MAP_COLUMNS
                                         + botBaseColumn + columnOffset);
            botCell.owner = Owner::Bot;
            botCell.troops = rowOffset == 0 && columnOffset == 0 ? 48.0f : 8.0f;
        }
    }

    map.getCell(playerBaseColumn, baseRow).is_base = true;
    map.getCell(botBaseColumn, baseRow).is_base = true;
}

void Game::setPlayerTarget(int targetIndex)
{
    if (targetIndex < 0 || targetIndex >= static_cast<int>(map.getCells().size()))
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
