#include <Game.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <utility>
#include <iostream>

using namespace std;

namespace
{
    // Ritmo de crecimiento y expansión territorial.
    constexpr float ATTACK_INTERVAL = 0.12f;

    // Constantes para luego el calculo de los puntos e interes
    constexpr float INITIAL_TROOPS = 500.0f;
    constexpr float INITIAL_INTEREST = 0.07f;
    constexpr float TROOPS_PER_PIXEL = 150.0f;


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

// Probabilidad de que el objetivo de un bot sea una entidad fronteriza.
// El resto de las veces ataca tierra (preferentemente tierra).
constexpr float BOT_ENEMY_TARGET_CHANCE = 0.30f;
}

Game::Game(std::string playerName, Color playerColor, const GameConfig &config):
      config(config),  
      game_time(0.0f),
      tick_timer(0.0f),
      game_over(false),
      winner(nullptr),
      map(config.map_type == MapType::MAP_FILE && !config.map_filename.empty()
          ? Map::fromImage(config.map_filename.c_str())
          : Map::generateMap(200, 200))
{
   
    createEntities(playerName, playerColor);
}

void Game::createEntities(std::string playerName, Color playerColor){
    auto* player = new Player(std::move(playerName), playerColor);
    player->setTroops(config.initial_troops);

    this->entities.push_back(player);

    static const Color BOT_COLORS[15] = {
        contrastingColor(playerColor),
            Color{255, 215, 0, 255},
            Color{128, 0, 128, 255},
            Color{0, 128, 0, 255},
            Color{65, 215, 0, 255},
            Color{128, 34, 234, 255},
            Color{124, 146, 0, 255},
            Color{230, 41, 55, 255},
            Color{53, 0, 128, 255},
            Color{0, 134, 132, 255},
            Color{255, 105, 180, 255},
            Color{0, 191, 255, 255},
            Color{210, 105, 30, 255},
            Color{147, 112, 219, 255},
            Color{0, 250, 154, 255}
    };

    for (std::size_t i=0; i<config.numero_bots && i < 15; ++i){
        auto* bot = new Bot(BOT_COLORS[i]);
        bot->setTroops(config.bot_troops);
        this->entities.push_back(bot);
    }
}


Game::~Game(){
    for (auto entity : entities) delete entity;
}

void Game::resetEntities(std::string playerName, Color playerColor){
    for (auto entity : entities) delete entity;
    this->entities.clear();
    createEntities(playerName, playerColor);
}

