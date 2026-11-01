#include <MenuState.hpp>
#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <raylib.h>
#include <memory>
#include <algorithm>
#include "system/ResourceManager.hpp"
#include <vector>
#include <cmath>

// para cargar siempre el mismo logo y fuentes
static const char* const LOGO_IMAGE_KEY = "menu.logo";
static const char* const MENU_FONT_REGULAR_KEY = "menu.font.regular";
static const char* const MENU_FONT_BOLD_KEY = "menu.font.bold";
static const char* const MENU_FONT_P_REGULAR_KEY = "menu.font.p.regular";
static const char* const MENU_FONT_PRESS_REGULAR_KEY = "menu.font.press.regular";

// colores disponibles para el territorio
static const Color TERRITORY_COLORS[] = {
    RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE
};

// funcion auxiliar que añade robustez si se ejecuta el juego en otro directorio
std::string MenuState::assetPath(const std::string& relativePath) const{
    std::string base = GetApplicationDirectory();
    if (!base.empty() && base.back() != '/') {
        base += '/';
    }
    return base + relativePath;
}

void MenuState::scanAvailableMaps(){
    available_maps.clear();
    
    const std::string mapsDir = assetPath("maps");
    //utilizamos las funciones de la libreria de raylib para ver los mapas creados
    if (!DirectoryExists(mapsDir.c_str())) {
        return;
    }

    FilePathList files = LoadDirectoryFiles(mapsDir.c_str());

    for (unsigned int i=0; i<files.count; i++){
        if (IsFileExtension(files.paths[i], ".png")){
            available_maps.push_back(files.paths[i]);
        }
    }

    UnloadDirectoryFiles(files);
    std::sort(available_maps.begin(), available_maps.end());
}

// cargar la imagen del logo, y cargar la textura en el ResourceManager
bool MenuState::loadLogo(){
    if (ResourceManager::getTexture(LOGO_IMAGE_KEY) != nullptr) return true;

    if (ResourceManager::getInstance() == nullptr) { //no se ha inicializado el resource manager
        return false;
    }

    const std::string path = assetPath("assets/images/logotipo-conquer.png"); //ruta del logotipo
    if (!FileExists(path.c_str())) { //si no existe el logo
        return false;
    }

    //cargamos la textura para poder pintarla a partir de la imagen y la guardamos en el resourcemanager
    Image image = LoadImage(path.c_str());
    if (image.data == nullptr) {
        return false;
    }
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    if (texture.id == 0) {
        return false;
    }

    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    ResourceManager::loadTexture(LOGO_IMAGE_KEY, texture);
    return true;
}

// funcion auxiliar para cargar las fuentes del menu y guardarla en el resource manager si no existe
bool MenuState::loadFontResource(const char* key, const char* relativePath){
    if (ResourceManager::getFont(key) != nullptr) return true;

    const std::string path = assetPath(relativePath);
    if (!FileExists(path.c_str())) return false;

    std::vector<int> codepoints;
    for (int c = 32; c <= 126; ++c) codepoints.push_back(c);
    for (int c : {0x00A1, 0x00A9, 0x00BF, 0x00C1, 0x00C9, 0x00CD, 0x00D3,
                  0x00DA, 0x00DC, 0x00D1, 0x00E1, 0x00E9, 0x00ED, 0x00F3,
                  0x00FA, 0x00FC, 0x00F1, 0x2191, 0x2193}) {
        codepoints.push_back(c);
    }

    Font font = LoadFontEx(path.c_str(), 64, codepoints.data(), static_cast<int>(codepoints.size()));

    if (font.texture.id == 0) return false;

    SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
    ResourceManager::loadFont(key, font);
    return true;
}
bool MenuState::loadMenuFonts(){
    const bool regular = loadFontResource(MENU_FONT_REGULAR_KEY, "assets/fonts/Silkscreen/Silkscreen-Regular.ttf");
    const bool bold = loadFontResource(MENU_FONT_BOLD_KEY, "assets/fonts/Silkscreen/Silkscreen-Bold.ttf");
    const bool regular_p = loadFontResource(MENU_FONT_P_REGULAR_KEY, "assets/fonts/Pixelify_Sans/static/PixelifySans-SemiBold.ttf");
    const bool regular_press2p = loadFontResource(MENU_FONT_PRESS_REGULAR_KEY, "assets/fonts/Press_Start_2P/PressStart2P-Regular.ttf");
    return regular && bold && regular_p && regular_press2p;
}

