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


int colorDistance(Color a, Color b)
{
    const int redDifference = static_cast<int>(a.r) - b.r;
    const int greenDifference = static_cast<int>(a.g) - b.g;
    const int blueDifference = static_cast<int>(a.b) - b.b;
    return redDifference * redDifference
        + greenDifference * greenDifference
        + blueDifference * blueDifference;
}

Color contrastingColor(Color color)
{
    const Color red{235, 65, 75, 255};
    const Color cyan{45, 205, 220, 255};
    return colorDistance(color, red) > colorDistance(color, cyan) ? red : cyan;
}

bool isConquerable(const TerritoryCell& cell, const Target& target)
{
    if (cell.isWater) return false;

    if (target.isLand()) return cell.owner == nullptr;
    if (target.isEnemy()) return cell.owner == target.getEntity();
    return false;
}

// Desplazamientos de los 4 vecinos: arriba, abajo, izquierda, derecha.
constexpr int NEIGHBOR_X[4] = {0, 0, -1, 1};
constexpr int NEIGHBOR_Y[4] = {-1, 1, 0, 0};
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
        bot->beginAttack(1.0f);
        expandTerritory(bot);
    }
}

/*
    Algoritmo de expansión

    Funcionamiento: obtiene la frontera del territorio y
    la va expandiendo mientras decrementa el "attack fuel" (tropas
    destinadas al ataque) del atacante.

    Por último, recalcula la frontera para futuras expansiones.

    Esta versión es una optimización importante en cuanto a rendimiento,
    ya que no se tienen en consideración los píxeles interiores, lo que
    reduce drásticamente el costo computacional.
*/
bool Game::expandTerritory(Entity* owner)
{
    const Target& target = owner->getTarget();
    if (target.isNone()) return false;

    const std::size_t fuel = owner->getAttackFuel();
    if (fuel == 0) return false;

    std::vector<std::pair<int, int>>& frontier = owner->getFrontier();
    if (frontier.empty())
    {
        rebuildFrontier(owner);
        if (frontier.empty()) return false;
    }

    const int columns = static_cast<int>(map.getWidth());
    const int rows = static_cast<int>(map.getHeight());

    std::vector<std::pair<int, int>> captured;
    std::vector<Entity*> previousOwners;
    std::size_t spent = 0;

    for (const std::pair<int, int>& cell : frontier)
    {
        if (spent >= fuel) break;
        const int x = cell.first;
        const int y = cell.second;
        if (map.getCell(x, y).owner != owner) continue;

        for (int i = 0; i < 4 && spent < fuel; ++i)
        {
            const int nx = x + NEIGHBOR_X[i];
            const int ny = y + NEIGHBOR_Y[i];
            if (nx < 0 || ny < 0 || nx >= columns || ny >= rows) continue;

            TerritoryCell& neighbor = map.getCell(nx, ny);
            if (!isConquerable(neighbor, target)) continue;

            previousOwners.push_back(neighbor.owner);
            neighbor.owner = owner;
            captured.emplace_back(nx, ny);
            ++spent;
        }
    }

    if (captured.empty()) return false;

    owner->setAttackFuel(fuel - spent);

    std::vector<std::pair<int, int>> nextFrontier;
    nextFrontier.reserve(frontier.size() + captured.size());
    for (const std::pair<int, int>& cell : frontier)
    {
        if (isFrontierCell(cell.first, cell.second, owner))
            nextFrontier.push_back(cell);
    }
    for (const std::pair<int, int>& cell : captured)
    {
        if (isFrontierCell(cell.first, cell.second, owner))
            nextFrontier.push_back(cell);
    }
    frontier.swap(nextFrontier);

    for (std::size_t i = 0; i < captured.size(); ++i)
    {
        Entity* victim = previousOwners[i];
        if (victim == nullptr || victim == owner) continue;
        addExposedFrontier(victim, captured[i].first, captured[i].second);
    }

    return true;
}

bool Game::isFrontierCell(int x, int y, Entity* owner) const
{
    const TerritoryCell& cell = map.getCell(x, y);
    if (cell.isWater || cell.owner != owner) return false;

    const int columns = static_cast<int>(map.getWidth());
    const int rows = static_cast<int>(map.getHeight());

    // Es frontera si algun vecino es tierra (neutral) o territorio enemigo.
    for (int i = 0; i < 4; ++i)
    {
        const int nx = x + NEIGHBOR_X[i];
        const int ny = y + NEIGHBOR_Y[i];
        if (nx < 0 || ny < 0 || nx >= columns || ny >= rows) continue;

        const TerritoryCell& neighbor = map.getCell(nx, ny);
        if (!neighbor.isWater && neighbor.owner != owner) return true;
    }

    return false;
}

void Game::rebuildFrontier(Entity* owner)
{
    std::vector<std::pair<int, int>>& frontier = owner->getFrontier();
    frontier.clear();

    const int columns = static_cast<int>(map.getWidth());
    const int rows = static_cast<int>(map.getHeight());
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < columns; ++x)
        {
            if (isFrontierCell(x, y, owner)) frontier.emplace_back(x, y);
        }
    }
}

void Game::addExposedFrontier(Entity* victim, int x, int y)
{
    const int columns = static_cast<int>(map.getWidth());
    const int rows = static_cast<int>(map.getHeight());

    std::vector<std::pair<int, int>>& frontier = victim->getFrontier();
    for (int i = 0; i < 4; ++i)
    {
        const int nx = x + NEIGHBOR_X[i];
        const int ny = y + NEIGHBOR_Y[i];
        if (nx < 0 || ny < 0 || nx >= columns || ny >= rows) continue;

        if (map.getCell(nx, ny).owner != victim) continue;
        if (!isFrontierCell(nx, ny, victim)) continue;

        const std::pair<int, int> neighbor{nx, ny};
        if (std::find(frontier.begin(), frontier.end(), neighbor) == frontier.end())
        {
            frontier.push_back(neighbor);
        }
    }
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

    // La base recien colocada define la frontera inicial de la entidad.
    rebuildFrontier(entity);
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
