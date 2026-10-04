#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <raylib.h>
#include <algorithm>
#include <utility>

namespace
{
// Convierte la propiedad lógica de una celda en el color que se dibuja.
Color cellColor(Owner owner, Color playerColor, Color botColor)
{
    if (owner == Owner::Player) return playerColor;
    if (owner == Owner::Bot) return botColor;
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
    // Tras finalizar la partida solo se acepta reiniciar; se ignoran órdenes de ataque.
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
        const int targetIndex = getCellIndex(GetMousePosition());
        game.setPlayerTarget(targetIndex);
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
    const std::vector<TerritoryCell>& map = game.getMap();
    const Player& player = game.getPlayer();
    const Bot& bot = game.getBot();
    const bool gameOver = game.isOver();
    int playerCells = 0;
    int botCells = 0;
    float playerTroops = 0.0f;
    float botTroops = 0.0f;
    for (const TerritoryCell& cell : map)
    {
        if (cell.owner == Owner::Player)
        {
            ++playerCells;
            playerTroops += cell.troops;
        }
        else if (cell.owner == Owner::Bot)
        {
            ++botCells;
            botTroops += cell.troops;
        }
    }
    // Las estadísticas del HUD se derivan del mapa actual para no duplicar estado.
    const int playerPercent = playerCells * 100 / static_cast<int>(map.size());
    const int botPercent = botCells * 100 / static_cast<int>(map.size());

    DrawRectangle(0, 0, screenWidth, 108, Color{31, 38, 47, 255});
    DrawText(player.getName().c_str(), 24, 12, 32, player.getColor());
    DrawText(TextFormat("%d%% territorio", playerPercent), 24, 51, 24, WHITE);
    DrawText(TextFormat("%d tropas", static_cast<int>(playerTroops)),
             24, 78, 22, WHITE);

    const char* title = "CONQUISTA EL MAPA";
    DrawText(title, (screenWidth - MeasureText(title, 30)) / 2, 10, 30, RAYWHITE);
    const int totalSeconds = static_cast<int>(game.getTime());
    const std::string clock = TextFormat("%02d:%02d", totalSeconds / 60, totalSeconds % 60);
    DrawText(clock.c_str(), (screenWidth - MeasureText(clock.c_str(), 24)) / 2, 55, 24, LIGHTGRAY);

    const char* botName = "BOT";
    const int botNameWidth = MeasureText(botName, 32);
    DrawText(botName, screenWidth - botNameWidth - 24, 12, 32, bot.getColor());
    const std::string botStats = TextFormat("%d%% territorio", botPercent);
    DrawText(botStats.c_str(), screenWidth - MeasureText(botStats.c_str(), 24) - 24,
             51, 24, WHITE);
    const std::string botTroopStats = TextFormat("%d tropas", static_cast<int>(botTroops));
    DrawText(botTroopStats.c_str(),
             screenWidth - MeasureText(botTroopStats.c_str(), 22) - 24,
             78, 22, WHITE);

    const Rectangle bounds = getMapBounds();
    DrawRectangleRec(bounds, Color{38, 45, 54, 255});
    const float cellWidth = bounds.width / Game::MAP_COLUMNS;
    const float cellHeight = bounds.height / Game::MAP_ROWS;
    const Vector2 mousePosition = GetMousePosition();
    const int hoveredCell = getCellIndex(mousePosition);

    // El vector del mapa está en orden por filas: índice = fila * columnas + columna.
    for (int i = 0; i < static_cast<int>(map.size()); ++i)
    {
        const int column = i % Game::MAP_COLUMNS;
        const int row = i / Game::MAP_COLUMNS;
        const Rectangle cellBounds{
            bounds.x + column * cellWidth,
            bounds.y + row * cellHeight,
            cellWidth,
            cellHeight
        };
        const TerritoryCell& cell = map[i];
        DrawRectangleRec(cellBounds, cellColor(cell.owner, player.getColor(), bot.getColor()));
        const Color ownershipColor = cell.owner == Owner::Player ? player.getColor()
            : cell.owner == Owner::Bot ? bot.getColor() : Color{31, 37, 45, 255};
        const float borderThickness = cell.capture_protection > 0.0f ? 3.0f
            : cell.owner == Owner::Land ? 0.7f : 1.5f;
        DrawRectangleLinesEx(cellBounds, borderThickness,
                             ownershipColor);

        if (cell.is_base)
        {
            DrawRectangleLinesEx(cellBounds, 2.0f, RAYWHITE);
        }
        else if (i == player.getTargetIndex() && !gameOver)
        {
            DrawRectangleLinesEx(cellBounds, 3.0f, GOLD);
        }
        else if (i == hoveredCell && cell.owner != Owner::Player && !gameOver)
        {
            DrawRectangleLinesEx(cellBounds, 2.0f, GOLD);
        }

        if (cell.owner != Owner::Land
            && cell.troops >= 10.0f && cellWidth >= 20.0f)
        {
            const int troops = static_cast<int>(cell.troops);
            DrawText(TextFormat("%d", troops),
                     static_cast<int>(cellBounds.x + 2),
                     static_cast<int>(cellBounds.y + 3),
                     14, RAYWHITE);
        }
    }

    DrawRectangle(0, screenHeight - 64, screenWidth, 64, Color{31, 38, 47, 255});
    const char* instructions = "Clic para avanzar hasta el territorio  |  Clic derecho o ESC para cancelar";
    DrawText(instructions, 24, screenHeight - 41, 21, LIGHTGRAY);
    const char* objective = "Captura la base rival para ganar";
    DrawText(objective, screenWidth - MeasureText(objective, 21) - 24,
             screenHeight - 41, 21, GOLD);

    if (gameOver)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.68f));
        const char* result = game.getWinner() == Owner::Player ? "¡VICTORIA!" : "DERROTA";
        const Color resultColor = game.getWinner() == Owner::Player ? GOLD : LIGHTGRAY;
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
    // Reserva espacio para la cabecera y el pie de instrucciones.
    return Rectangle{
        24.0f,
        124.0f,
        static_cast<float>(std::max(1, GetScreenWidth() - 48)),
        static_cast<float>(std::max(1, GetScreenHeight() - 212))
    };
}

int MainGameState::getCellIndex(Vector2 position) const
{
    const Rectangle bounds = getMapBounds();
    // Fuera del rectángulo del tablero no hay una celda seleccionable.
    if (position.x < bounds.x || position.y < bounds.y
        || position.x >= bounds.x + bounds.width
        || position.y >= bounds.y + bounds.height)
    {
        return -1;
    }

    // Escala las coordenadas de pantalla a índices de cuadrícula (fila y columna).
    // Es una transformación geométrica directa, no un algoritmo de búsqueda.
    const int column = static_cast<int>((position.x - bounds.x) / bounds.width * Game::MAP_COLUMNS);
    const int row = static_cast<int>((position.y - bounds.y) / bounds.height * Game::MAP_ROWS);
    return row * Game::MAP_COLUMNS + column;
}