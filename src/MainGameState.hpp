#pragma once
#include <GameState.hpp>
#include <raylib.h>
#include <string>
#include <vector>

class MainGameState : public GameState
{
    public:
        MainGameState(std::string playerName, Color playerColor);
        ~MainGameState() = default;

        void init() override;
        void handleInput() override;
        void update(float deltaTime) override;
        void render() override;

        void pause(){};
        void resume(){};

    
    private:
        struct Cell
        {
            int owner = -1;
            float troops = 0.0f;
            float capture_protection = 0.0f;
            float combat_timer = 0.0f;
            int protected_from = -1;
            bool is_base = false;
        };

        static constexpr int MAP_COLUMNS = 64;
        static constexpr int MAP_ROWS = 28;

        Rectangle getMapBounds() const;
        int getCellIndex(Vector2 position) const;
        int getNextPlayerTarget() const;
        void attack(int targetIndex, int attacker);
        void updateBot(int excludedTarget);
        void resetGame();

        std::string player_name;
        Color player_color;
        Color bot_color;
        std::vector<Cell> map;
        float attack_timer;
        float bot_timer;
        float game_time;
        int player_target_index;
        std::size_t bot_move_count;
        bool game_over;
        int winner;
};