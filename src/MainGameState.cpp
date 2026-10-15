#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <MenuState.hpp>
#include <Target.hpp>
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{

Color cellColor(const TerritoryCell& cell)
{
    if (cell.owner != nullptr) return cell.owner->getColor();
    if (cell.owner == nullptr && cell.isWater) return Color{0,0,0,255};
    return Color{55, 63, 73, 255};
}

//esto lo que hace es ver el color de fondo de cada jugador y dependiendo el contraste que genere pone el nombre en blanco o negro
Color readableTextColor(Color background){
    const int luminance =(299 * static_cast<int>(background.r)+ 587 * static_cast<int>(background.g) + 114 * static_cast<int>(background.b)) / 1000;
    return luminance > 140 ? Color{20, 24, 30, 255} : RAYWHITE;
}

constexpr float LABEL_WIDTH_FILL = 0.70f;
constexpr float LABEL_HEIGHT_FILL = 0.70f;
//esta constante es para el nivel de grande
constexpr float LABEL_SIZE_FACTOR = 0.75f;
constexpr int LABEL_MIN_FONT = 8;
constexpr int LABEL_MAX_FONT = 40;
//constante para las trops, subirlo si las queremos mas pequeñas ese texto
constexpr float LABEL_TROOPS_SCALE = 0.65f;
constexpr float LABEL_LINE_GAP = 0.25f;

//esto lo que hace es la etiqueta de la tropas y nombres para mostrarla
struct TerritoryLabel{
    Entity* owner = nullptr;
    std::string name;
    std::string troops;
    Rectangle rect{};
    std::size_t cells = 0; 
    int nameFontSize = 0;
    int troopsFontSize = 0;
    bool visible = false;
};

//aqui estan los datos de un territorio
struct TerritoryStats{
    int minColumn = std::numeric_limits<int>::max();
    int maxColumn = std::numeric_limits<int>::min();
    int minRow = std::numeric_limits<int>::max();
    int maxRow = std::numeric_limits<int>::min();
    double columnSum = 0.0;
    double rowSum = 0.0;
    std::size_t cells = 0;
};

void clampToBounds(Rectangle& rect, const Rectangle& bounds){
    //evita que el rectangulo salga del limite de ancho por la izquierda y por la derecha de ancho
    if (rect.width <= bounds.width){
        rect.x = std::min(std::max(rect.x, bounds.x),bounds.x + bounds.width - rect.width);
    }

    //igual para la altura
    if (rect.height <= bounds.height){
        rect.y = std::min(std::max(rect.y, bounds.y),bounds.y + bounds.height - rect.height);
    }
}

bool findOwnedCellNear(const Map& map, Entity* owner,int startColumn, int startRow, int& outColumn, int& outRow){
    const int columns = static_cast<int>(map.getWidth());
    const int rows = static_cast<int>(map.getHeight());
    const int maxRadius = std::max(columns, rows);

    for (int radius = 0; radius <= maxRadius; ++radius)
    {
        for (int deltaRow = -radius; deltaRow <= radius; ++deltaRow)
        {
            for (int deltaColumn = -radius; deltaColumn <= radius; ++deltaColumn)
            {
                if (std::abs(deltaColumn) != radius && std::abs(deltaRow) != radius){
                    continue;
                }
                const int column = startColumn + deltaColumn;
                const int row = startRow + deltaRow;
                if (column < 0 || row < 0 || column >= columns || row >= rows) {
                    continue;
                }

                if (map.getCell(column, row).owner != owner) {
                    continue;
                }

                outColumn = column;
                outRow = row;
                return true;
            }
        }
    }
    return false;
}

std::unordered_map<Entity*, TerritoryStats> collectTerritoryStats(
    const Map& map, const std::vector<Entity*>& entities){
    std::unordered_map<Entity*, TerritoryStats> stats;
    stats.reserve(entities.size());

    for (std::size_t row = 0; row < map.getHeight(); ++row){
        for (std::size_t column = 0; column < map.getWidth(); ++column){
            Entity* owner = map.getCell(column, row).owner;
            if (owner == nullptr) {
                continue;
            }

            //saca las coordenadas para calcular el centroide
            TerritoryStats& s = stats[owner];
            s.minColumn = std::min(s.minColumn, static_cast<int>(column));
            s.maxColumn = std::max(s.maxColumn, static_cast<int>(column));
            s.minRow = std::min(s.minRow, static_cast<int>(row));
            s.maxRow = std::max(s.maxRow, static_cast<int>(row));
            s.columnSum += static_cast<double>(column);
            s.rowSum += static_cast<double>(row);
            ++s.cells;
        }
    }
    return stats;
}


//esta lo que haces es ir calculando el tamaño del territiorio para ajustar el texto cada vez que se aumenta o disminuye el territorio
TerritoryLabel buildLabel(const Map& map, Entity* owner, const TerritoryStats& stats, const Rectangle& mapBounds){
    TerritoryLabel label;
    label.owner = owner;
    label.cells = stats.cells;

    //no se muestra si no tenemos un territorio
    if (stats.cells == 0) {
        return label;
    }

    //se calcula el centroide
    const int centroidColumn =static_cast<int>(std::lround(stats.columnSum / static_cast<double>(stats.cells)));
    const int centroidRow =static_cast<int>(std::lround(stats.rowSum / static_cast<double>(stats.cells)));
    int anchorColumn = centroidColumn;
    int anchorRow = centroidRow;

    //si no es del owner se busca la mas proxima
    if (map.getCell(static_cast<std::size_t>(anchorColumn), static_cast<std::size_t>(anchorRow)).owner != owner){
        findOwnedCellNear(map, owner, centroidColumn, centroidRow, anchorColumn, anchorRow);
    }
    const float cellWidth = mapBounds.width / static_cast<float>(map.getWidth());
    const float cellHeight = mapBounds.height / static_cast<float>(map.getHeight());
    
    //este es el rectangulo donde se muestran el nombre y las tropas
    const Rectangle box{
        mapBounds.x + stats.minColumn * cellWidth,
        mapBounds.y + stats.minRow * cellHeight,
        (stats.maxColumn - stats.minColumn + 1) * cellWidth,
        (stats.maxRow - stats.minRow + 1) * cellHeight
    };

    label.name = owner->getName();
    label.troops = TextFormat("%d", static_cast<int>(owner->getTroops()));

    //calcula el tamaño del nombre
    const int referenceSize = 100;
    const int nameWidthRef = MeasureText(label.name.c_str(), referenceSize);
    int nameFontSize = nameWidthRef <= 0 ? referenceSize : static_cast<int>(referenceSize * (box.width * LABEL_WIDTH_FILL) / static_cast<float>(nameWidthRef));
    nameFontSize = std::min(static_cast<int>(nameFontSize * LABEL_SIZE_FACTOR), LABEL_MAX_FONT);

    //lo mismo con las tropas
    float gap = nameFontSize * LABEL_LINE_GAP;
    int troopsFontSize = static_cast<int>(nameFontSize * LABEL_TROOPS_SCALE);
    float blockHeight = static_cast<float>(nameFontSize + troopsFontSize) + gap;

    //si es mas alto que el bloque donde se mete, se reduce 
    const float maxBlockHeight = box.height * LABEL_HEIGHT_FILL;
    if (blockHeight > maxBlockHeight && blockHeight > 0.0f){
        const float scale = maxBlockHeight / blockHeight;
        nameFontSize = static_cast<int>(nameFontSize * scale);
        troopsFontSize = static_cast<int>(nameFontSize * LABEL_TROOPS_SCALE);
        gap = nameFontSize * LABEL_LINE_GAP;
        blockHeight = static_cast<float>(nameFontSize + troopsFontSize) + gap;
    }

    //si es muy pequeño no se muestra
    if (nameFontSize < LABEL_MIN_FONT) {
        return label;
    }

    const int textWidth =std::max(MeasureText(label.name.c_str(), nameFontSize),MeasureText(label.troops.c_str(), troopsFontSize));
    const float anchorX = mapBounds.x + (anchorColumn + 0.5f) * cellWidth;
    const float anchorY = mapBounds.y + (anchorRow + 0.5f) * cellHeight;

    //lo centra y lo marca visible
    label.nameFontSize = nameFontSize;
    label.troopsFontSize = troopsFontSize;
    label.rect = Rectangle{
        anchorX - textWidth / 2.0f,
        anchorY - blockHeight / 2.0f,
        static_cast<float>(textWidth),
        blockHeight
    };
    label.visible = true;
    return label;
}

//dibuja la etiqueta en pantalla
void drawTerritoryLabel(const TerritoryLabel& label){
    if (!label.visible) {
        return;
    }

    const Color color = readableTextColor(label.owner->getColor());
    const float gap = label.nameFontSize * LABEL_LINE_GAP;
    const float nameWidth = MeasureText(label.name.c_str(), label.nameFontSize);
    const float troopsWidth = MeasureText(label.troops.c_str(), label.troopsFontSize);

    DrawText(label.name.c_str(),static_cast<int>(label.rect.x + (label.rect.width - nameWidth) / 2.0f),static_cast<int>(label.rect.y),label.nameFontSize, color);
    DrawText(label.troops.c_str(),static_cast<int>(label.rect.x + (label.rect.width - troopsWidth) / 2.0f),static_cast<int>(label.rect.y + label.nameFontSize + gap),label.troopsFontSize, color);
}


//esta resuelve el solape entre varios territorios
void separateLabels(std::vector<TerritoryLabel>& labels, const Rectangle& mapBounds){
    for (std::size_t i = 0; i < labels.size(); ++i){
        if (!labels[i].visible) {
            continue;
        }
        for (std::size_t j = i + 1; j < labels.size(); ++j){
            if (!labels[j].visible) {
                continue;
            }

            TerritoryLabel& a = labels[i];
            TerritoryLabel& b = labels[j];
            if (!CheckCollisionRecs(a.rect, b.rect)) {
                continue;
            }

            const Rectangle& ra = a.rect;
            const Rectangle& rb = b.rect;
            const float overlapX =std::min(ra.x + ra.width, rb.x + rb.width) - std::max(ra.x, rb.x);
            const float overlapY =std::min(ra.y + ra.height, rb.y + rb.height) - std::max(ra.y, rb.y);

            //si el solape horizaontal el menor lo mueve a la dir opuesta 1 pixel 
            if (overlapX < overlapY){
                const float shift = overlapX / 2.0f + 1.0f;
                const float direction =(ra.x + ra.width / 2.0f <= rb.x + rb.width / 2.0f) ? -1.0f : 1.0f;
                a.rect.x += direction * shift;
                b.rect.x -= direction * shift;
            }
            //si el vertical es menor lo hace con este 
            else{
                const float shift = overlapY / 2.0f + 1.0f;
                const float direction =(ra.y + ra.height / 2.0f <= rb.y + rb.height / 2.0f) ? -1.0f : 1.0f;
                a.rect.y += direction * shift;
                b.rect.y -= direction * shift;
            }

            clampToBounds(a.rect, mapBounds);
            clampToBounds(b.rect, mapBounds);
        }
    }

    //si sigue con solapes se oculta la de menor tamaño
    for (std::size_t i = 0; i < labels.size(); ++i){
        if (!labels[i].visible) {
            continue;
        }
        for (std::size_t j = i + 1; j < labels.size(); ++j){
            if (!labels[j].visible) {
                continue;
            }
            if (!CheckCollisionRecs(labels[i].rect, labels[j].rect)) {
                continue;
            }
            if (labels[i].cells >= labels[j].cells) {
                labels[j].visible = false;
            }
            else {
                labels[i].visible = false;
                break;
            }
        }
    }
}
}

