#pragma once
#include <GameState.hpp>
#include <Game.hpp>
#include <GameConfig.hpp>
#include <DebugOverlay.hpp>
#include <ContextMenu.hpp>
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

        Game game;
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