#pragma once
#include "raylib.h"

//para no declaralos en menustate o directamente al pintar
struct UiTheme {
    Color background{ 24, 29, 36, 255 };
    Color panel{ 14, 22, 32, 220 };
    Color border{ 70, 90, 105, 255 };
    Color borderFocus{ 52, 67, 80, 255 };
    Color text{ 242, 245, 247, 255 };
    Color textMuted{ 138, 153, 168, 255 };

    Color primary{ 0, 117, 44, 255 };
    Color primaryHover{ 0, 150, 58, 255 };
    Color danger{ 231, 76, 60, 255 };
    Color disabled{ 80, 90, 100, 255 };

    Color secondary{ 30, 38, 48, 255 };
    Color secondaryHover{ 42, 52, 64, 255 };
    Color textHover{ 255, 255, 255, 255 };
    
    int fieldFontSize = 16;
    int buttonFontSize = 28;
    int smallFontSize = 10;
    float spacing = 24.0f;
};
