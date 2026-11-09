#include "UiWidgets.hpp"
#include "system/AudioManager.hpp"
#include <algorithm>
#include <cmath>

void Button::update(Vector2 mouse) {
    const bool wasHovered = hovered;
    hovered = enabled && CheckCollisionPointRec(mouse, bounds); //si esta activo (nombre de jugador puesto) y raton por encima
    clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT); //si esta activo, raton encima y se pulsa clic izq

    if (clicked) {
        AudioManager::play(Sfx::UiClick);
    } else if (hovered && !wasHovered) { //solo suena al entrar en el boton, no cada frame
        AudioManager::play(Sfx::UiHover);
    }
}

void Button::draw(const UiTheme& theme, float scale, const Font* font) const{
    const Font& f = font ? *font : GetFontDefault();
    const float textSize = (fontSize > 0 ? static_cast<float>(fontSize) : static_cast<float>(theme.buttonFontSize)) * scale;
    const Vector2 size = MeasureTextEx(f, label.c_str(), textSize, 1.0f);
    const Vector2 pos{
        bounds.x + (bounds.width  - size.x) * 0.5f,
        bounds.y + (bounds.height - size.y) * 0.5f
    };

    switch (style) { //estilo del boton
        case ButtonStyle::Primary:
            DrawRectangleRec(bounds, enabled ? (hovered ? theme.primaryHover : theme.primary) : theme.disabled);
            DrawRectangleLinesEx(bounds, std::max(1.0f, scale), theme.border);
            DrawTextEx(f, label.c_str(), pos, textSize, 1.0f, theme.text);
            break;

        case ButtonStyle::Secondary:
            DrawRectangleRec(bounds, hovered ? theme.secondaryHover : theme.secondary);
            DrawRectangleLinesEx(bounds, std::max(1.0f, scale), theme.border);
            DrawTextEx(f, label.c_str(), pos, textSize, 1.0f, hovered ? theme.textHover : theme.text);
            break;

        case ButtonStyle::Text:
            DrawTextEx(f, label.c_str(), pos, textSize, 1.0f, hovered ? theme.textHover : theme.textMuted);
            break;
    }
}

void TextField::handleInput(float deltaTime){
    if (!focused) {
        backspaceTimer = 0.0f;
        backspaceRepeating = false;
        return;
    }

    int key = GetCharPressed(); //entrada de texto para el nombre del jugador
    while (key > 0) {
        if (key >= 32 && key <= 125 && text.size() < maxLength) {
            text.push_back(static_cast<char>(key));
            AudioManager::play(Sfx::UiType);
        }
        key = GetCharPressed();
    }

    if (IsKeyDown(KEY_BACKSPACE) && !text.empty()) { //borrado del texto
        if (IsKeyPressed(KEY_BACKSPACE)) {
            text.pop_back();
            AudioManager::play(Sfx::UiType);
            backspaceTimer = 0.0f;
            backspaceRepeating = false;
        } else {
            backspaceTimer += deltaTime;
            if (!backspaceRepeating && backspaceTimer >= 0.4f) {
                backspaceRepeating = true;
                backspaceTimer = 0.0f;
                if (!text.empty()) text.pop_back();
            } else if (backspaceRepeating && backspaceTimer >= 0.05f) {
                backspaceTimer = 0.0f;
                if (!text.empty()) text.pop_back();
            }
        }
    } else {
        backspaceTimer = 0.0f;
        backspaceRepeating = false;
    }

    cursorTimer += deltaTime;
}

void TextField::draw(const UiTheme& theme, float scale, const Font* font) const{
    const Font& f = font ? *font : GetFontDefault();
    const float fontSize = theme.fieldFontSize * scale;

    DrawRectangleRec(bounds, theme.panel);
    DrawRectangleLinesEx(bounds, focused ? std::max(2.0f, scale) : std::max(1.0f, scale), focused ? theme.borderFocus : theme.border);

    const char* shown = text.empty() ? placeholder.c_str() : text.c_str();
    const Color color = text.empty() ? theme.textMuted : theme.text;
    const Vector2 size = MeasureTextEx(f, shown, fontSize, 1.0f);
    const Vector2 pos{
        bounds.x + 15.0f * scale,
        bounds.y + (bounds.height - size.y) * 0.5f
    };

    DrawTextEx(f, shown, pos, fontSize, 1.0f, color);

    if (focused && std::fmod(cursorTimer, 1.0f) < 0.5f) { //para el cursor parpadenate en la caja de texto
        const float cursorX = pos.x + (text.empty() ? 0.0f : size.x) + 2.0f * scale;
        DrawRectangle(static_cast<int>(cursorX), static_cast<int>(pos.y), std::max(1, static_cast<int>(scale)), static_cast<int>(fontSize), theme.text);
    }
}

void Panel::draw(const UiTheme& theme, float scale) const {
    DrawRectangleRec(bounds, theme.panel);
    DrawRectangleLinesEx(bounds, std::max(1.0f, scale), theme.border);
}

