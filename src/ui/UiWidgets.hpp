#pragma once
#include "raylib.h"
#include "UiTheme.hpp"
#include <string>

enum class ButtonStyle {
    Primary,
    Secondary,
    Text,
};
struct Button {
    Rectangle bounds{};
    std::string label;
    ButtonStyle style = ButtonStyle::Primary;
    bool enabled = true;
    bool hovered = false;
    bool clicked = false;

    void update(Vector2 mouse);
    void draw(const UiTheme& theme, float scale, const Font* font = nullptr) const;
};

struct TextField { //logica de la caja de texto para el nombre del jugador
    Rectangle bounds{};
    std::string text;
    std::string placeholder;
    std::size_t maxLength = 15;
    bool focused = false;

    float cursorTimer = 0.0f;
    float backspaceTimer = 0.0f;
    bool backspaceRepeating = false;

    void handleInput(float deltaTime);
    void draw(const UiTheme& theme, float scale, const Font* font = nullptr) const;
};