void MenuState::updateLayout(){
    ui_layout.update(GetScreenWidth(), GetScreenHeight()); //calcula el factor de escalado y offsets para

    //calcula donde se colocan los rectangulos en la pantalla real del menu
    menu_layout.logo = ui_layout.rect(612.0f,  80.0f, 696.0f, 220.0f); //logo *2
    menu_layout.nameField = ui_layout.rect(690.0f, 436.0f, 460.0f,  64.0f);
    menu_layout.colorSelector = ui_layout.rect(1166.0f, 436.0f,  64.0f, 64.0f);
    menu_layout.colorHint = ui_layout.rect(1246.0f, 456.0f, 260.0f, 24.0f);
    menu_layout.primaryButton = ui_layout.rect(700.0f, 510.0f, 520.0f,  76.0f);
    menu_layout.settingsButton = ui_layout.rect(860.0f, 630.0f, 200.0f,  56.0f);
    menu_layout.exitButton = ui_layout.rect(860.0f, 710.0f, 200.0f,  56.0f);
    menu_layout.credits = ui_layout.rect(24.0f, 1046.0f, 500.0f, 30.0f);
    menu_layout.version = ui_layout.rect(1396.0f, 1046.0f, 500.0f, 30.0f);

    //le pasa los atributos a los elementos del menu
    name_field.bounds = menu_layout.nameField;
    create_button.bounds = menu_layout.primaryButton;
    settings_button.bounds = menu_layout.settingsButton;
    exit_button.bounds = menu_layout.exitButton;
    settings_back_button.bounds = ui_layout.rect(860.0f, 620.0f, 200.0f, 56.0f);
}