void Checkbox::update(Vector2 mouse) {
    clicked = CheckCollisionPointRec(mouse, bounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (clicked) AudioManager::play(Sfx::UiChange);
}

void Checkbox::draw(const UiTheme& theme, float scale, const Font* font) const {
    const float box = 18.0f * scale;
    const float boxY = bounds.y + (bounds.height - box) * 0.5f;

    DrawRectangle(bounds.x, boxY, box, box, checked ? theme.primary : theme.secondary); //el rect ocupa el cuadrado y el nombre
    DrawRectangleLinesEx({ bounds.x, boxY, box, box }, std::max(1.0f, scale), checked ? theme.primary : theme.border);

    const Font& f = font ? *font : GetFontDefault();
    const float fs = theme.checkboxFontSize * scale;
    const Vector2 ts = MeasureTextEx(f, label.c_str(), fs, 1.0f);

    DrawTextEx(f, label.c_str(), { bounds.x + box + 10.0f * scale, bounds.y + (bounds.height - ts.y) * 0.5f }, fs, 1.0f, theme.text);
}


//funciones auxiliares para los botones de personalizacion de -, + y el campo
static Rectangle stepperMinus(const Rectangle& b) {
    const float s = b.width / 210.0f;
    return { b.x, b.y, 45.0f * s, b.height };
}

static Rectangle stepperField(const Rectangle& b) {
    const float s = b.width / 210.0f;
    return { b.x + 55.0f * s, b.y, 100.0f * s, b.height };
}

static Rectangle stepperPlus(const Rectangle& b) {
    const float s = b.width / 210.0f;
    return { b.x + 165.0f * s, b.y, 45.0f * s, b.height };
}

bool NumberStepper::update(Vector2 mouse){
    const bool minus = CheckCollisionPointRec(mouse, stepperMinus(bounds)) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    const bool plus = CheckCollisionPointRec(mouse, stepperPlus(bounds)) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (minus || plus) AudioManager::play(Sfx::UiChange);

    if (minus) {
        std::size_t v = numericValue();
        v = (v <= minValue + step) ? minValue : v - step;
        value = std::to_string(v);
    }

    if (plus) {
        std::size_t v = numericValue();
        v = std::min(maxValue, v + step);
        value = std::to_string(v);
    }

    return CheckCollisionPointRec(mouse, stepperField(bounds)) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void NumberStepper::handleInput(float deltaTime){
    (void)deltaTime;
    if (!focused) return;

    int key = GetCharPressed();
    while (key > 0) {
        if (key >= '0' && key <= '9' && value.size() < maxDigits) {
            value.push_back(static_cast<char>(key));
            AudioManager::play(Sfx::UiType);
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !value.empty()) {
        value.pop_back();
        AudioManager::play(Sfx::UiType);
    }
}

void NumberStepper::draw(const UiTheme& theme, float scale, const Font* font) const {
    const Font& f = font ? *font : GetFontDefault();
    const float fs = theme.valueFontSize * scale;

    const Rectangle minus = stepperMinus(bounds); //boton menos
    const Rectangle field = stepperField(bounds); //campo del numero
    const Rectangle plus = stepperPlus(bounds); //boton mas

    DrawRectangleRec(minus, theme.secondary);
    DrawRectangleLinesEx(minus, std::max(1.0f, scale), theme.border);

    DrawRectangleRec(field, focused ? theme.background : theme.secondary);
    DrawRectangleLinesEx(field, focused ? std::max(2.0f, scale) : std::max(1.0f, scale), focused ? theme.primary : theme.border);

    DrawRectangleRec(plus, theme.secondary);
    DrawRectangleLinesEx(plus, std::max(1.0f, scale), theme.border);

    const Vector2 minusSize = MeasureTextEx(f, "-", fs, 1.0f);
    const Vector2 plusSize = MeasureTextEx(f, "+", fs, 1.0f);
    const Vector2 valueSize = MeasureTextEx(f, value.c_str(), fs, 1.0f);

    DrawTextEx(f, "-", { minus.x + (minus.width - minusSize.x) * 0.5f, minus.y + (minus.height - minusSize.y) * 0.5f }, fs, 1.0f, theme.text);

    DrawTextEx(f, value.c_str(), { field.x + (field.width - valueSize.x) * 0.5f, field.y + (field.height - valueSize.y) * 0.5f }, fs, 1.0f, theme.text);

    DrawTextEx(f, "+", { plus.x + (plus.width - plusSize.x) * 0.5f, plus.y + (plus.height - plusSize.y) * 0.5f }, fs, 1.0f, theme.text);
}


//funciones auxiliares para cuando se deja la caja de texto de nº tropas o bots vacio o comprobar que no haya pasado los limites
std::size_t NumberStepper::numericValue() const{
    if (value.empty()) return defaultValue;
    try {
        return std::stoul(value);
    } catch (...) {
        return defaultValue;
    }
}

void NumberStepper::checkValue(){
    if (value.empty()) {
        value = std::to_string(defaultValue);
        return;
    }

    std::size_t v = numericValue();
    if (v < minValue) v = minValue;
    if (v > maxValue) v = maxValue;
    value = std::to_string(v);
}