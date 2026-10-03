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

    Player player;
    Bot bot;
    std::vector<TerritoryCell> map;
    float game_time;
    bool game_over;
    Owner winner;
};