void MenuState::updateCustomizationLayout() {
    ui_layout.update(GetScreenWidth(), GetScreenHeight());

    const float panelX = 400.0f;
    const float panelY = 190.0f;
    const float panelW = 1120.0f;
    const float panelH = 700.0f;

    const float startY = panelY + 80.0f;
    const float contentMargin = 120.0f;
    const float col1X = panelX + contentMargin;
    const float col2X = col1X + 270.0f + 40.0f;
    const float col3X = col2X + 270.0f + 40.0f;

    customization_layout.panel = ui_layout.rect(panelX, panelY, panelW, panelH); //panel donde se dibujara la pantalla de personalizacion
    customization_layout.title = ui_layout.rect(panelX, panelY + 20.0f, panelW, 40.0f);

    // Columna 1: mapa
    customization_layout.mapLabel = ui_layout.rect(col1X, startY, 270.0f, 24.0f);
    customization_layout.mapProcedural = ui_layout.rect(col1X, startY + 35.0f, 250.0f, 24.0f);
    customization_layout.mapFile = ui_layout.rect(col1X, startY + 65.0f, 250.0f, 24.0f);
    customization_layout.mapsLabel = ui_layout.rect(col1X, startY + 110.0f, 250.0f, 24.0f);
    for (int i = 0; i < 6; ++i) {
        customization_layout.mapFiles[i] = ui_layout.rect(col1X + 10.0f, startY + 135.0f + i * 30.0f, 250.0f, 24.0f);
    }

    // Columna 2: dificultad
    customization_layout.difficultyLabel = ui_layout.rect(col2X, startY, 270.0f, 24.0f);
    for (int i = 0; i < 5; ++i) {
        customization_layout.difficultyRows[i] = ui_layout.rect(col2X, startY + 35.0f + i * 35.0f, 250.0f, 24.0f);
    }

    // Columna 3: tropas
    customization_layout.troopsLabel = ui_layout.rect(col3X, startY, 260.0f, 24.0f);

    customization_layout.playerLabel = ui_layout.rect(col3X, startY + 35.0f, 260.0f, 20.0f);
    customization_layout.playerStepper = ui_layout.rect(col3X, startY + 60.0f, 260.0f, 40.0f);

    customization_layout.botsLabel = ui_layout.rect(col3X, startY + 125.0f, 260.0f, 20.0f);
    customization_layout.botsStepper = ui_layout.rect(col3X, startY + 150.0f, 260.0f, 40.0f);

    customization_layout.numBotsLabel = ui_layout.rect(col3X, startY + 215.0f, 260.0f, 20.0f);
    customization_layout.numBotsStepper = ui_layout.rect(col3X, startY + 240.0f, 260.0f, 40.0f);

    // Botones inferiores
    const float buttonsY = panelY + panelH - 75.0f;
    const float backX = panelX + (panelW - (180.0f * 2.0f + 30.0f)) * 0.5f;

    customization_layout.backButton = ui_layout.rect(backX, buttonsY, 180.0f, 50.0f);
    customization_layout.playButton = ui_layout.rect(backX + 180.0f + 30.0f, buttonsY, 180.0f, 50.0f);

    // Asignar bounds a los widgets
    customization_panel.bounds = customization_layout.panel;

    map_procedural_checkbox.bounds = customization_layout.mapProcedural;
    map_file_checkbox.bounds = customization_layout.mapFile;

    for (int i = 0; i < 5; ++i) {
        difficulty_checkboxes[i].bounds = customization_layout.difficultyRows[i];
        difficulty_checkboxes[i].checked = (static_cast<int>(config.difficulty) == i);
    }

    player_troops_stepper.bounds = customization_layout.playerStepper;
    bots_troops_stepper.bounds = customization_layout.botsStepper;
    num_bots_stepper.bounds = customization_layout.numBotsStepper;

    customization_back_button.bounds = customization_layout.backButton;
    customization_play_button.bounds = customization_layout.playButton;

    // Estados de mapa
    map_procedural_checkbox.checked = (config.map_type == MapType::PROCEDURAL);
    map_file_checkbox.checked = (config.map_type == MapType::MAP_FILE);

    // Lista de mapas
    for (int i = 0; i < 6; ++i) {
        if (i < static_cast<int>(available_maps.size())) {
            map_file_checkboxes[i].bounds = customization_layout.mapFiles[i];

            std::string name = available_maps[i];
            const std::size_t slash = name.find_last_of("/\\");
            if (slash != std::string::npos) {
                name = name.substr(slash + 1);
            }

            map_file_checkboxes[i].label = name;
            map_file_checkboxes[i].checked = (config.map_type == MapType::MAP_FILE && selected_map_index == i);
        } else {
            map_file_checkboxes[i].bounds = {};
            map_file_checkboxes[i].label.clear();
            map_file_checkboxes[i].checked = false;
        }
    }
}


// Constructor
MenuState::MenuState(): selected_color(0){
    name_field.text = "player_" + std::to_string(GetRandomValue(100, 999));
    name_field.placeholder = "Introduce un nombre";
    name_field.maxLength = 15;
}

