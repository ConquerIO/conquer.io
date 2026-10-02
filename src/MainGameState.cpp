#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <raylib.h>

MainGameState::MainGameState()
{
}

void MainGameState::init()
{
    // TODO: Inicializar mapa, jugadores, bots, etc.
}

void MainGameState::handleInput()
{

}

void MainGameState::update(float deltaTime)
{
    // TODO: Lógica del juego (ticks, tropas, expansión, IA...)
}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(Color{30, 30, 30, 255});

    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    // Indicador de que el estado funciona
    const char* title = "en desarrollo";
    const int titleW = MeasureText(title, 30);
    DrawText(title, (screenW - titleW) / 2, 40, 30, WHITE);


    EndDrawing();
}