#pragma once

enum class Owner
{
    Neutral,
    Player,
    Bot
};

struct TerritoryCell
{
    Owner owner = Owner::Neutral;
    float troops = 0.0f;
    float capture_protection = 0.0f;
    float combat_timer = 0.0f;
    Owner protected_from = Owner::Neutral;
    bool is_base = false;
};