// Inicialización
void MenuState::init()
{
    // (cargar fuentes, texturas, audio, etc.)
    loadLogo();
    loadMenuFonts();
    scanAvailableMaps();

    create_button.label = "CREAR PARTIDA";
    create_button.style = ButtonStyle::Primary;

    settings_button.label = "AJUSTES";
    settings_button.style = ButtonStyle::Text;

    exit_button.label = "SALIR";
    exit_button.style = ButtonStyle::Text;

    settings_back_button.label = "VOLVER";
    settings_back_button.style = ButtonStyle::Secondary;

    //inicializacion de los elementos de la pantalla de personalizacion
    map_procedural_checkbox.label = "Procedural";
    map_file_checkbox.label = "Desde archivo";

    const char* DIFF_NAMES[] = {
        "Muy facil", "Facil", "Medio", "Dificil", "Extremo"
    };
    for (int i = 0; i < 5; ++i) {
        difficulty_checkboxes[i].label = DIFF_NAMES[i];
    }

    player_troops_stepper.value = "500";
    player_troops_stepper.defaultValue = 500;
    player_troops_stepper.minValue = MIN_PLAYER_TROOPS;
    player_troops_stepper.maxValue = MAX_PLAYER_TROOPS;
    player_troops_stepper.step = 50;
    player_troops_stepper.maxDigits = 4;

    bots_troops_stepper.value = "250";
    bots_troops_stepper.defaultValue = 250;
    bots_troops_stepper.minValue = MIN_BOT_TROOPS;
    bots_troops_stepper.maxValue = MAX_BOT_TROOPS;
    bots_troops_stepper.step = 25;
    bots_troops_stepper.maxDigits = 4;

    num_bots_stepper.value = "10";
    num_bots_stepper.defaultValue = 10;
    num_bots_stepper.minValue = MIN_BOTS;
    num_bots_stepper.maxValue = MAX_BOTS;
    num_bots_stepper.step = 1;
    num_bots_stepper.maxDigits = 2;

    customization_back_button.label = "VOLVER";
    customization_back_button.style = ButtonStyle::Secondary;
    customization_back_button.fontSize = ui_theme.smallButtonFontSize;

    customization_play_button.label = "JUGAR";
    customization_play_button.style = ButtonStyle::Primary;
    customization_play_button.fontSize = ui_theme.smallButtonFontSize;
}


//--- handelinputs de menu, personalizacion y settings ---

void MenuState::handleMainMenuInput(){
    updateLayout();

    const Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { //comprobacion para mantener el foco en la caja de texto
        name_field.focused = CheckCollisionPointRec(mouse, name_field.bounds);
    }

    name_field.handleInput(GetFrameTime()); //nombre del jugador
    create_button.enabled = !name_field.text.empty();

    //update comprueba si esta hovered y clicado
    create_button.update(mouse);
    settings_button.update(mouse);
    exit_button.update(mouse);

    if (create_button.clicked) {
        show_customization = true;
    }
    if (settings_button.clicked) {
        show_settings = true;
    }
    if (exit_button.clicked) {
        state_machine->remove_state(true);
    }

    if (!name_field.focused) { //logica del selector de color
        if (IsKeyPressed(KEY_UP)) {
            selected_color = (selected_color - 1 + NUM_COLORS) % NUM_COLORS;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            selected_color = (selected_color + 1) % NUM_COLORS;
        }
    }
}