MainGameState::MainGameState(std::string playerName, Color playerColor, const GameConfig &config)
    : game(std::move(playerName), playerColor, config), config(config)
{
}

void MainGameState::init()
{
    game.reset();
}

void MainGameState::handleInput()
{
    if (game.isOver())
    {
        if (IsKeyPressed(KEY_R)) {
            this->state_machine->add_state(
                std::make_unique<MenuState>(),
                true
            );
        }
        return;
    }

    if(game.getWinner() != nullptr){
        if (GetCharPressed() != 0) {
            this->state_machine->add_state(
                std::make_unique<MenuState>(),
                true
            );
        }
        return;
    }

    if (game.getPhase() == GamePhase::SPAWN) {                                                                                      
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {                                                                                       
            const std::pair<int, int> targetCoordinates = getCellCoordinates(GetMousePosition());                                            
            const int column = targetCoordinates.first;                                                                                      
            const int row = targetCoordinates.second;                                                                                        
                                                                                                                                                
            if (column >= 0 && row >= 0) {                                                           
                game.setSpawnPreview(column + row * game.getMap().getWidth());                                                               
            }                                                                                                                                
        }                                                                                                                                    
        return;                                   
    }     

    const Vector2 mousePosition = GetMousePosition();
    const Rectangle slider = getAttackSliderBounds();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        && CheckCollisionPointRec(mousePosition, slider))
    {
        dragging_attack_slider = true;
    }

    if (dragging_attack_slider && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        float ratio = (mousePosition.x - slider.x) / slider.width;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        attack_percent = static_cast<int>(ratio * 100.0f + 0.5f);
        return;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        dragging_attack_slider = false;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ESCAPE))
    {
        game.cancelPlayerAttack();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const std::pair<int, int> targetCoordinates = getCellCoordinates(GetMousePosition());
        const int column = targetCoordinates.first;
        const int row = targetCoordinates.second;
        if (column < 0 || row < 0) return; // Clic fuera del tablero.

        const TerritoryCell& cell = game.getMap().getCell(
            static_cast<std::size_t>(column), static_cast<std::size_t>(row));

        if (cell.isWater) return;

        const float ratio = static_cast<float>(attack_percent) / 100.0f;

        if (cell.owner == nullptr)
        {
            game.setPlayerTarget(Target::land(), ratio);
        }
        else if (cell.owner != game.getPlayer())
        {
            game.setPlayerTarget(Target::enemy(cell.owner), ratio);
        }
    }
}

