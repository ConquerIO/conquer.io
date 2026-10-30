#pragma once
#include "raylib.h"

// funciona un traductor para convetir los pixeles de diseño a coordenadas reales de la ventana
struct UiLayout {
    static constexpr float DESIGN_WIDTH  = 1920.0f;
    static constexpr float DESIGN_HEIGHT = 1080.0f;

    float scale = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    void update(int screenWidth, int screenHeight);

    Rectangle rect(float x, float y, float width, float height) const;
    Vector2 point(float x, float y) const;
    Vector2 toDesign(Vector2 screenPoint) const;
};