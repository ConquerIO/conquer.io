#pragma once
#include <string>

// Por ahora solo se guarda el valor elegido (NO afecta a su comportamient - por implementar)
enum class BotDifficulty{
    VERY_EASY,
    EASY,
    MEDIUM,
    HARD,
    EXTREME
};

// Tipo de mapa (generado con perlin o cargado desde un png)
enum class MapType{
    PROCEDURAL,
    MAP_FILE
};


struct GameConfig {
    MapType map_type = MapType::PROCEDURAL;
    std::string map_filename = ""; // Solo relevante si map_type == FROM_FILE
    BotDifficulty difficulty = BotDifficulty::MEDIUM;
    std::size_t initial_troops = 500; // Tropas iniciales del jugador
    std::size_t bot_troops = 350; // Tropas iniciales de cada bot
    std::size_t numero_bots = 10; //numero de bots en la partida
};
