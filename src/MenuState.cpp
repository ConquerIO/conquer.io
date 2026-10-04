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
    if (IsKeyPressed(KEY_RIGHT))
    {
        selected_color = (selected_color + 1) % NUM_COLORS;
    }
    if (IsKeyPressed(KEY_LEFT))
    {
        selected_color = (selected_color - 1 + NUM_COLORS) % NUM_COLORS;
    }

    // Transición al estado de juego
    if (IsKeyPressed(KEY_ENTER) && !player_name.empty())
    {
        this->state_machine->add_state(
            std::make_unique<MainGameState>(player_name, TERRITORY_COLORS[selected_color]),
            true
        );
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

    // Sección: selección de color
    const char* colorLabel = "Elige el color de tu territorio:";
    const int clSize = 28;
    const int clW = MeasureText(colorLabel, clSize);
    DrawText(colorLabel, (screenW - clW) / 2, 415, clSize, DARKGRAY);

    // Muestras de color
    const int swatchSize = 58;
    const int spacing = 24;
    const int totalW = NUM_COLORS * swatchSize + (NUM_COLORS - 1) * spacing;
    const int startX = (screenW - totalW) / 2;
    const int swatchY = 465;

    for (int i = 0; i < NUM_COLORS; i++)
    {
        const int sx = startX + i * (swatchSize + spacing);

        // Cuadro de color
        DrawRectangle(sx, swatchY, swatchSize, swatchSize, TERRITORY_COLORS[i]);

        if (i == selected_color)
        {
            // Borde grueso negro para el color seleccionado
            DrawRectangleLinesEx(
                Rectangle{
                    static_cast<float>(sx - 3),
                    static_cast<float>(swatchY - 3),
                    static_cast<float>(swatchSize + 6),
                    static_cast<float>(swatchSize + 6)
                },
                3.0f, BLACK
            );

            // Nombre del color bajo las muestras
            const int nameW = MeasureText(COLOR_NAMES[i], 24);
            DrawText(COLOR_NAMES[i], (screenW - nameW) / 2,
                     swatchY + swatchSize + 16, 24, DARKGRAY);
        }
        else
        {
            DrawRectangleLines(sx, swatchY, swatchSize, swatchSize, GRAY);
        }
    }

    // Instrucciones
    const char* arrowHint = "< >  Cambiar color";
    const int ahW = MeasureText(arrowHint, 22);
    DrawText(arrowHint, (screenW - ahW) / 2, 575, 22, GRAY);

    if (!player_name.empty())
    {
        const char* enterHint = "Pulsa ENTER para jugar";
        const int ehW = MeasureText(enterHint, 26);
        DrawText(enterHint, (screenW - ehW) / 2, 635, 26, DARKGREEN);
    }
    else
    {
        const char* waitHint = "Escribe tu nombre para continuar";
        const int whW = MeasureText(waitHint, 26);
        DrawText(waitHint, (screenW - whW) / 2, 635, 26, MAROON);
    }

    EndDrawing();
}
