#include <MenuState.hpp>
#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <raylib.h>
#include <memory>
#include <algorithm>

// Colores disponibles para el territorio
static const Color TERRITORY_COLORS[] = {
    RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE
};
static const char* COLOR_NAMES[] = {
    "Rojo", "Azul", "Verde", "Amarillo", "Morado", "Naranja"
};


void MenuState::scanAvailableMaps(){
    available_maps.clear();

    //utilizamos las funciones de la libreria de raylib para ver los mapas creados
    if (!DirectoryExists("maps")) {
        return;
    }

    FilePathList files = LoadDirectoryFiles("maps");

    for (unsigned int i=0; i<files.count; i++){
        if (IsFileExtension(files.paths[i], ".png")){
            available_maps.push_back(files.paths[i]);
        }
    }

    UnloadDirectoryFiles(files);
    std::sort(available_maps.begin(), available_maps.end());
}

// Dibuja una checkbox con texto al lado y devuelve el Rectangle de la checkbox (para detectar clics después).
Rectangle MenuState::drawCheckbox(int x, int y, const char* label, bool checked){
    const int boxSize = 18;

    if (checked){
        DrawRectangle(x, y, boxSize, boxSize, GREEN);
    } else{
        DrawRectangle(x, y, boxSize, boxSize, Color{ 50, 55, 65, 255 });
    }
    DrawRectangleLines(x, y, boxSize, boxSize, checked ? GREEN : GRAY);

    // el texto al lado
    DrawText(label, x + boxSize + 10, y + 1, 18, RAYWHITE);

    return Rectangle{ (float)x, (float)y, (float)(boxSize + 10 + MeasureText(label, 18)), (float)boxSize };
}


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
    Rectangle buttonRect = {(float)buttonX, (float)buttonY, (float)buttonW, (float)buttonH};
    Color colorFondoBoton;
    Color colorTextoBoton;

    if (player_name.empty()) {
        colorFondoBoton = Fade(DARKGREEN, 0.75f);
        colorTextoBoton = Fade(RAYWHITE, 0.7f);
    } else if (CheckCollisionPointRec(GetMousePosition(), buttonRect)){
        colorFondoBoton = Color{ 0, 135, 0, 255 };                                                                                                                                                                             
        colorTextoBoton = RAYWHITE;
    } else {
        colorFondoBoton = DARKGREEN;
        colorTextoBoton = RAYWHITE;
    }

    DrawRectangleRec(buttonRect, colorFondoBoton);
    const char* btnText = "PERSONALIZAR PARTIDA";
    const int btnTextW = MeasureText(btnText, 26);
    DrawText(btnText,
                buttonX + (buttonW - btnTextW) / 2,
                buttonY + (buttonH - 26) / 2,
                26, colorTextoBoton);

    DrawRectangleLines(buttonX, buttonY, buttonW, buttonH, DARKGRAY);
}


