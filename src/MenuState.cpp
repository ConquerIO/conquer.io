#include <MenuState.hpp>
#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <raylib.h>
#include <memory>

// Colores disponibles para el territorio
static const Color TERRITORY_COLORS[] = {
    RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE
};
static const char* COLOR_NAMES[] = {
    "Rojo", "Azul", "Verde", "Amarillo", "Morado", "Naranja"
};

// Constructor
MenuState::MenuState(): player_name(""), selected_color(0), frame_counter(0), backspace_timer(0.0f), backspace_repeating(false){}

// Inicialización
void MenuState::init()
{
    // La ventana ya está inicializada en main.cpp.
    // Aquí se puede añadir inicialización específica del menú
    // (cargar fuentes, texturas, audio, etc.)
}

// Entrada
void MenuState::handleInput()
{
    // Entrada de texto para el nombre del jugador
    int key = GetCharPressed();
    while (key > 0)
    {
        // Solo caracteres imprimibles ASCII (espacio...'~')
        if (key >= 32 && key <= 125
            && static_cast<int>(player_name.length()) < MAX_NAME_LENGTH)
        {
            player_name += static_cast<char>(key);
        }
        key = GetCharPressed();
    }

    // Borrado: mantener pulsado para borrar continuamente
    if (IsKeyDown(KEY_BACKSPACE) && !player_name.empty())
    {
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            // Primera pulsación: borrar inmediatamente
            player_name.pop_back();
            backspace_timer = 0.0f;
            backspace_repeating = false;
        }
        else
        {
            backspace_timer += GetFrameTime();
            if (!backspace_repeating && backspace_timer >= 0.4f)
            {
                // Pasó el retardo inicial → empezar a repetir
                backspace_repeating = true;
                backspace_timer = 0.0f;
                if (!player_name.empty()) player_name.pop_back();
            }
            else if (backspace_repeating && backspace_timer >= 0.05f)
            {
                // Repetición rápida cada 50ms
                backspace_timer = 0.0f;
                if (!player_name.empty()) player_name.pop_back();
            }
        }
    }
    else
    {
        backspace_timer = 0.0f;
        backspace_repeating = false;
    }

    // Selección de color con flechas izquierda/derecha
    if (IsKeyPressed(KEY_UP))
    {
        selected_color = (selected_color - 1 + NUM_COLORS) % NUM_COLORS;   
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        selected_color = (selected_color + 1) % NUM_COLORS;
    }
}

// Actualización
void MenuState::update(float deltaTime)
{
    frame_counter++;
}

// Renderizado
void MenuState::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    const int screenW = GetScreenWidth();

    // Título 
    const char* title    = "Conquer.io";
    const int titleSize  = 68;
    const int titleW     = MeasureText(title, titleSize);
    DrawText(title, (screenW - titleW) / 2, 80, titleSize, DARKGRAY);

    // Subtítulo
    const char* subtitle = "Conquista el mapa. Domina el territorio.";
    const int subSize    = 24;
    const int subW       = MeasureText(subtitle, subSize);
    DrawText(subtitle, (screenW - subW) / 2, 175, subSize, GRAY);

    // Sección: nombre del jugador
    const char* nameLabel = "Introduce tu nombre:";
    const int labelSize   = 28;
    const int labelW      = MeasureText(nameLabel, labelSize);
    DrawText(nameLabel, (screenW - labelW) / 2, 270, labelSize, DARKGRAY);

    // Caja de texto
    const int boxW = 460;
    const int boxH = 60;
    const int boxX = (screenW - boxW) / 2;
    const int boxY = 315;

    DrawRectangle(boxX, boxY, boxW, boxH, LIGHTGRAY);
    DrawRectangleLines(boxX, boxY, boxW, boxH, DARKGRAY);

    // Texto introducido + cursor parpadeante
    std::string displayText = player_name;
    if ((frame_counter / 30) % 2 == 0)
    {
        displayText += "|";
    }
    DrawText(displayText.c_str(), boxX + 12, boxY + 15, 28, DARKGRAY);

    // Selector de color
    const int colorBoxSize = boxH;  // mismo alto que la caja de texto (60px)
    const int colorGap = 16;        // separación entre caja de nombre y cuadrado de color
    const int colorBoxX = boxX + boxW + colorGap;
    const int colorBoxY = boxY;

    // cuadrado del color actual
    DrawRectangle(colorBoxX, colorBoxY, colorBoxSize, colorBoxSize, TERRITORY_COLORS[selected_color]);
    DrawRectangleLines(colorBoxX, colorBoxY, colorBoxSize, colorBoxSize, DARKGRAY);

    float centroX = colorBoxX + (colorBoxSize / 2.0f);
    float parteArriba = colorBoxY;
    float parteAbajo  = colorBoxY + colorBoxSize;

    // flechita arriba
    DrawTriangle(
        Vector2{ centroX, parteArriba - 14.0f },        // Punta
        Vector2{ centroX - 8.0f, parteArriba - 8.0f },  // Base izquierda
        Vector2{ centroX + 8.0f, parteArriba - 8.0f },  // Base derecha
        DARKGRAY
    );

    // flechita abajo
    DrawTriangle(
        Vector2{ centroX, parteAbajo + 14.0f },         // Punta
        Vector2{ centroX + 8.0f, parteAbajo + 8.0f },   // Base derecha (antihorario)
        Vector2{ centroX - 8.0f, parteAbajo + 8.0f },   // Base izquierda
        DARKGRAY
    );

    // nombre del color debajo de las flechas
    const int colorNameW = MeasureText(COLOR_NAMES[selected_color], 18);
    DrawText(COLOR_NAMES[selected_color],
            colorBoxX + (colorBoxSize - colorNameW) / 2,
            colorBoxY + colorBoxSize + 34,
            18, GRAY);

    if (!player_name.empty())
    {
        
    }
    else
    {
        const char* waitHint = "Escribe tu nombre para continuar";
        const int whW = MeasureText(waitHint, 26);
        DrawText(waitHint, (screenW - whW) / 2, 635, 26, MAROON);
    }

    EndDrawing();
}
