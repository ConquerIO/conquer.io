#pragma once
#include "raylib.h"

//para no declaralos en menustate o directamente al pintar
struct UiTheme {
    Color background{ 24, 29, 36, 255 };
    Color panel{ 14, 22, 32, 220 };
    Color border{ 70, 90, 105, 255 };
    Color text{ 242, 245, 247, 255 };
    Color textMuted{ 138, 153, 168, 255 };

    Color primary{ 46, 204, 113, 255 };
    Color primaryHover{ 66, 224, 139, 255 };
    Color danger{ 231, 76, 60, 255 };
    Color disabled{ 80, 90, 100, 255 };

    int buttonFontSize = 28;
    int smallFontSize = 20;
    float spacing = 24.0f;
};