void MainGameState::update(float deltaTime)
{
    // El estado de presentación delega toda la simulación en Game.
    game.update(deltaTime);
}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(Color{24, 29, 36, 255});

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const Map& map = game.getMap();
    Player* player = game.getPlayer();
    Bot* bot = (Bot* )game.getEntities()[1];
    const bool gameOver = game.isOver();
    const auto winner = game.getWinner();
    std::size_t playerCells = 0;
    std::size_t botCells = 0;
    std::size_t landCells = 0;

    for (const TerritoryCell& cell : map.getCells())
    {
        if (cell.isWater) continue;

        ++landCells;
        if (cell.owner == player)
        {
            ++playerCells;
        }
        else if (cell.owner != nullptr)
        {
            ++botCells;
        }
    }
    const int playerPercent = landCells == 0 ? 0
        : static_cast<int>(100.0 * static_cast<double>(playerCells) / landCells);
    const int botPercent = landCells == 0 ? 0
        : static_cast<int>(100.0 * static_cast<double>(botCells) / landCells);

    DrawRectangle(0, 0, screenWidth, 108, Color{31, 38, 47, 255});
    if (game.getPhase() == GamePhase::SPAWN) {
        float timer = game.getSpawnTimer();

        const char* title = "ELIGE TU PUNTO DE PARTIDA";
        DrawText(title, (screenWidth - MeasureText(title, 30)) / 2, 20, 30, RAYWHITE);
        if (timer>=11.0f){
        } else{
            int timeLeft = static_cast<int>(timer);
            std::string clock = TextFormat("El juego comienza en: %d s", timeLeft);
            DrawText(clock.c_str(), (screenWidth - MeasureText(clock.c_str(), 24)) / 2, 60, 24, LIGHTGRAY);
        }
    } else{
        const char* title = "CONQUISTA EL MAPA";
        DrawText(title, (screenWidth - MeasureText(title, 30)) / 2, 10, 30, RAYWHITE);
        const int totalSeconds = static_cast<int>(game.getTime());
        const std::string clock = TextFormat("%02d:%02d", totalSeconds / 60, totalSeconds % 60);
        DrawText(clock.c_str(), (screenWidth - MeasureText(clock.c_str(), 24)) / 2, 55, 24, LIGHTGRAY);
    }

    const Rectangle bounds = getMapBounds();
    DrawRectangleRec(bounds, Color{38, 45, 54, 255});
    const float cellWidth = bounds.width / static_cast<float>(map.getWidth());
    const float cellHeight = bounds.height / static_cast<float>(map.getHeight());
    const Vector2 mousePosition = GetMousePosition();

    for(std::size_t row = 0; row < map.getHeight(); row++){
        for(std::size_t column = 0; column < map.getWidth(); column++){

            const Rectangle cellBounds{
                bounds.x + column * cellWidth,
                bounds.y + row * cellHeight,
                cellWidth,
                cellHeight
            };
            const TerritoryCell& cell = map.getCell(column, row);
            const std::size_t cellIndex = row * map.getWidth() + column;
            DrawRectangleRec(cellBounds, cellColor(cell));
            const Color ownershipColor = cell.owner == player ? player->getColor(): Color{31, 37, 45, 255};
        }
    }

    {
        const std::vector<Entity*> entities = game.getEntities();
        const std::unordered_map<Entity*, TerritoryStats> stats =
            collectTerritoryStats(map, entities);

        std::vector<TerritoryLabel> labels;
        labels.reserve(entities.size());
        const TerritoryStats empty;
        for (Entity* entity : entities)
        {
            const auto it = stats.find(entity);
            labels.push_back(buildLabel(map, entity,
                                        it != stats.end() ? it->second : empty, bounds));
        }

        separateLabels(labels, bounds);
        for (const TerritoryLabel& label : labels)
        {
            drawTerritoryLabel(label);
        }
    }

    if (game.getPhase() == GamePhase::SPAWN) {                                                                                               
        int previewIdx = game.getSpawnPreview();                                                                                             
        if (previewIdx >= 0) {                                                                                                               
            int cx = previewIdx % map.getWidth();                                                                                            
            int cy = previewIdx / map.getWidth();                                                                                            
                                                                                                                                                
            Color previewColor = player->getColor();                                                                                         
            previewColor.a = 150;                                                                              
                                                                                                                                                
            for (int dy = -1; dy <= 1; ++dy) {                                                                                               
                for (int dx = -1; dx <= 1; ++dx) {                                                                                           
                    int px = cx + dx;                                                                                                        
                    int py = cy + dy;                                                                                                        
                    if (px >= 0 && px < map.getWidth() && py >= 0 && py < map.getHeight()) {                                                 
                        const TerritoryCell& cell = map.getCell(px, py);
                        
                        if (!cell.isWater && cell.owner == nullptr) { 
                            Rectangle cellBounds{                                                                                                
                                bounds.x + px * cellWidth,                                                                                       
                                bounds.y + py * cellHeight,                                                                                      
                                cellWidth,                                                                                                       
                                cellHeight                                                                                                       
                            };                                                                                                                   
                            DrawRectangleRec(cellBounds, previewColor);                                                                          
                        }
                    }                                                                                                                        
                }                                                                                                                            
            }                                                                                                                                
        }                                                                                                                                    
    }
               
    if (game.getPhase() == GamePhase::PLAYING) {
        const Rectangle slider = getAttackSliderBounds();
        const float sliderRatio = static_cast<float>(attack_percent) / 100.0f;
        const Color playerColor = player->getColor();

        DrawRectangleRounded(slider, 0.5f, 8, Color{24, 29, 36, 255});
        if (sliderRatio > 0.0f)
        {
            Rectangle fill = slider;
            fill.width = slider.width * sliderRatio;
            DrawRectangleRounded(fill, 0.5f, 8, playerColor);
        }

        const float knobX = slider.x + slider.width * sliderRatio;
        DrawCircle(static_cast<int>(knobX),
                static_cast<int>(slider.y + slider.height / 2.0f),
                slider.height * 0.62f, RAYWHITE);

        const std::size_t attackTroops = static_cast<std::size_t>(
            static_cast<float>(player->getTroops()) * sliderRatio);
        const char* sliderLabel = TextFormat("%d%% (%d)", attack_percent,
                                            static_cast<int>(attackTroops));

        const int labelFontSize = 20;
        const int labelWidth = MeasureText(sliderLabel, labelFontSize);
        const int playerLuminance =
            (299 * static_cast<int>(playerColor.r)
            + 587 * static_cast<int>(playerColor.g)
            + 114 * static_cast<int>(playerColor.b)) / 1000;
        const Color labelColor =
            (sliderRatio >= 0.5f && playerLuminance > 140) ? Color{20, 24, 30, 255}
                                                        : RAYWHITE;
        DrawText(sliderLabel,
                static_cast<int>(slider.x + (slider.width - labelWidth) / 2.0f),
                static_cast<int>(slider.y + (slider.height - labelFontSize) / 2.0f),
                labelFontSize, labelColor);
    }

    if (gameOver)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.68f));
        const char* result = game.getWinner() == player ? "¡VICTORIA!" : "DERROTA";
        const Color resultColor = game.getWinner() == player ? GOLD : LIGHTGRAY;
        DrawText(result, (screenWidth - MeasureText(result, 54)) / 2,
                 screenHeight / 2 - 60, 54, resultColor);
        const char* restart = "Pulsa R para jugar otra vez";
        DrawText(restart, (screenWidth - MeasureText(restart, 22)) / 2,
                 screenHeight / 2 + 14, 22, RAYWHITE);
    }
    else if(winner != nullptr){
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.68f));
        const auto result = TextFormat("GANADOR: %s", winner->getName().c_str());
        DrawText(result, (screenWidth - MeasureText(result, 54)) / 2, screenHeight / 2 - 60, 54, GOLD);
        DrawText(result, (screenWidth - MeasureText(result, 54)) / 2,
                 screenHeight / 2 - 60, 54, GOLD);
        const char* restart = "Pulsa cualquier tecla para salir";
    }

    EndDrawing();
}

