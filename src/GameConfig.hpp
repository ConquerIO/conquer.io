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
    float initial_troops = 500.0f; // Tropas iniciales del jugador
    float bot_troops = 250.0f; // Tropas iniciales de cada bot
};
