#pragma once
#include <GameState.hpp>
#include <Game.hpp>
#include <GameConfig.hpp>
#include <DebugOverlay.hpp>
#include <ContextMenu.hpp>
#include <raylib.h>
#include <string>
#include <iostream>
#include <vector>

class MainGameState : public GameState
{
    public:
        MainGameState(std::string playerName, Color playerColor, const GameConfig &config);
        ~MainGameState() override;

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
        void renderMap();

        Game game;
        Camera2D map_camera{};
        RenderTexture2D map_texture{};
        bool map_texture_ready = false;
        std::vector<Entity*> map_cell_owners;
        float camera_zoom = 1.0f;
        Vector2 camera_pan{0.0f, 0.0f};
        bool dragging_camera = false;
        float camera_drag_distance = 0.0f;
        int attack_percent = 100;
        bool dragging_attack_slider = false;

        GameConfig config;
        float game_over_timer = 0.0f;

        DebugOverlay debug_overlay;

        //funciones aux para el menu
        bool isCellAttackable(int column, int row) const;
        bool playerHasNeutralFrontier() const;
        bool playerIsAdjacentTo(Entity* enemy) const;
        void executeContextAttack();
        ContextMenu context_menu;
};