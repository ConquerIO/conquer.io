#pragma once

enum class Owner
{
    Neutral, // Casilla sin dueño.
    Player,  // Controlada por el jugador.
    Bot      // Controlada por el bot.
};

struct TerritoryCell
{
    Owner owner = Owner::Neutral;
    float troops = 0.0f; // Tropas estacionadas en la casilla.
    float capture_protection = 0.0f; // Tiempo restante de protección tras una captura enemiga.
    float combat_timer = 0.0f; // Tiempo durante el que esta casilla no genera tropas.
    Owner protected_from = Owner::Neutral; // Bando contra el que protege la captura temporal.
    bool is_base = false; // Si es la base cuya captura decide la partida.
};
