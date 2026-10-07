#pragma once

#include <Bot.hpp>
#include <Player.hpp>
#include <TerritoryCell.hpp>
#include <Target.hpp>
#include <Map.hpp>
#include <cstddef>
#include <string>
#include <vector>

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


private:
    void spawnEntity(Entity* entity, int x, int y);
    bool expandTerritory(Entity* owner);
    std::size_t getPlayerPixels(Entity* owner) const;
    float getPlayerTroops(Entity* owner) const;
    float calculateInterest(float troops, float pixels) const;
    float calculateTroopLimit(float pixels) const;
    void updateTroopGrowth(Entity* owner);

    std::vector<Entity*> entities;
    Map map;
    float game_time;
    bool game_over;
    //Este es para controlar el tema de los ticks de crecimiento
    float tick_timer;
    Entity* winner;
};
