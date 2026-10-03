#pragma once
#include <GameState.hpp>
#include <Game.hpp>
#include <raylib.h>
#include <string>

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
        int getCellIndex(Vector2 position) const;

        Game game;
};