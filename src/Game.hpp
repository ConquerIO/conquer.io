#pragma once

#include <Bot.hpp>
#include <Player.hpp>
#include <TerritoryCell.hpp>
#include <Target.hpp>
#include <Map.hpp>
#include <GameConfig.hpp>
#include <cstddef>
#include <string>
#include <vector>

using namespace std;

enum class GamePhase {
    SPAWN,
    PLAYING
};

class Game
{
public:
    Game(std::string playerName, Color playerColor, const GameConfig &config);
    ~Game();
    // Game es dueño de las entidades, no se puede copiar.
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;


    void update(float deltaTime);
    void reset();
    void setPlayerTarget(const Target& target, float ratio);
    void cancelPlayerAttack();

    Player* getPlayer() const { return (Player*)entities[0];}
    const Map& getMap() const { return map; }
    float getTime() const { return game_time; }
    bool isOver() const { return game_over; }
    Entity* getWinner() const { return winner; }
    std::vector<Entity*> getEntities() const {return entities;}

    //intervalo de ticks
    static constexpr float TICK_INTERVAL = 1.0f;

    float getTickTimer() const {
        return tick_timer;
    }
    float getInterest(Entity* owner) const;
    float getTroopLimit(Entity* owner) const;
    std::size_t getTerritoryCells(Entity* owner) const;
    std::size_t getLandCellCount() const;

    void setSpawnPreview(int targetIndex);
    GamePhase getPhase() const { return current_phase; }
    float getSpawnTimer() const { return spawn_timer; }
    int getSpawnPreview() const { return player_spawn_preview; }
    


private:
    void createEntities(string playerName, Color playerColor);
    void resetEntities(string playerName, Color playerColor);
    void spawnEntity(Entity* entity, int x, int y);
    bool expandTerritory(Entity* owner);
    // Indica si (x, y) es una celda de la frontera de `owner` (toca tierra o enemigo).
    bool isFrontierCell(int x, int y, Entity* owner) const;
    void rebuildFrontier(Entity* owner);
    void addExposedFrontier(Entity* victim, int x, int y);
    Target chooseBotTarget(Entity* owner) const;
    Entity* randomNeighborEntity(Entity* owner) const;
    std::size_t getPlayerPixels(Entity* owner) const;
    float getPlayerTroops(Entity* owner) const;
    float calculateInterest(float troops, float pixels) const;
    float calculateTroopLimit(float pixels) const;
    void updateTroopGrowth(Entity* owner);

    void placePlayerAndStart();
    void placeBots();
    
    GamePhase current_phase;
    float spawn_timer;
    int player_spawn_preview;
    
    std::vector<Entity*> entities;
    Map map;
    float game_time;
    bool game_over;
    //Este es para controlar el tema de los ticks de crecimiento
    float tick_timer;
    Entity* winner;

    GameConfig config;
};
