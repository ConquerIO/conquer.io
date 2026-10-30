#include "UiLayout.hpp"
#include <algorithm>

void UiLayout::update(int screenWidth, int screenHeight){
    scale = std::min(screenWidth / DESIGN_WIDTH, screenHeight / DESIGN_HEIGHT); //factor de escalado para reducir/aumentar el tamaño

    if (scale <= 0.0f) scale = 1.0f;

    // ajuste por si no tiene la misma proporcion la ventana real con 1920x1080
    offsetX = (screenWidth  - DESIGN_WIDTH  * scale) * 0.5f;
    offsetY = (screenHeight - DESIGN_HEIGHT * scale) * 0.5f;
}

//convierte un rectangulo de diseño a pantalla real
Rectangle UiLayout::rect(float x, float y, float width, float height) const{
    return Rectangle{offsetX + x * scale, offsetY + y * scale, width * scale, height * scale};
}

//convierte un punto de diseño a pantalla real (para textos, iconos ...)
Vector2 UiLayout::point(float x, float y) const{
    return Vector2{ offsetX + x * scale, offsetY + y * scale };
}

//connvierte un punto real a la pantalla de diseño
Vector2 UiLayout::toDesign(Vector2 screenPoint) const{
    return Vector2{(screenPoint.x - offsetX) / scale, (screenPoint.y - offsetY) / scale};
}
