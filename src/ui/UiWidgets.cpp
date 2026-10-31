#include "UiWidgets.hpp"
#include <algorithm>
#include <cmath>

void Button::update(Vector2 mouse) {
    hovered = enabled && CheckCollisionPointRec(mouse, bounds); //si esta activo (nombre de jugador puesto) y raton por encima
    clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT); //si esta activo, raton encima y se pulsa clic izq
}

void Button::draw(const UiTheme& theme, float scale, const Font* font) const{
    const Font& f = font ? *font : GetFontDefault();
    const float fontSize = theme.buttonFontSize * scale;
    const Vector2 size = MeasureTextEx(f, label.c_str(), fontSize, 1.0f);
    const Vector2 pos{
        bounds.x + (bounds.width  - size.x) * 0.5f,
        bounds.y + (bounds.height - size.y) * 0.5f
    };

    switch (style) { //estilo del boton
        case ButtonStyle::Primary:
            DrawRectangleRec(bounds, enabled ? (hovered ? theme.primaryHover : theme.primary) : theme.disabled);
            DrawRectangleLinesEx(bounds, std::max(1.0f, scale), theme.border);
            DrawTextEx(f, label.c_str(), pos, fontSize, 1.0f, theme.text);
            break;

        case ButtonStyle::Secondary:
            DrawRectangleRec(bounds, hovered ? theme.secondaryHover : theme.secondary);
            DrawRectangleLinesEx(bounds, std::max(1.0f, scale), theme.border);
            DrawTextEx(f, label.c_str(), pos, fontSize, 1.0f, hovered ? theme.textHover : theme.text);
            break;

        case ButtonStyle::Text:
            DrawTextEx(f, label.c_str(), pos, fontSize, 1.0f, hovered ? theme.textHover : theme.textMuted);
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
        }
        key = GetCharPressed();
    }

    if (IsKeyDown(KEY_BACKSPACE) && !text.empty()) { //borrado del texto
        if (IsKeyPressed(KEY_BACKSPACE)) {
            text.pop_back();
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