void MenuState::handleCustomizationInput(){
    //volver al menu de inicio
    if (IsKeyPressed(KEY_ESCAPE)){
        show_customization = false;
        return;
    }
    updateCustomizationLayout();

    const Vector2 mouse = GetMousePosition();
    map_procedural_checkbox.update(mouse);
    map_file_checkbox.update(mouse);

    // seleccion de mapa
    if (map_procedural_checkbox.clicked) {
        config.map_type = MapType::PROCEDURAL;
        config.map_filename = "";
    }
    if (map_file_checkbox.clicked && !available_maps.empty()) {
        config.map_type = MapType::MAP_FILE;
        if (selected_map_index < 0) selected_map_index = 0;
        config.map_filename = available_maps[selected_map_index];
    }
    for (int i = 0; i < static_cast<int>(available_maps.size()) && i < 6; ++i) {
        map_file_checkboxes[i].update(mouse);

        if (map_file_checkboxes[i].clicked) {
            config.map_type = MapType::MAP_FILE;
            selected_map_index = i;
            config.map_filename = available_maps[i];
        }
    }

    // seleccion de dificultad
    for (int i = 0; i < 5; ++i) {
        difficulty_checkboxes[i].update(mouse);

        if (difficulty_checkboxes[i].clicked) {
            config.difficulty = static_cast<BotDifficulty>(i);
        }
    }

    // seleccion del nº de torpas de jugador, bots y nº de bots
    const bool playerFocus = player_troops_stepper.update(mouse);
    const bool botsFocus = bots_troops_stepper.update(mouse);
    const bool numBotsFocus = num_bots_stepper.update(mouse);

    if (playerFocus || botsFocus || numBotsFocus) {
        player_troops_stepper.focused = playerFocus;
        bots_troops_stepper.focused = botsFocus;
        num_bots_stepper.focused = numBotsFocus;
    }

    if (player_troops_stepper.focused) {
        player_troops_stepper.handleInput(GetFrameTime());
    } else if (bots_troops_stepper.focused) {
        bots_troops_stepper.handleInput(GetFrameTime());
    } else if (num_bots_stepper.focused) {
        num_bots_stepper.handleInput(GetFrameTime());
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !playerFocus && !botsFocus && !numBotsFocus) {
        player_troops_stepper.focused = false;
        bots_troops_stepper.focused = false;
        num_bots_stepper.focused = false;

        player_troops_stepper.checkValue();
        bots_troops_stepper.checkValue();
        num_bots_stepper.checkValue();
    }

    customization_back_button.update(mouse);
    customization_play_button.update(mouse);

    if (customization_back_button.clicked) {
        show_customization = false;
        return;
    }

    if (customization_play_button.clicked) {
        player_troops_stepper.checkValue();
        bots_troops_stepper.checkValue();
        num_bots_stepper.checkValue();

        config.initial_troops = player_troops_stepper.numericValue();
        config.bot_troops = bots_troops_stepper.numericValue();
        config.numero_bots = num_bots_stepper.numericValue();

        state_machine->add_state(std::make_unique<MainGameState>(name_field.text, TERRITORY_COLORS[selected_color], config), true);
    }
}

void MenuState::handleSettingsInput(){
    updateLayout();

    if (IsKeyPressed(KEY_ESCAPE)) {
        show_settings = false;
        return;
    }

    settings_back_button.update(GetMousePosition());

    if (settings_back_button.clicked) {
        show_settings = false;
    }
}

// Entrada
void MenuState::handleInput()
{
    if (show_customization){
        handleCustomizationInput();
    } else if (show_settings){
        handleSettingsInput();
    } else{
        handleMainMenuInput();
    }
}

// Actualización
void MenuState::update(float deltaTime){
    (void)deltaTime;
}

// --- renderizado de menu, personalizacion y settings ---

