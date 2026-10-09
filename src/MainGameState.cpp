#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <Target.hpp>
#include <raylib.h>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>

namespace
{

Color cellColor(const TerritoryCell& cell)
{
    if (cell.owner != nullptr) return cell.owner->getColor();
    if (cell.owner == nullptr && cell.isWater) return Color{0,0,0,255};
    return Color{55, 63, 73, 255};
}
}

MainGameState::MainGameState(std::string playerName, Color playerColor)
    : game(std::move(playerName), playerColor)
{
}

void MainGameState::init()
{
    game.reset();
}

void MainGameState::handleInput()
{
    // Al finalizar la partida solo se acepta reiniciar
    if (game.isOver())
    {
        if (IsKeyPressed(KEY_R)) game.reset();
        return;
    }

    if (game.getPhase() == GamePhase::SPAWN) { // para bloquer ataques ... y capturar el spawn del jugador                                                                                      
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {                                                                                       
            const std::pair<int, int> targetCoordinates = getCellCoordinates(GetMousePosition());                                            
            const int column = targetCoordinates.first;                                                                                      
            const int row = targetCoordinates.second;                                                                                        
                                                                                                                                                
            if (column >= 0 && row >= 0) { // Si el clic es dentro del tablero                                                               
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
    // Las estadísticas del HUD se derivan del mapa actual para no duplicar estado.
    const int playerPercent = landCells == 0 ? 0
        : static_cast<int>(100.0 * static_cast<double>(playerCells) / landCells);
    const int botPercent = landCells == 0 ? 0
        : static_cast<int>(100.0 * static_cast<double>(botCells) / landCells);

    DrawRectangle(0, 0, screenWidth, 108, Color{31, 38, 47, 255});
    DrawText(player->getName().c_str(), 24, 12, 32, player->getColor());
    if (game.getPhase() == GamePhase::SPAWN) {
        // HUD para la fase SPAWN
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
        DrawText(TextFormat("%d%% territorio", playerPercent), 24, 51, 24, WHITE);
        DrawText(TextFormat("%d tropas", player->getTroops()),
                    24, 78, 22, WHITE);

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

            if (cell.owner == nullptr && cellWidth >= 20.0f)
            {
                const int troops = 0;// static_cast<int>(cell.troops);
                DrawText(TextFormat("%d", player->getTroops()),
                        static_cast<int>(cellBounds.x + 2),
                        static_cast<int>(cellBounds.y + 3),
                        14, RAYWHITE);
            }  


        }
    }

    if (game.getPhase() == GamePhase::SPAWN) {                                                                                               
        int previewIdx = game.getSpawnPreview();                                                                                             
        if (previewIdx >= 0) {                                                                                                               
            int cx = previewIdx % map.getWidth();                                                                                            
            int cy = previewIdx / map.getWidth();                                                                                            
                                                                                                                                                
            Color previewColor = player->getColor();                                                                                         
            previewColor.a = 150; // Hacerlo semitransparente                                                                                
                                                                                                                                                
            for (int dy = -1; dy <= 1; ++dy) {                                                                                               
                for (int dx = -1; dx <= 1; ++dx) {                                                                                           
                    int px = cx + dx;                                                                                                        
                    int py = cy + dy;                                                                                                        
                    if (px >= 0 && px < map.getWidth() && py >= 0 && py < map.getHeight()) {                                                 
                        const TerritoryCell& cell = map.getCell(px, py);
                        
                        // Solo pintamos el cuadrado si es tierra Y está libre
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

    EndDrawing();
}

Rectangle MainGameState::getMapBounds() const
{
    // Ajusta la cuadrícula al espacio disponible conservando celdas cuadradas.
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
    // Fuera del rectángulo del tablero no hay una celda seleccionable.
    if (position.x < bounds.x || position.y < bounds.y
        || position.x >= bounds.x + bounds.width
        || position.y >= bounds.y + bounds.height)
    {
        return {-1, -1};
    }

    // Escala las coordenadas de pantalla a índices de cuadrícula (fila y columna).
    // Es una transformación geométrica directa, no un algoritmo de búsqueda.
    const std::size_t column = static_cast<std::size_t>(
        (position.x - bounds.x) / bounds.width * map.getWidth());
    const std::size_t row = static_cast<std::size_t>(
        (position.y - bounds.y) / bounds.height * map.getHeight());

    return {column, row};
}