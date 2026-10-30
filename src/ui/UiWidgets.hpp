#pragma once
#include "raylib.h"
#include "UiTheme.hpp"
#include <string>

struct Button {
    Rectangle bounds{};
    std::string label;
    bool enabled = true;
    bool hovered = false;
    bool clicked = false;

    void update(Vector2 mouse);
    void draw(const UiTheme& theme, float scale, const Font* font = nullptr) const;
};
