#pragma once
#include <GameState.hpp>
#include <Game.hpp>
#include <raylib.h>
#include <string>
#include <iostream>

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
        Rectangle getMapBounds() const;
        Rectangle getAttackSliderBounds() const;
        std::pair<int, int> getCellCoordinates(Vector2 position) const;

        Game game;
        int attack_percent = 100;            // Porcentaje de tropas destinado al ataque.
        bool dragging_attack_slider = false;
};