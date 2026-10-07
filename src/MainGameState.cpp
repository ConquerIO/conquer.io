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

        // El agua no es un objetivo valido: el clic se ignora.
        if (cell.isWater) return;

        if (cell.owner == nullptr)
        {
            game.setPlayerTarget(Target::land());
        }
        else if (cell.owner != game.getPlayer())
        {
            game.setPlayerTarget(Target::enemy(cell.owner));
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
    float playerTroops = 0.0f;
    float botTroops = 0.0f;
    for (const TerritoryCell& cell : map.getCells())
    {
        if (!cell.isWater && cell.owner == nullptr) ++landCells;
        if (cell.owner == player)
        {
            ++playerCells;
        }
        else if (!cell.isWater && cell.owner != nullptr) 
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
    DrawText(TextFormat("%d%% territorio", playerPercent), 24, 51, 24, WHITE);
    DrawText(TextFormat("%d tropas", static_cast<int>(playerTroops)),
             24, 78, 22, WHITE);

    const char* title = "CONQUISTA EL MAPA";
    DrawText(title, (screenWidth - MeasureText(title, 30)) / 2, 10, 30, RAYWHITE);
    const int totalSeconds = static_cast<int>(game.getTime());
    const std::string clock = TextFormat("%02d:%02d", totalSeconds / 60, totalSeconds % 60);
    DrawText(clock.c_str(), (screenWidth - MeasureText(clock.c_str(), 24)) / 2, 55, 24, LIGHTGRAY);

    const Rectangle bounds = getMapBounds();
    DrawRectangleRec(bounds, Color{38, 45, 54, 255});
    const float cellWidth = bounds.width / static_cast<float>(map.getWidth());
    const float cellHeight = bounds.height / static_cast<float>(map.getHeight());
    const Vector2 mousePosition = GetMousePosition();
    //const std::pair<int, int> hoveredCell = getCellCoordinates(mousePosition);

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
                DrawText(TextFormat("%d", playerTroops /* player.troops*/),
                        static_cast<int>(cellBounds.x + 2),
                        static_cast<int>(cellBounds.y + 3),
                        14, RAYWHITE);
            }  


        }
    }

    DrawRectangle(0, screenHeight - 64, screenWidth, 64, Color{31, 38, 47, 255});
    const char* instructions = "Clic en terreno neutral para expandirte | Clic derecho o ESC para cancelar";
    DrawText(instructions, 24, screenHeight - 41, 21, LIGHTGRAY);
    const char* objective = "Expande tu territorio por las zonas neutrales";
    DrawText(objective, screenWidth - MeasureText(objective, 21) - 24,
             screenHeight - 41, 21, GOLD);

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