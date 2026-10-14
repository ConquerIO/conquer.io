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
MenuState::MenuState(): player_name("player_"+std::to_string(GetRandomValue(100, 999))), selected_color(0), frame_counter(0), backspace_timer(0.0f), backspace_repeating(false){}

// Inicialización
void MenuState::init()
{
    // La ventana ya está inicializada en main.cpp.
    // Aquí se puede añadir inicialización específica del menú
    // (cargar fuentes, texturas, audio, etc.)

    scanAvailableMaps();
}

void MenuState::scanAvailableMaps(){

}

// Entrada
void MenuState::handleInput()
{
    if (show_customization){
        handleCustomizationInput();
    } else{
        handleMainMenuInput();
    }
}

void MenuState::handleMainMenuInput(){
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
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !player_name.empty())
    {
        const int screenW = GetScreenWidth();
        const int buttonW = 460;
        const int buttonH = 55;
        const int buttonX = (screenW - buttonW) / 2;
        const int buttonY = 405;

        Vector2 mouse = GetMousePosition();
        Rectangle buttonRect = {
            static_cast<float>(buttonX), static_cast<float>(buttonY),
            static_cast<float>(buttonW), static_cast<float>(buttonH)
        };

        if (CheckCollisionPointRec(mouse, buttonRect))
        {
            show_customization = true;
        }
    }
}

void MenuState::handleCustomizationInput(){

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

    renderMainMenu();

    if (show_customization)
    {
        renderCustomizationOverlay();
    }

    EndDrawing();
}

void MenuState::renderMainMenu(){
    const int screenW = GetScreenWidth();

    // Título 
    const char* title = "Conquer.io";
    const int titleSize = 100;
    const int titleW = MeasureText(title, titleSize);
    DrawText(title, (screenW - titleW) / 2, 140, titleSize, DARKGRAY);

    // Subtítulo
    const char* subtitle = "Conquista el mapa. Domina el territorio.";
    const int subSize = 24;
    const int subW = MeasureText(subtitle, subSize);
    DrawText(subtitle, (screenW - subW) / 2, 240, subSize, GRAY);

    // Caja de texto
    const int boxW = 460;
    const int boxH = 60;
    const int boxX = (screenW - boxW) / 2;
    const int boxY = 315;

    DrawRectangle(boxX, boxY, boxW, boxH, LIGHTGRAY);
    DrawRectangleLines(boxX, boxY, boxW, boxH, DARKGRAY);

    if (player_name.empty()){
        DrawText("Introduce un nombre", boxX + 15, boxY + 16, 26, GRAY);
    } else {
        // Texto introducido + cursor parpadeante
        std::string displayText = player_name;
        if ((frame_counter / 30) % 2 == 0)
        {
            displayText += "|";
        }
        DrawText(displayText.c_str(), boxX + 12, boxY + 15, 28, DARKGRAY);
    }

    // Selector de color
    const int colorBoxSize = boxH;  // mismo alto que la caja de texto (60px)
    const int colorGap = 6;        // separación entre caja de nombre y cuadrado de color
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

    // Botón JUGAR
    const int buttonW = 460;
    const int buttonH = 55;
    const int buttonX = (screenW - buttonW) / 2;
    const int buttonY = 380;

    Color colorFondoBoton;
    Color colorTextoBoton;

    if (!player_name.empty()) {
        colorFondoBoton = DARKGREEN;
        colorTextoBoton = RAYWHITE;
    } else {
        colorFondoBoton = Fade(DARKGREEN, 0.75f);
        colorTextoBoton = Fade(RAYWHITE, 0.7f);
    }

    DrawRectangle((float)buttonX, (float)buttonY, (float)buttonW, (float)buttonH, colorFondoBoton);
    const char* btnText = "PERSONALIZAR PARTIDA";
    const int btnTextW = MeasureText(btnText, 26);
    DrawText(btnText,
                buttonX + (buttonW - btnTextW) / 2,
                buttonY + (buttonH - 26) / 2,
                26, colorTextoBoton);

    DrawRectangleLines(buttonX, buttonY, buttonW, buttonH, DARKGRAY);
}


void MenuState::renderCustomizationOverlay(){


}