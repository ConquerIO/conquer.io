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
    int fontSize = 0;
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

struct Panel {
    Rectangle bounds{};
    void draw(const UiTheme& theme, float scale) const;
};

struct Checkbox { //para las opciones del menu de personalizacion
    Rectangle bounds{};
    std::string label;
    bool checked = false;
    bool clicked = false;

    void update(Vector2 mouse);
    void draw(const UiTheme& theme, float scale, const Font* font = nullptr) const;
};

struct NumberStepper { // + tropas - | + nº bots -
    Rectangle bounds{}; // ancho total: 210 de diseño
    std::string value;
    std::size_t defaultValue = 0;
    std::size_t minValue = 0;
    std::size_t maxValue = 0;
    std::size_t step = 1;
    std::size_t maxDigits = 4;
    bool focused = false;

    bool update(Vector2 mouse); // true si se pulsa la caja
    void handleInput(float deltaTime);
    void draw(const UiTheme& theme, float scale, const Font* font = nullptr) const;
    void checkValue();
    std::size_t numericValue() const;
};