void MenuState::renderMainMenu(){
    updateLayout();

    //carga las fuentes
    Font* bold = ResourceManager::getFont(MENU_FONT_BOLD_KEY);
    Font* regular = ResourceManager::getFont(MENU_FONT_PRESS_REGULAR_KEY);
    //carga y pinta el logo
    Texture2D* logo = ResourceManager::getTexture(LOGO_IMAGE_KEY);
    if (logo != nullptr) {
        // escala entera para no romper el logo
        const int logoScale = std::max(1, static_cast<int>(std::lround(GetScreenHeight() / 540.0f)));

        const float logoWidth = logo->width * logoScale;
        const float logoHeight = logo->height * logoScale;

        Rectangle source{0.0f, 0.0f, static_cast<float>(logo->width), static_cast<float>(logo->height)}; //la textura del logo
        
        const float nudgeLeft = 72.0f * ui_layout.scale; //para tirar el logo un poco a la izq
        Rectangle dest{ //donde y a que tamaño se dibuja
            menu_layout.logo.x + (menu_layout.logo.width  - logoWidth)  * 0.5f - nudgeLeft,
            menu_layout.logo.y + (menu_layout.logo.height - logoHeight) * 0.5f, logoWidth, logoHeight};

        DrawTexturePro(*logo, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    }

    //pinta la caja texto para escribir el nombre
    name_field.draw(ui_theme, ui_layout.scale, regular);

    // selector del color
    DrawRectangleRec(menu_layout.colorSelector, TERRITORY_COLORS[selected_color]);
    DrawRectangleLinesEx(menu_layout.colorSelector, std::max(1.0f, ui_layout.scale), ui_theme.border);

    if (!name_field.focused && regular != nullptr) {
        const float hintSize = ui_theme.smallFontSize * ui_layout.scale;
        const char* hint = "↑/↓ cambiar color";

        const Vector2 hintTextSize = MeasureTextEx(*regular, hint, hintSize, 1.0f);
        const Vector2 hintPos{menu_layout.colorHint.x, menu_layout.colorHint.y + (menu_layout.colorHint.height - hintTextSize.y) * 0.5f};
        DrawTextEx(*regular, hint, hintPos,hintSize, 1.0f, ui_theme.textMuted);
    }

    // pinta los botones
    create_button.draw(ui_theme, ui_layout.scale, bold);
    settings_button.draw(ui_theme, ui_layout.scale, bold);
    exit_button.draw(ui_theme, ui_layout.scale, bold);

    if (regular != nullptr) {
        const float small = ui_theme.smallFontSize * ui_layout.scale;

        DrawTextEx(*regular, "Desarrollado por WarGames ©", Vector2{menu_layout.credits.x, menu_layout.credits.y}, small, 1.0f, ui_theme.textMuted);

        const char* version = "v0.1.0-alpha";
        const Vector2 versionSize = MeasureTextEx(*regular, version, small, 1.0f);
        DrawTextEx(*regular, version, Vector2{menu_layout.version.x + menu_layout.version.width - versionSize.x, menu_layout.version.y}, small, 1.0f, ui_theme.textMuted);
    }
}


void MenuState::renderCustomizationOverlay(){
    updateCustomizationLayout();

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.65f));

    customization_panel.draw(ui_theme, ui_layout.scale);

    Font* bold = ResourceManager::getFont(MENU_FONT_BOLD_KEY);
    Font* regular = ResourceManager::getFont(MENU_FONT_REGULAR_KEY);
    const Font& boldFont = bold ? *bold : GetFontDefault();
    const Font& regularFont = regular ? *regular : GetFontDefault();

    const float scale = ui_layout.scale;
    const float titleSize = 32.0f * scale;
    const float sectionSize = ui_theme.sectionFontSize * scale;
    const float labelSize = ui_theme.checkboxFontSize * scale;

    // Título
    const char* title = "PERSONALIZAR PARTIDA";
    const Vector2 titleMeasure = MeasureTextEx(boldFont, title, titleSize, 1.0f);

    DrawTextEx(boldFont, title,Vector2{customization_layout.title.x + (customization_layout.title.width - titleMeasure.x) * 0.5f,
                                    customization_layout.title.y + (customization_layout.title.height - titleMeasure.y) * 0.5f}, titleSize, 1.0f, ui_theme.text);

    // Columnas
    DrawTextEx(regularFont, "Mapa", { customization_layout.mapLabel.x, customization_layout.mapLabel.y }, sectionSize, 1.0f, ui_theme.text);

    DrawTextEx(regularFont, "Dificultad", { customization_layout.difficultyLabel.x, customization_layout.difficultyLabel.y }, sectionSize, 1.0f, ui_theme.text);

    DrawTextEx(regularFont, "Tropas", { customization_layout.troopsLabel.x, customization_layout.troopsLabel.y }, sectionSize, 1.0f, ui_theme.text);

    // Columna 1: mapa
    map_procedural_checkbox.draw(ui_theme, scale, regular);
    map_file_checkbox.draw(ui_theme, scale, regular);

    if (!available_maps.empty()) {
        DrawTextEx(regularFont, "Archivos disponibles:", { customization_layout.mapsLabel.x, customization_layout.mapsLabel.y }, labelSize, 1.0f, ui_theme.textMuted);

        for (int i = 0; i < static_cast<int>(available_maps.size()) && i < 6; ++i) {
            map_file_checkboxes[i].draw(ui_theme, scale, regular);
        }
    } else {
        DrawTextEx(regularFont, "No hay mapas en maps/", { customization_layout.mapFiles[0].x, customization_layout.mapFiles[0].y }, labelSize, 1.0f, ui_theme.textMuted);
    }

    // Columna 2: dificultad
    for (int i = 0; i < 5; ++i) {
        difficulty_checkboxes[i].draw(ui_theme, scale, regular);
    }

    // Columna 3: tropas
    DrawTextEx(regularFont, "Jugador (100 - 2000):", { customization_layout.playerLabel.x, customization_layout.playerLabel.y }, labelSize, 1.0f, ui_theme.textMuted);
    player_troops_stepper.draw(ui_theme, scale, regular);

    DrawTextEx(regularFont, "Bots (50 - 1000):", { customization_layout.botsLabel.x, customization_layout.botsLabel.y }, labelSize, 1.0f, ui_theme.textMuted);
    bots_troops_stepper.draw(ui_theme, scale, regular);

    DrawTextEx(regularFont, "Nº Bots (1 - 15):", { customization_layout.numBotsLabel.x, customization_layout.numBotsLabel.y }, labelSize, 1.0f, ui_theme.textMuted);
    num_bots_stepper.draw(ui_theme, scale, regular);

    // Botones inferiores
    customization_back_button.draw(ui_theme, scale, bold);
    customization_play_button.draw(ui_theme, scale, bold);
}

