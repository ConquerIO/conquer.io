#pragma once

enum class Owner : unsigned char
{
    Water = 0,
    Land = 1, // Casilla sin dueño.
    Player,  // Controlada por el jugador.
    Bot      // Controlada por el bot.
};

struct TerritoryCell
{
    Owner owner = Owner::Land;
};
