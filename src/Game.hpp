#pragma once

#include <Bot.hpp>
#include <Player.hpp>
#include <TerritoryCell.hpp>
#include <Target.hpp>
#include <Map.hpp>
#include <cstddef>
#include <string>
#include <vector>

enum class GamePhase {
    SPAWN,
    PLAYING
};

class Game
{
public:
    Game(std::string playerName, Color playerColor);

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

    void setSpawnPreview(int targetIndex);
    GamePhase getPhase() const { return current_phase; }
    float getSpawnTimer() const { return spawn_timer; }
    int getSpawnPreview() const { return player_spawn_preview; }
    


private:
    void spawnEntity(Entity* entity, int x, int y);
    bool expandTerritory(Entity* owner);
    // Indica si (x, y) es una celda de la frontera de `owner` (toca tierra o enemigo).
    bool isFrontierCell(int x, int y, Entity* owner) const;
    // Recalcula desde cero la frontera de `owner` recorriendo todo el mapa.
    void rebuildFrontier(Entity* owner);
    // Anade a la frontera de `victim` las celdas que quedan expuestas al perder
    // la celda (x, y).
    void addExposedFrontier(Entity* victim, int x, int y);
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
};
