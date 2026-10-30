#include "UiWidgets.hpp"
#include <algorithm>

void Button::update(Vector2 mouse) {
    hovered = enabled && CheckCollisionPointRec(mouse, bounds);
    clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void Button::draw(const UiTheme& theme, float scale, const Font* font) const{
    Color fill = !enabled ? theme.disabled : (hovered ? theme.primaryHover : theme.primary);

    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, std::max(1.0f, scale), theme.border);

    const Font& f = font ? *font : GetFontDefault();
    const float fontSize = theme.buttonFontSize * scale;
    const Vector2 size = MeasureTextEx(f, label.c_str(), fontSize, 1.0f);

    const Vector2 pos{
        bounds.x + (bounds.width  - size.x) * 0.5f,
        bounds.y + (bounds.height - size.y) * 0.5f
    };

    DrawTextEx(f, label.c_str(), pos, fontSize, 1.0f, theme.text);
}
