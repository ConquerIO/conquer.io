#pragma once

#include <Bot.hpp>
#include <Player.hpp>
#include <TerritoryCell.hpp>
#include <string>
#include <vector>

class Game
{
public:
    static constexpr int MAP_COLUMNS = 64;
    static constexpr int MAP_ROWS = 28;

    Game(std::string playerName, Color playerColor);

    void update(float deltaTime);
    void reset();
    void setPlayerTarget(int targetIndex);
    void cancelPlayerAttack();

    const std::vector<TerritoryCell>& getMap() const { return map; }
    const Player& getPlayer() const { return player; }
    const Bot& getBot() const { return bot; }
    float getTime() const { return game_time; }
    bool isOver() const { return game_over; }
    Owner getWinner() const { return winner; }
    

private:
    void attack(int targetIndex, Owner attacker);

    //Estas es para la parte del sistema de puntos
    float getPlayerPixels(Owner owner) const;
    float getPlayerTroops(Owner owner) const;
    float calculateInterest(float troops, float pixels) const;
    float calculateTroopLimit(float pixels) const;
    void updateTroopGrowth(float deltaTime, Owner owner);

    Player player;
    Bot bot;
    std::vector<TerritoryCell> map;
    float game_time;
    bool game_over;
    //Este es para controlar el tema de los ticks de crecimiento
    float tick_timer;
    Owner winner;
};
