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
static const char* COLOR_NAMES[] = {
    "Rojo", "Azul", "Verde", "Amarillo", "Morado", "Naranja"
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

void MenuState::updateLayout(){
    ui_layout.update(GetScreenWidth(), GetScreenHeight());

    //calcula donde se colocan los rectangulos en la pantalla real
    menu_layout.logo = ui_layout.rect(612.0f,  80.0f, 696.0f, 220.0f); //logo *2
    menu_layout.nameField = ui_layout.rect(690.0f, 436.0f, 460.0f,  64.0f);
    menu_layout.colorSelector = ui_layout.rect(1166.0f, 436.0f,  64.0f, 64.0f);
    menu_layout.colorHint = ui_layout.rect(1246.0f, 456.0f, 260.0f, 24.0f);
    menu_layout.primaryButton = ui_layout.rect(700.0f, 510.0f, 520.0f,  76.0f);
    menu_layout.settingsButton = ui_layout.rect(860.0f, 630.0f, 200.0f,  56.0f);
    menu_layout.exitButton = ui_layout.rect(860.0f, 710.0f, 200.0f,  56.0f);
    menu_layout.credits = ui_layout.rect(48.0f, 1020.0f, 500.0f, 30.0f);
    menu_layout.version = ui_layout.rect(1372.0f, 1020.0f, 500.0f, 30.0f);

    //le pasa los atributos a los elementos del menu
    name_field.bounds = menu_layout.nameField;
    create_button.bounds = menu_layout.primaryButton;
    settings_button.bounds = menu_layout.settingsButton;
    exit_button.bounds = menu_layout.exitButton;
    settings_back_button.bounds = ui_layout.rect(860.0f, 620.0f, 200.0f, 56.0f);
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
        active_troop_box = 0;
        return;
    }

    if (active_troop_box != 0){
        std::string& currentStr = (active_troop_box == 1) ? player_troops_str : (active_troop_box == 2) ? bot_troops_str : num_bots_str;
        std::size_t maxLimit = (active_troop_box == 1) ? MAX_PLAYER_TROOPS : (active_troop_box == 2) ? MAX_BOT_TROOPS : MAX_BOTS;

        // escribir dígitos
        int key = GetCharPressed();
        while (key > 0){
            int maxlen = (active_troop_box == 3) ? 2 : 4;
            if (key >= '0' && key <= '9' && currentStr.length() < maxlen) { // solo numeros
                std::string testStr = currentStr + static_cast<char>(key);
                unsigned long val = std::stoul(testStr);
                if (val <= maxLimit){
                    currentStr = testStr;
                }
            }
            key = GetCharPressed();
        }

        // Borrar
        if (IsKeyPressed(KEY_BACKSPACE) && !currentStr.empty()){
            currentStr.pop_back();
        }
    }

    // procesar los clics
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        return;
    }

    Vector2 mouse = GetMousePosition();

    // coordenadas (deben coincidir con las del render)
    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();
    const int panelW = 1000;
    const int panelH = 700;
    const int panelX = (screenW - panelW) / 2;
    const int panelY = (screenH - panelH) / 2;
    const int colW = 270;
    const int colGap = 40;
    const int startY = panelY + 75;
    const int col1X = panelX + 60;
    const int col2X = col1X + colW + colGap;
    const int col3X = col2X + colW + colGap;

    // Columna 1 - MAPA
    // checkbox "Procedural" (misma posición que en el render: col1X, startY + 35)
    Rectangle cbProcedural = { (float)col1X, (float)(startY + 35), 200, 18 };
    if (CheckCollisionPointRec(mouse, cbProcedural)){
        config.map_type = MapType::PROCEDURAL;
        config.map_filename = "";
    }

    // checkbox "Desde archivo"
    Rectangle cbFile = { (float)col1X, (float)(startY + 65), 200, 18 };
    if (CheckCollisionPointRec(mouse, cbFile) && !available_maps.empty()){
        config.map_type = MapType::MAP_FILE;
        // Si no hay ninguno seleccionado aún, seleccionar el primero
        if (selected_map_index < 0) selected_map_index = 0;
        config.map_filename = available_maps[selected_map_index];
    }

    // clic en la lista de archivos
    for (size_t i = 0; i < available_maps.size() && i < 6; i++){
        int itemY = startY + 135 + static_cast<int>(i) * 30;
        Rectangle itemRect = { (float)(col1X + 10), (float)itemY, 250, 18 };
        if (CheckCollisionPointRec(mouse, itemRect)){
            config.map_type = MapType::MAP_FILE;
            selected_map_index = static_cast<int>(i);
            config.map_filename = available_maps[selected_map_index];
        }
    }

    // Columna 2 - DIFICULTAD
    for (int i = 0; i < 5; i++){
        int y = startY + 35 + i * 35;
        Rectangle cbDiff = { (float)col2X, (float)y, 200, 18 };
        if (CheckCollisionPointRec(mouse, cbDiff)){
            config.difficulty = static_cast<BotDifficulty>(i);
        }
    }


    // Columna 3 - Tropas iniciales del jugador y bots, y numero de bots
    int playerRowY = startY + 60;
    Rectangle rectMinusPlayer = {(float)col3X, (float)playerRowY, 45, 40};
    Rectangle rectBoxPlayer = {(float)(col3X + 55), (float)playerRowY, 100, 40};
    Rectangle rectPlusPlayer = {(float)(col3X + 165), (float)playerRowY, 45, 40};

    int botRowY = startY + 150;
    Rectangle rectMinusBot = {(float)col3X, (float)botRowY, 45, 40};
    Rectangle rectBoxBot = {(float)(col3X + 55), (float)botRowY, 100, 40};
    Rectangle rectPlusBot = {(float)(col3X + 165), (float)botRowY, 45, 40};

    int numBotsRowY = startY + 240;                                                                                                                                                       
    Rectangle rectMinusNumBots = {(float)col3X, (float)numBotsRowY, 45, 40};
    Rectangle rectBoxNumBots = {(float)(col3X + 55), (float)numBotsRowY, 100, 40};
    Rectangle rectPlusNumBots = {(float)(col3X + 165), (float)numBotsRowY, 45, 40};

    // [-] Jugador
    if (CheckCollisionPointRec(mouse, rectMinusPlayer)){
        std::size_t val = player_troops_str.empty() ? 500 : std::stoul(player_troops_str);
        val = (val <= MIN_PLAYER_TROOPS + 50) ? MIN_PLAYER_TROOPS : val - 50;
        player_troops_str = std::to_string(val);
    }
    // [+] Jugador
    if (CheckCollisionPointRec(mouse, rectPlusPlayer)){
        std::size_t val = player_troops_str.empty() ? 500 : std::stoul(player_troops_str);
        val = std::min(MAX_PLAYER_TROOPS, val + 50);
        player_troops_str = std::to_string(val);
    }
    // Foco caja jugador
    if (CheckCollisionPointRec(mouse, rectBoxPlayer)){
        active_troop_box = 1;
    }

    // [-] Bots
    if (CheckCollisionPointRec(mouse, rectMinusBot)){
        std::size_t val = bot_troops_str.empty() ? 250 : std::stoul(bot_troops_str);
        val = (val <= MIN_BOT_TROOPS + 25) ? MIN_BOT_TROOPS : val - 25;
        bot_troops_str = std::to_string(val);
    }
    // [+] Bots
    if (CheckCollisionPointRec(mouse, rectPlusBot)){
        std::size_t val = bot_troops_str.empty() ? 250 : std::stoul(bot_troops_str);
        val = std::min(MAX_BOT_TROOPS, val + 25);
        bot_troops_str = std::to_string(val);
    }
    // Foco caja bots
    if (CheckCollisionPointRec(mouse, rectBoxBot)){
        active_troop_box = 2;
    }

    // [-] nº bots
    if (CheckCollisionPointRec(mouse, rectMinusNumBots)){
        std::size_t val = num_bots_str.empty() ? 10 : std::stoul(num_bots_str);
        val = (val <=  MIN_BOTS) ?  MIN_BOTS : val - 1;
        num_bots_str = std::to_string(val);
    }
    // [+] nº bots
    if (CheckCollisionPointRec(mouse, rectPlusNumBots)){
        std::size_t val = num_bots_str.empty() ? 10 : std::stoul(num_bots_str);
        val = std::min(MAX_BOTS, val + 1);
        num_bots_str = std::to_string(val);
    }
    // Foco caja nº bots
    if (CheckCollisionPointRec(mouse, rectBoxNumBots)){
        active_troop_box = 3;
    }


    // Quitar foco si haces clic fuera de las cajas de texto
    if (!CheckCollisionPointRec(mouse, rectBoxPlayer) && !CheckCollisionPointRec(mouse, rectBoxBot) && !CheckCollisionPointRec(mouse, rectBoxNumBots)){
        active_troop_box = 0;
        // validar tropas del jugador al salir de la caja
        if (player_troops_str.empty()) {
            player_troops_str = "500"; // si lo dejó vacío, vuelve al valor por defecto
        } else {
            std::size_t val = std::stoul(player_troops_str);
            if (val < MIN_PLAYER_TROOPS) player_troops_str = std::to_string(MIN_PLAYER_TROOPS); // Si puso 20, se sube a 100
            if (val > MAX_PLAYER_TROOPS) player_troops_str = std::to_string(MAX_PLAYER_TROOPS);
        }
  
        // validar tropas de los bots al salir de la caja
        if (bot_troops_str.empty()) {
            bot_troops_str = "250";
        } else {
            std::size_t val = std::stoul(bot_troops_str);
            if (val < MIN_BOT_TROOPS) bot_troops_str = std::to_string(MIN_BOT_TROOPS); // Si puso 10, se sube a 50
            if (val > MAX_BOT_TROOPS) bot_troops_str = std::to_string(MAX_BOT_TROOPS);
        }

        //validar numero de bots
        if (num_bots_str.empty()) {
            num_bots_str = "10";
        } else {
            std::size_t val = std::stoul(num_bots_str);
            if (val < MIN_BOTS) num_bots_str = std::to_string(MIN_BOTS); // Si puso menos de 1, se pone en 1
            if (val > MAX_BOTS) num_bots_str = std::to_string(MAX_BOTS);
        }
    }

    // Botones inferiores
    const int btnW = 180;
    const int btnH = 50;
    const int btnGap = 30;
    const int totalBtnW = btnW * 2 + btnGap;
    const int btnStartX = panelX + (panelW - totalBtnW) / 2;
    const int btnY = panelY + panelH - 75;

    // botón ESC
    Rectangle escRect = {(float)btnStartX, (float)btnY, (float)btnW, (float)btnH};
    if (CheckCollisionPointRec(mouse, escRect)){
        show_customization = false;
        active_troop_box = 0;
    }

    // Botón JUGAR
    int jugarX = btnStartX + btnW + btnGap;
    Rectangle jugarRect = { (float)jugarX, (float)btnY, (float)btnW, (float)btnH };
    if (CheckCollisionPointRec(mouse, jugarRect)){
        // validar tropas
        std::size_t pTroops = player_troops_str.empty() ? MIN_PLAYER_TROOPS : std::stoul(player_troops_str);
        pTroops = std::max(MIN_PLAYER_TROOPS, std::min(MAX_PLAYER_TROOPS, pTroops));
        config.initial_troops = pTroops;

        std::size_t bTroops = bot_troops_str.empty() ? MIN_BOT_TROOPS : std::stoul(bot_troops_str);
        bTroops = std::max(MIN_BOT_TROOPS, std::min(MAX_BOT_TROOPS, bTroops));
        config.bot_troops = bTroops;
        
        std::size_t nBots = num_bots_str.empty() ? 10 : std::stoul(num_bots_str);
        nBots = std::max(MIN_BOTS, std::min(MAX_BOTS, nBots));
        config.numero_bots = nBots;

        // iniciar partida
        this->state_machine->add_state(std::make_unique<MainGameState>(name_field.text, TERRITORY_COLORS[selected_color], config), true);
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

    // Número de Bots
    int numBotsY = startY + 215;
    DrawText("Cantidad de Bots (1 - 15):", col3X, numBotsY, 16, GRAY);

    int numBotsRowY = numBotsY + 25;

    // [-]
    DrawRectangle(col3X, numBotsRowY, 45, 40, Color{ 55, 60, 70, 255 });
    DrawRectangleLines(col3X, numBotsRowY, 45, 40, GRAY);
    DrawText("-", col3X + 16, numBotsRowY + 6, 28, RAYWHITE);

    // Caja del número
    bool focusNumBots = (active_troop_box == 3);
    DrawRectangle(col3X + 55, numBotsRowY, 100, 40, focusNumBots ? Color{ 20, 25, 30, 255 } : Color{ 40, 45, 55, 255 });
    DrawRectangleLines(col3X + 55, numBotsRowY, 100, 40, focusNumBots ? GREEN : GRAY);
    int nbW = MeasureText(num_bots_str.c_str(), 22);
    DrawText(num_bots_str.c_str(), col3X + 55 + (100 - nbW) / 2, numBotsRowY + 10, 22, RAYWHITE);

    // [+]
    DrawRectangle(col3X + 165, numBotsRowY, 45, 40, Color{ 55, 60, 70, 255 });
    DrawRectangleLines(col3X + 165, numBotsRowY, 45, 40, GRAY);
    DrawText("+", col3X + 165 + 14, numBotsRowY + 6, 28, RAYWHITE);

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