Rectangle MainGameState::getMapBounds() const
{
    const float availableWidth = static_cast<float>(std::max(1, GetScreenWidth() - 48));
    const float availableHeight = static_cast<float>(std::max(1, GetScreenHeight() - 212));
    const Map& map = game.getMap();
    const float mapColumns = static_cast<float>(map.getWidth());
    const float mapRows = static_cast<float>(map.getHeight());
    const float cellSize = std::min(availableWidth / mapColumns,
                                   availableHeight / mapRows);
    const float mapWidth = cellSize * mapColumns;
    const float mapHeight = cellSize * mapRows;

    return Rectangle{
        24.0f + (availableWidth - mapWidth) / 2.0f,
        124.0f + (availableHeight - mapHeight) / 2.0f,
        mapWidth,
        mapHeight
    };
}

Rectangle MainGameState::getAttackSliderBounds() const
{
    const float width = std::min(460.0f, static_cast<float>(GetScreenWidth()) - 96.0f);
    const float height = 34.0f;
    return Rectangle{
        (static_cast<float>(GetScreenWidth()) - width) / 2.0f,
        static_cast<float>(GetScreenHeight()) - 52.0f,
        width,
        height
    };
}

std::pair<int, int> MainGameState::getCellCoordinates(Vector2 position) const
{
    const Map& map = game.getMap();
    const Rectangle bounds = getMapBounds();
    if (position.x < bounds.x || position.y < bounds.y
        || position.x >= bounds.x + bounds.width
        || position.y >= bounds.y + bounds.height)
    {
        return {-1, -1};
    }

    const std::size_t column = static_cast<std::size_t>(
        (position.x - bounds.x) / bounds.width * map.getWidth());
    const std::size_t row = static_cast<std::size_t>(
        (position.y - bounds.y) / bounds.height * map.getHeight());

    return {column, row};
}