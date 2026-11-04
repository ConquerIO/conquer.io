#pragma once
#include <GameState.hpp>
#include <Game.hpp>
#include <GameConfig.hpp>
#include <DebugOverlay.hpp>
#include <raylib.h>
#include <string>
#include <iostream>

class MainGameState : public GameState
{
    public:
        MainGameState(std::string playerName, Color playerColor, const GameConfig &config);
        ~MainGameState() = default;

        void init() override;
        void handleInput() override;
        void update(float deltaTime) override;
        void render() override;

        void pause(){};
        void resume(){};

    
    private:
        Rectangle getMapBounds() const;
        Rectangle getAttackSliderBounds() const;
        std::pair<int, int> getCellCoordinates(Vector2 position) const;
        Camera2D buildMapCamera() const;

        Game game;
        Camera2D map_camera{};
        float camera_zoom = 1.0f;
        Vector2 camera_pan{0.0f, 0.0f};
        bool dragging_camera = false;
        float camera_drag_distance = 0.0f;
        int attack_percent = 100;
        bool dragging_attack_slider = false;

        GameConfig config;
        float game_over_timer = 0.0f;

        DebugOverlay debug_overlay;
};