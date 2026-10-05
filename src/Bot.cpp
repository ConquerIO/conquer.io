#include <Bot.hpp>

Bot::Bot(Color color)
    : color(color),
      move_timer(0.0f)
{
}

bool Bot::shouldMove(float deltaTime, float interval)
{
    move_timer += deltaTime;
    if (move_timer < interval) return false;

    // Reinicia el temporizador al habilitar un movimiento del bot.
    move_timer = 0.0f;
    return true;
}

void Bot::reset()
{
    // Reinicia el ritmo de expansión para una nueva partida.
    move_timer = 0.0f;
}
