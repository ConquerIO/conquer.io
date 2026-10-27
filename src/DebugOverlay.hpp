#pragma once

#include <SystemStats.hpp>

class Game;
struct GameConfig;

class DebugOverlay
{
public:
    void toggle() { visible = !visible; }
    bool isVisible() const { return visible; }

    //para dibujar la pantalla, (los dos paneles)
    void draw(const Game& game, const GameConfig& config, float deltaTime);

private:
    void drawGamePanel(const Game& game, const GameConfig& config);
    void drawTechPanel(const Game& game);

    bool visible = false;
    SystemStats system;

    //para ver los cambios de los fps durante el juego, se almacena en el array
    static constexpr int FPS_HISTORY = 120;
    float fps_history[FPS_HISTORY] = {};
    int fps_head = 0;   
    int fps_count = 0;  
};
