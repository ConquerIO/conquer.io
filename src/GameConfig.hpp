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

struct BotTuning { //parametros para configurar el nivel de difucultad de los bots
    bool allow_alliances = true;
    float alliance_request_chance = 0.35f;
    float alliance_accept_chance = 0.55f;
    float alliance_request_interval_min = 20.0f;
    float alliance_request_interval_max = 40.0f;
    float betrayal_chance = 0.40f;

    float min_attack_ratio = 0.15f;
    float max_attack_ratio = 0.28f;
    float first_attack_delay_min = 5.5f;
    float first_attack_delay_max = 7.0f;
    float attack_delay_min = 12.0f;
    float attack_delay_max = 18.0f;
    float enemy_target_chance = 0.30f;

    bool coordinated_attacks = false;
    float coordination_chance = 0.0f;

    bool allow_bot_donations = false;
    float donation_interval_min = 30.0f;
    float donation_interval_max = 60.0f;
    float min_donation_ratio = 0.10f;
    float max_donation_ratio = 0.20f;
};

inline const BotTuning& tuningFor(BotDifficulty difficulty) //configuracion de los diferentes niveles de dificultad
{   
    //si se omites valores, se usa el de por defecto
    static const BotTuning very_easy = [] {
        BotTuning t;
        t.allow_alliances = false; //no puede hacer alianzas
        t.min_attack_ratio = 0.00f; //min y max del ratio de tropas con el que atacan
        t.max_attack_ratio = 1.00f;
        t.first_attack_delay_min = 4.0f; //min y max del ratio de delay del primer ataque de los bots
        t.first_attack_delay_max = 7.0f;
        t.attack_delay_min = 8.0f; //min y max del ratio de delay de ataque normal de los bots
        t.attack_delay_max = 14.0f;
        t.enemy_target_chance = 0.50f; //prob de atacar a un enemigo y no a tierra
        return t;
    }();

    static const BotTuning easy = [] {
        BotTuning t;
        t.allow_alliances = false;
        t.min_attack_ratio = 0.10f;
        t.max_attack_ratio = 0.50f;
        t.enemy_target_chance = 0.30f;
        return t;
    }();

    static const BotTuning medium = [] {
        BotTuning t; //permite alianzas
        t.alliance_request_chance = 0.20f; //probabiliadad de enviar una alizanza
        t.alliance_accept_chance = 0.45f; //probabiladad de aceptar una solicitud recibida
        t.alliance_request_interval_min = 25.0f; //tiempo min y max entre opurtunidades de pedir alianza (cada cuanto tiempo el bot vuelve a plantearse una alianza como min y max)
        t.alliance_request_interval_max = 45.0f;
        t.betrayal_chance = 0.30f; //porb de traiccionar a un aliado
        t.min_attack_ratio = 0.15f;
        t.max_attack_ratio = 0.30f;
        t.enemy_target_chance = 0.30f;
        return t;
    }();

    static const BotTuning hard = [] {
        BotTuning t;
        t.alliance_request_chance = 0.45f;
        t.alliance_accept_chance = 0.70f;
        t.betrayal_chance = 0.15f;
        t.min_attack_ratio = 0.15f;
        t.max_attack_ratio = 0.20f;
        t.enemy_target_chance = 0.40f;
        t.coordinated_attacks = true; //permite ataques coordinados
        t.coordination_chance = 0.60f; //porb de coordinarse cada vez que un bot va a atacar
        return t;
    }();

    static const BotTuning extreme = [] {
        BotTuning t = hard; //como dificil, pero
        t.allow_bot_donations = true; //permite donaciones entre bots
        return t;
    }();

    switch (difficulty) {
        case BotDifficulty::VERY_EASY: return very_easy;
        case BotDifficulty::EASY: return easy;
        case BotDifficulty::MEDIUM: return medium;
        case BotDifficulty::HARD: return hard;
        case BotDifficulty::EXTREME: return extreme;
    }
    return medium;
}

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
    std::size_t bot_troops = 450; // Tropas iniciales de cada bot
    std::size_t numero_bots = 40; //numero de bots en la partida
};