void MenuState::renderCustomizationOverlay(){
    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    // fondo semi transparente
    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.65f));

    //ventana principal
    const int panelW = 1000;
    const int panelH = 700;
    const int panelX = (screenW - panelW) / 2;
    const int panelY = (screenH - panelH) / 2;

    DrawRectangle(panelX, panelY, panelW, panelH, Color{ 30, 34, 42, 255 });
    DrawRectangleLines(panelX, panelY, panelW, panelH, DARKGRAY);

    //titulo
    const char* title = "PERSONALIZAR PARTIDA";
    DrawText(title, panelX + (panelW - MeasureText(title, 28)) / 2, panelY + 20, 28, RAYWHITE);

    // ancho de cada una de las 3 columnas
    const int colW = 270;
    const int colGap = 40;
    const int startY = panelY + 75;
    const int col1X = panelX + 60;
    const int col2X = col1X + colW + colGap;
    const int col3X = col2X + colW + colGap;

    // COLUMNA 1 - Eleccion de mapa
    DrawText("Mapa", col1X, startY, 22, LIGHTGRAY);

    bool isProcedural = (config.map_type == MapType::PROCEDURAL);
    bool isFile = (config.map_type == MapType::MAP_FILE);

    drawCheckbox(col1X, startY + 35, "Procedural", isProcedural);
    drawCheckbox(col1X, startY + 65, "Desde archivo", isFile);

    if (!available_maps.empty()){
        DrawText("Archivos disponibles:", col1X, startY + 110, 16, GRAY);

        for (size_t i=0; i<available_maps.size() && i<6; i++){
            int itemY = startY + 135 + static_cast<int>(i) * 30;
            bool isSelected = (isFile && selected_map_index == static_cast<int>(i));

            // Extraer solo el nombre del archivo (sin "maps/")
            const char* nameOnly = available_maps[i].c_str();
            size_t lastSlash = available_maps[i].rfind('/');
            if (lastSlash != std::string::npos){
                nameOnly += lastSlash + 1;
            }

            drawCheckbox(col1X + 10, itemY, nameOnly, isSelected);
        }
    } else{
        DrawText("No hay mapas en maps/", col1X + 10, startY + 110, 16, GRAY);
    }

    // Columna 2 - Dificultad de los Bots
    DrawText("Dificultad", col2X, startY, 22, LIGHTGRAY);

    const char* DIFF_NAMES[] = { "Muy facil", "Facil", "Medio", "Dificil", "Extremo" };
    for (int i = 0; i < 5; i++){
        int y = startY + 35 + i * 35;
        bool isSelected = (static_cast<int>(config.difficulty) == i);
        drawCheckbox(col2X, y, DIFF_NAMES[i], isSelected);
    }


    // Columna 3 - Tropas iniciales
    DrawText("Tropas", col3X, startY, 22, LIGHTGRAY);

    // tropas del jugador
    DrawText("Jugador (100 - 2000):", col3X, startY + 35, 16, GRAY);

    int playerRowY = startY + 60;

    // [-]
    DrawRectangle(col3X, playerRowY, 45, 40, Color{ 55, 60, 70, 255 });
    DrawRectangleLines(col3X, playerRowY, 45, 40, GRAY);
    DrawText("-", col3X + 16, playerRowY + 6, 28, RAYWHITE);

    // Caja de texto del número
    bool focusPlayer = (active_troop_box == 1);
    DrawRectangle(col3X + 55, playerRowY, 100, 40, focusPlayer ? Color{ 20, 25, 30, 255 } : Color{ 40, 45, 55, 255 });
    DrawRectangleLines(col3X + 55, playerRowY, 100, 40, focusPlayer ? GREEN : GRAY);
    int ptW = MeasureText(player_troops_str.c_str(), 22);
    DrawText(player_troops_str.c_str(), col3X + 55 + (100 - ptW) / 2, playerRowY + 10, 22, RAYWHITE);

    // [+]
    DrawRectangle(col3X + 165, playerRowY, 45, 40, Color{ 55, 60, 70, 255 });
    DrawRectangleLines(col3X + 165, playerRowY, 45, 40, GRAY);
    DrawText("+", col3X + 165 + 14, playerRowY + 6, 28, RAYWHITE);

    // tropas de los bots
    DrawText("Bots (50 - 1000):", col3X, startY + 125, 16, GRAY);

    int botRowY = startY + 150;

    // [-]
    DrawRectangle(col3X, botRowY, 45, 40, Color{ 55, 60, 70, 255 });
    DrawRectangleLines(col3X, botRowY, 45, 40, GRAY);
    DrawText("-", col3X + 16, botRowY + 6, 28, RAYWHITE);

    // Caja de texto del número
    bool focusBot = (active_troop_box == 2);
    DrawRectangle(col3X + 55, botRowY, 100, 40, focusBot ? Color{ 20, 25, 30, 255 } : Color{ 40, 45, 55, 255 });
    DrawRectangleLines(col3X + 55, botRowY, 100, 40, focusBot ? GREEN : GRAY);
    int btW = MeasureText(bot_troops_str.c_str(), 22);
    DrawText(bot_troops_str.c_str(), col3X + 55 + (100 - btW) / 2, botRowY + 10, 22, RAYWHITE);

    // [+]
    DrawRectangle(col3X + 165, botRowY, 45, 40, Color{ 55, 60, 70, 255 });
    DrawRectangleLines(col3X + 165, botRowY, 45, 40, GRAY);
    DrawText("+", col3X + 165 + 14, botRowY + 6, 28, RAYWHITE);

    // Botones inferiores
    const int btnW = 180;
    const int btnH = 50;
    const int btnGap = 30;
    const int totalBtnW = btnW * 2 + btnGap;
    const int btnStartX = panelX + (panelW - totalBtnW) / 2;
    const int btnY = panelY + panelH - 75;

    // botón ESC (gris)
    Rectangle escRect = { (float)btnStartX, (float)btnY, (float)btnW, (float)btnH };
    bool hoverEsc = CheckCollisionPointRec(GetMousePosition(), escRect);
    DrawRectangleRec(escRect, hoverEsc ? Color{ 80, 85, 95, 255 } : Color{ 60, 65, 75, 255 });
    DrawRectangleLines(btnStartX, btnY, btnW, btnH, GRAY);
    const char* escText = "ESC";
    DrawText(escText, btnStartX + (btnW - MeasureText(escText, 22)) / 2, btnY + 14, 22, RAYWHITE);

    // botón JUGAR (verde)
    int jugarX = btnStartX + btnW + btnGap;
    Rectangle jugarRect = { (float)jugarX, (float)btnY, (float)btnW, (float)btnH };
    bool hoverJugar = CheckCollisionPointRec(GetMousePosition(), jugarRect);
    DrawRectangleRec(jugarRect, hoverJugar ? Color{ 0, 160, 0, 255 } : DARKGREEN);
    DrawRectangleLines(jugarX, btnY, btnW, btnH, RAYWHITE);
    const char* jugarText = "JUGAR";
    DrawText(jugarText, jugarX + (btnW - MeasureText(jugarText, 22)) / 2, btnY + 14, 22, RAYWHITE);
}