void Game::update(float deltaTime)
{   
    if (current_phase == GamePhase::SPAWN) {                                                                                                     
        spawn_timer -= deltaTime;                                                                                                                
        if (spawn_timer <= 0.0f) {                                                                                                               
            placePlayerAndStart();                                                                                                                 
            current_phase = GamePhase::PLAYING;                                                                                                  
        }                                                                                                                                        
        return; // Evita que se ejecute la lógica de juego mientras tanto                                                                        
    } 

    if (game_over) return;

    game_time += deltaTime;

    //Aqui se ajusta es sistema de los ticks para ajustar el crecimiento
    tick_timer += deltaTime;

    const auto player = getPlayer();
    if(getPlayerPixels(player)==0){
        game_over = true;
    }

    for(auto it = entities.begin(); it != entities.end(); ){
        if(*it != player && getPlayerPixels(*it) == 0){
            it = entities.erase(it);
        } else {
            ++it;
        }
    }

    winner = nullptr;
    for (Entity* entity : entities) {
        if (getPlayerPixels(entity) == 0) continue;
        if (winner != nullptr) { winner = nullptr; break; }
        winner = entity;
    }

    while(tick_timer>=TICK_INTERVAL){
        tick_timer-=TICK_INTERVAL;

        for(auto entity: entities) updateTroopGrowth(entity);
    }

    if (!player->getTarget().isNone())
    {
        if (player->shouldAttack(deltaTime, ATTACK_INTERVAL)
            && !expandTerritory(player))
        {
            player->cancelAttack();
        }
    }

    for(int i = 1; i < entities.size(); ++i){
        auto bot = (Bot*)entities[i];

        if (bot->getTarget().isNone())
        {
            if (bot->shouldStartAttack(deltaTime))
            {
                bot->setTarget(chooseBotTarget(bot));
                bot->beginAttack(bot->getAttackRatio());
            }
        }
        else if (bot->shouldExpand(deltaTime, ATTACK_INTERVAL)
                 && !expandTerritory(bot))
        {
            bot->endAttack();
        }
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

    //si nos atacan, se chocan los dos ataques y se reduce su attack_fuel (las tropas que usa para atacar)
    //si el ataque es mutuo se reduce el attack_fuel de
    if (target.isEnemy())
    {
        Entity* enemy = target.getEntity();
        if (enemy != nullptr && enemy->getTarget().getEntity() == owner)
        {
            const std::size_t mine = owner->getAttackFuel();
            const std::size_t enemyFuel = enemy->getAttackFuel();
            enemy->setAttackFuel(enemyFuel > mine ? enemyFuel - mine : 0);
            owner->setAttackFuel(0);
            return false;
        }
    }

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
    const std::size_t resistence= owner->getAttackResistence();

    for (const std::pair<int, int>& cell : frontier)
    {
        if (spent + resistence > fuel) break;
        const int x = cell.first;
        const int y = cell.second;
        if (map.getCell(x, y).owner != owner) continue;

        for (int i = 0; i < 4 && spent + resistence <= fuel; ++i)
        {
            const int nx = x + NEIGHBOR_X[i];
            const int ny = y + NEIGHBOR_Y[i];
            if (nx < 0 || ny < 0 || nx >= columns || ny >= rows) continue;

            TerritoryCell& neighbor = map.getCell(nx, ny);
            if (!isConquerable(neighbor, target)) continue;

            previousOwners.push_back(neighbor.owner);
            neighbor.owner = owner;
            captured.emplace_back(nx, ny);
            spent+=resistence;
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

Entity* Game::randomNeighborEntity(Entity* owner) const
{
    // Entidades distintas de `owner` que tocan su frontera.
    std::vector<Entity*> neighbors;
    const int columns = static_cast<int>(map.getWidth());
    const int rows = static_cast<int>(map.getHeight());

    for (const std::pair<int, int>& cell : owner->getFrontier())
    {
        const int x = cell.first;
        const int y = cell.second;
        for (int i = 0; i < 4; ++i)
        {
            const int nx = x + NEIGHBOR_X[i];
            const int ny = y + NEIGHBOR_Y[i];
            if (nx < 0 || ny < 0 || nx >= columns || ny >= rows) continue;

            Entity* neighborOwner = map.getCell(nx, ny).owner;
            if (neighborOwner == nullptr || neighborOwner == owner) continue;

            if (std::find(neighbors.begin(), neighbors.end(), neighborOwner)
                == neighbors.end())
            {
                neighbors.push_back(neighborOwner);
            }
        }
    }

    if (neighbors.empty()) return nullptr;
    return neighbors[std::rand() % neighbors.size()];
}

Target Game::chooseBotTarget(Entity* owner) const
{
    // Preferentemente tierra; con cierta probabilidad, una entidad fronteriza.
    const float roll = static_cast<float>(std::rand())
        / static_cast<float>(RAND_MAX);

    if (roll < BOT_ENEMY_TARGET_CHANCE)
    {
        Entity* enemy = randomNeighborEntity(owner);
        if (enemy != nullptr) return Target::enemy(enemy);
    }
    return Target::land();
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

void Game::spawnEntity(Entity* entity, int x, int y){
    for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
        for (int columnOffset = -1; columnOffset <= 1; ++columnOffset) {
            int nx = x + columnOffset;
            int ny = y + rowOffset;
            
            // Comprobamos que no se salga de los límites del mapa
            if (nx >= 0 && nx < static_cast<int>(map.getWidth()) && 
                ny >= 0 && ny < static_cast<int>(map.getHeight())) 
            {
                // Solo conquistamos si la celda es tierra y no tiene dueño
                if (!map.getCell(nx, ny).isWater && map.getCell(nx, ny).owner == nullptr) {
                    map.getCell(nx, ny).owner = entity;
                }
            }
        }
    }

    // La base recien colocada define la frontera inicial de la entidad.
    rebuildFrontier(entity);
}


void Game::setSpawnPreview(int targetIndex){
    // Si ya no estamos en fase de cortesía, ignoramos el clic
    if (current_phase != GamePhase::SPAWN) {
        return;
    }

    // Si el clic fue fuera de los límites del mapa, lo ignoramos
    if (targetIndex < 0 || static_cast<std::size_t>(targetIndex) >= map.getCells().size()) {
        return;
    }

    // Comprobamos qué hay en esa celda
    const TerritoryCell& cell = map.getCellFromIndex(targetIndex);
    
    // Solo permitimos elegir si es tierra firme (no agua) y no pertenece a nadie aún
    if (!cell.isWater && cell.owner == nullptr) {
        player_spawn_preview = targetIndex;
    }
}

/*
Algoritmo basado en Poisson Disk Sampling, un método que garantiza que las bases se esparzan de forma natural y orgánica por el mapa evitando que se peguen, 
pero utilizando la versión de Muestreo por Rechazo, que consiste en probar coordenadas al azar y descartar automáticamente aquellas que caigan dentro del radio 
de seguridad de otra base, reduciendo dicho radio si el mapa se llena.
*/
void Game::placeBots(){
    constexpr float MIN_BASE_DISTANCE = 6.0f;                                                                                                
    constexpr int MAX_RADIUS_REDUCTIONS = 20;                                                                                                
                                                                                                                                            
    std::vector<std::pair<int, int>> bases;                                                                                                  
                                                                                                                                            
    int landCells = 0;                                                                                                                       
    for (const auto& cell : map.getCells()) {                                                                                                
        if (!cell.isWater) landCells++;                                                                                                      
    }                                                                                                                                        
                                                                                                                                            
    float R = std::sqrt(static_cast<float>(landCells) / entities.size()) * 0.75f;                                                            
    if (R < MIN_BASE_DISTANCE) R = MIN_BASE_DISTANCE;

    // Bucle para colocar bots                                                                                                               
    for (size_t i = 1; i < entities.size(); ++i) {                                                                                           
        Entity* bot = entities[i];                                                                                                           
        bool botPlaced = false;                                                                                                              
        int reductions = 0;                                                                                                                  
                                                                                                                                                
        while (!botPlaced) {                                                                                                                 
            for (int attempt = 0; attempt < 100; ++attempt) {                                                                                
                int randomX = GetRandomValue(0, map.getWidth() - 1);                                                                         
                int randomY = GetRandomValue(0, map.getHeight() - 1);                                                                        
                                                                                                                                                
                // Tiene que caer en tierra firme                                                                                            
                if (map.getCell(randomX, randomY).isWater) continue;                                                                         
                                                                                                                                                
                bool validPosition = true;                                                                                                   
                for (const auto& base : bases) {                                                                                             
                    float dx = static_cast<float>(randomX - base.first);                                                                     
                    float dy = static_cast<float>(randomY - base.second);                                                                    
                    if (sqrt(dx * dx + dy * dy) < R) {                                                                                  
                        validPosition = false;                                                                                               
                        break;
                    }
                }

                if (validPosition) {
                    spawnEntity(bot, randomX, randomY);
                    bases.push_back({randomX, randomY});
                    botPlaced = true;
                    break;
                }
            }
            
            if (!botPlaced) {
                R *= 0.85f;
                reductions++;
                if (R < MIN_BASE_DISTANCE || reductions >= MAX_RADIUS_REDUCTIONS) {
                    cout << "Mapa saturado. Se colocaron " << i - 1 << " bots." << endl;
                    return; 
                }
            }
        }
    }
}

void Game::placePlayerAndStart(){
    int player_index = player_spawn_preview;

    if (player_index == -1) {
        // Si el jugador no ha elegido un lugar en los 10s, colocamos su base en la primera celda disponible
        do{
            player_index = GetRandomValue(0, map.getCells().size() - 1);  
        } while(map.getCellFromIndex(player_index).isWater || map.getCellFromIndex(player_index).owner != nullptr);
    }

    int player_x = player_index % map.getWidth();
    int player_y = player_index / map.getWidth();

    spawnEntity(getPlayer(), player_x, player_y);
}

void Game::reset()
{
    game_time = 0.0f;
    tick_timer=0.0;
    game_over = false;
    winner = nullptr;
    getPlayer()->cancelAttack();
    if (config.map_type == MapType::MAP_FILE && !config.map_filename.empty()) {
        this->map = Map::fromImage(config.map_filename.c_str());
    } else {
        this->map = Map::generateMap(200, 200);
    }
    
    resetEntities(getPlayer()->getName(), getPlayer()->getColor());

    for (size_t i = 1; i < entities.size(); ++i) {
        static_cast<Bot*>(entities[i])->reset();
    }    

    this->current_phase = GamePhase::SPAWN;
    this->spawn_timer = 13.0f;
    this->player_spawn_preview = -1;
    
    // Llamamos al algoritmo para que esparza a los bots por el mapa
    placeBots();
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


//devuelve el total de pixeles
std::size_t Game::getTerritoryCells(Entity* owner) const {
    if (owner == nullptr) {
        return 0;
    }
    return getPlayerPixels(owner);
}

//devuelve el interes 
float Game::getInterest(Entity* owner) const{
    return calculateInterest(static_cast<float>(owner->getTroops()), static_cast<float>(getPlayerPixels(owner)));
}

//devuelve los limites de tropas
float Game::getTroopLimit(Entity* owner) const {
    return calculateTroopLimit(static_cast<float>(getPlayerPixels(owner)));
}

//devuelve las celdas que son tierra en el mapa 
std::size_t Game::getLandCellCount() const {
    std::size_t land = 0;
    for (const TerritoryCell& cell : map.getCells()){
        if (!cell.isWater) {
            ++land;
        }
    }
    return land;
}