void MenuState::renderSettingsOverlay (){
    updateLayout();

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.65f));

    const Rectangle panel = ui_layout.rect(660.0f, 300.0f, 600.0f, 400.0f);
    DrawRectangleRec(panel, ui_theme.panel);
    DrawRectangleLinesEx(panel, std::max(1.0f, ui_layout.scale), ui_theme.border);

    Font* bold = ResourceManager::getFont(MENU_FONT_BOLD_KEY);
    Font* regular = ResourceManager::getFont(MENU_FONT_REGULAR_KEY);

    if (bold != nullptr) {
        const float titleSize = 32.0f * ui_layout.scale;
        const Vector2 titleW = MeasureTextEx(*bold, "AJUSTES", titleSize, 1.0f);
        DrawTextEx(*bold, "AJUSTES", Vector2{panel.x + (panel.width - titleW.x) * 0.5f, panel.y + 40.0f * ui_layout.scale}, titleSize, 1.0f, ui_theme.text);
    }

    if (regular != nullptr) {
        const float textSize = 22.0f * ui_layout.scale;
        const char* line1 = "EN DESARROLLO";
        
        const Vector2 w1 = MeasureTextEx(*regular, line1, textSize, 1.0f);
        
        DrawTextEx(*regular, line1, Vector2{panel.x + (panel.width - w1.x) * 0.5f, panel.y + 140.0f * ui_layout.scale}, textSize, 1.0f, ui_theme.textMuted);
    }

    settings_back_button.draw(ui_theme, ui_layout.scale, bold);
}

// Renderizado
void MenuState::render()
{
    BeginDrawing();
    ClearBackground(ui_theme.background);

    renderMainMenu();

    if (show_customization){
        renderCustomizationOverlay();
    } else if (show_settings){
        renderSettingsOverlay();
    }

    EndDrawing();
}
