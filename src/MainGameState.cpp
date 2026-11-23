#include <MainGameState.hpp>
#include <MenuState.hpp>
#include <StateMachine.hpp>
#include <MenuState.hpp>
#include <Target.hpp>
#include <raylib.h>
#include "rlImGui.h"
#include "system/AudioManager.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{

// Segundos en la pantalla de Game Over antes de aceptar teclas para volver al menú.
constexpr float GAME_OVER_INPUT_DELAY = 1.0f;

constexpr float CAMERA_ZOOM_STEP = 1.15f;
constexpr float CAMERA_MIN_ZOOM = 0.5f;
constexpr float CAMERA_MAX_ZOOM = 10.0f;
constexpr float CAMERA_DRAG_THRESHOLD = 4.0f;

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

//icono de alianza encima del nombre
constexpr float ALLY_ICON_SIZE = 14.0f;
constexpr float ALLY_ICON_GAP = 2.0f;

//esto lo que hace es la etiqueta de la tropas y nombres para mostrarla
struct TerritoryLabel{
    Entity* owner = nullptr;
    std::string name;
    std::string troops;
    Rectangle rect{};
    Rectangle iconRect{};
    std::size_t cells = 0; 
    int nameFontSize = 0;
    int troopsFontSize = 0;
    bool visible = false;
    bool allied = false;
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


constexpr double STATS_REFRESH = 0.1;

struct StatsCache{
    std::unordered_map<Entity*, TerritoryStats> stats;
    unsigned generation = ~0u;
    unsigned version = ~0u;
    double time = 0.0;
};

//icono de dos manos estrechandose
void drawHandshakeIcon(float cx, float cy, float size, Color color){
    const Color outline{20, 24, 30, 255};

    const Vector2 leftStart= { cx - size * 0.50f, cy - size * 0.15f };
    const Vector2 rightStart = { cx + size * 0.50f, cy - size * 0.15f };
    const Vector2 center= { cx, cy + size * 0.15f };

    DrawLineEx(leftStart, center, std::max(2.0f, size * 0.28f), outline);
    DrawLineEx(rightStart, center, std::max(2.0f, size * 0.28f), outline);
    DrawCircleV(center, size * 0.22f, outline);

    DrawLineEx(leftStart, center, std::max(1.0f, size * 0.14f), color);
    DrawLineEx(rightStart, center, std::max(1.0f, size * 0.14f), color);
    DrawCircleV(center, size * 0.14f, color);
}

//esta lo que haces es ir calculando el tamaño del territiorio para ajustar el texto cada vez que se aumenta o disminuye el territorio
TerritoryLabel buildLabel(const Map& map, Entity* owner, const TerritoryStats& stats, const Camera2D& camera, bool isAlly){
    TerritoryLabel label;
    label.owner = owner;
    label.cells = stats.cells;
    label.allied = isAlly;

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
    const Vector2 boxTopLeft = GetWorldToScreen2D(
        Vector2{static_cast<float>(stats.minColumn), static_cast<float>(stats.minRow)}, camera);
    const Vector2 boxBottomRight = GetWorldToScreen2D(
        Vector2{static_cast<float>(stats.maxColumn + 1), static_cast<float>(stats.maxRow + 1)}, camera);

    //este es el rectangulo donde se muestran el nombre y las tropas
    const Rectangle box{
        boxTopLeft.x,
        boxTopLeft.y,
        boxBottomRight.x - boxTopLeft.x,
        boxBottomRight.y - boxTopLeft.y
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

    const float iconSize = isAlly ? ALLY_ICON_SIZE : 0.0f;
    const float iconGap = isAlly ? ALLY_ICON_GAP : 0.0f;

    float textHeight = static_cast<float>(nameFontSize + troopsFontSize) + gap;
    float blockHeight = textHeight + iconSize + iconGap;

    //si es mas alto que el bloque donde se mete, se reduce 
    const float maxBlockHeight = box.height * LABEL_HEIGHT_FILL;
    const float maxTextHeight = std::max(0.0f, maxBlockHeight - iconSize - iconGap);

    if (textHeight > maxTextHeight && textHeight > 0.0f){
        const float scale = maxTextHeight / textHeight;
        nameFontSize = static_cast<int>(nameFontSize * scale);
        troopsFontSize = static_cast<int>(nameFontSize * LABEL_TROOPS_SCALE);
        gap = nameFontSize * LABEL_LINE_GAP;
        textHeight = static_cast<float>(nameFontSize + troopsFontSize) + gap;
        blockHeight = textHeight + iconSize + iconGap;
    }

    //si es muy pequeño no se muestra
    if (nameFontSize < LABEL_MIN_FONT) {
        return label;
    }

    const int textWidth =std::max(MeasureText(label.name.c_str(), nameFontSize),MeasureText(label.troops.c_str(), troopsFontSize));
    const float labelWidth = std::max(static_cast<float>(textWidth), iconSize);

    const Vector2 anchor = GetWorldToScreen2D(
        Vector2{static_cast<float>(anchorColumn) + 0.5f, static_cast<float>(anchorRow) + 0.5f}, camera);
    const float anchorX = anchor.x;
    const float anchorY = anchor.y;

    //lo centra y lo marca visible
    label.nameFontSize = nameFontSize;
    label.troopsFontSize = troopsFontSize;
    label.rect = Rectangle{
        anchorX - labelWidth / 2.0f,
        anchorY - blockHeight / 2.0f,
        labelWidth,
        blockHeight
    };

    if (isAlly) {
        label.iconRect = Rectangle{
            label.rect.x + (label.rect.width - iconSize) / 2.0f,
            label.rect.y,
            iconSize,
            iconSize
        };
    }

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

    float textTop = label.rect.y;

    if (label.allied) {
        drawHandshakeIcon(label.iconRect.x + label.iconRect.width / 2.0f,label.iconRect.y + label.iconRect.height / 2.0f,label.iconRect.width,GOLD);
        textTop += label.iconRect.height + ALLY_ICON_GAP;
    }
    DrawText(label.name.c_str(),static_cast<int>(label.rect.x + (label.rect.width - nameWidth) / 2.0f),static_cast<int>(textTop),label.nameFontSize, color);
    DrawText(label.troops.c_str(),static_cast<int>(label.rect.x + (label.rect.width - troopsWidth) / 2.0f),static_cast<int>(textTop + label.nameFontSize + gap),label.troopsFontSize, color);
}

//dibuja la corona del primero en la clasificacion
void drawCrownIcon(float x, float y, float width, float height, Color color){
    auto drawShape = [](float sx, float sy, float sw, float sh, Color c){
        DrawRectangle(static_cast<int>(sx),static_cast<int>(sy + sh * 0.60f),static_cast<int>(sw),static_cast<int>(sh * 0.35f),c);
        DrawTriangle(Vector2{sx, sy + sh * 0.60f},Vector2{sx + sw * 0.25f, sy},Vector2{sx + sw * 0.50f, sy + sh * 0.60f},c);
        DrawTriangle(Vector2{sx + sw * 0.25f, sy + sh * 0.60f},Vector2{sx + sw * 0.50f, sy},Vector2{sx + sw * 0.75f, sy + sh * 0.60f},c);
        DrawTriangle(Vector2{sx + sw * 0.50f, sy + sh * 0.60f},Vector2{sx + sw * 0.75f, sy},Vector2{sx + sw, sy + sh * 0.60f},c);
    };
    drawShape(x - 1.0f, y - 1.0f, width + 2.0f, height + 2.0f,Color{40, 32, 10, 255});
    drawShape(x, y, width, height, color);
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

MainGameState::~MainGameState()
{
    if (map_texture_ready && IsWindowReady())
    {
        UnloadRenderTexture(map_texture);
    }
}

void MainGameState::init()
{
    game.reset();
    if (map_texture_ready && IsWindowReady())
    {
        UnloadRenderTexture(map_texture);
    }
    map_texture_ready = false;
    changed_cells_buffer.clear();
    camera_zoom = 1.0f;
    camera_pan = Vector2{0.0f, 0.0f};
    dragging_camera = false;
    camera_drag_distance = 0.0f;
    context_menu.close();
    end_sound_played = false;
    AudioManager::play(Sfx::GameStart);
}

void MainGameState::handleInput()
{
    if (IsKeyPressed(KEY_F1)) {
        debug_overlay.toggle();
    }

    if (game.isOver())
    {
        // Margen inicial para que una tecla o clic ya pulsado al perder no salte la pantalla.
        if (game_over_timer < GAME_OVER_INPUT_DELAY) return;

        if (GetKeyPressed() != 0
            || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            // Vuelve al menú inicial; la partida actual se destruye al reemplazar el estado.
            state_machine->add_state(std::make_unique<MenuState>(), true);
        }
        return;
    }

    if(game.getWinner() != nullptr) {
        if (GetCharPressed() != 0) {
            this->state_machine->add_state(std::make_unique<MenuState>(), true);
        }
    }

    // Si hay una solicitud de alianza entrante, sus botones tienen prioridad.
    if (handleAlliancePanelInput()) {
        return;
    }

    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f)
    {
        camera_zoom *= std::pow(CAMERA_ZOOM_STEP, wheel);
        camera_zoom = std::clamp(camera_zoom, CAMERA_MIN_ZOOM, CAMERA_MAX_ZOOM);
    }

    if (!context_menu.isOpen() && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        dragging_camera = true;
        camera_drag_distance = 0.0f;
    }

    if (dragging_camera && IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        const Vector2 delta = GetMouseDelta();
        const float zoom = buildMapCamera().zoom;
        if (zoom > 0.0f)
        {
            camera_pan.x -= delta.x / zoom;
            camera_pan.y -= delta.y / zoom;
        }
        camera_drag_distance += std::sqrt(delta.x * delta.x + delta.y * delta.y);
    }

    if (dragging_camera && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
    {
        dragging_camera = false;
        if (camera_drag_distance < CAMERA_DRAG_THRESHOLD)
        {
            game.cancelPlayerAttack();
        }
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

    if (context_menu.isOpen()){
        const int index = context_menu.getCellIndex();
        const int width = static_cast<int>(game.getMap().getWidth());

        const int column = index % width;
        const int row = index / width;

        const TerritoryCell& cell = game.getMap().getCell(column, row);
        Entity* targetEntity = cell.owner;
        Player* player = game.getPlayer();
        const AllianceManager& alliances = game.getAlliances();

        const bool alreadyAllied =targetEntity != nullptr && alliances.areAllied(player, targetEntity);
        const bool pending =targetEntity != nullptr && alliances.hasPendingFrom(player, targetEntity);
        const float cooldown =targetEntity != nullptr? alliances.cooldownRemaining(player, targetEntity, game.getTime()): 0.0f;
        const bool canAlly =targetEntity != nullptr&& targetEntity != player&& !alreadyAllied&& !pending&& cooldown <= 0.0f;

        //la donacion es lo mismo destinado al ataque
        const std::size_t donadorTroops = player->getTroops();
        const std::size_t donation = static_cast<std::size_t>(static_cast<float>(donadorTroops)* attack_percent / 100.0f);

        const bool canAttack =(attack_percent > 0) && isCellAttackable(column, row);
        const bool canDonate =cell.owner != nullptr && cell.owner != player&& donadorTroops > 1 && donation > 0;

        const ContextMenu::Action action =context_menu.handleInput(canAttack, canDonate, canAlly);

        if (action == ContextMenu::Action::Attack){
            executeContextAttack();
        }
        else if(action==ContextMenu::Action::Donate){
            executeContextDonation();
        }
        else if (action == ContextMenu::Action::Alliance){
            executeContextAlliance();
        }

        return;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        game.cancelPlayerAttack();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const std::pair<int, int> targetCoordinates = getCellCoordinates(GetMousePosition());
        const int column = targetCoordinates.first;
        const int row = targetCoordinates.second;
        if (column < 0 || row < 0) return;

        const TerritoryCell& cell = game.getMap().getCell(
            static_cast<std::size_t>(column), static_cast<std::size_t>(row));

        if (cell.isWater) return;
        //para que no se abra si se le da a él mismo
        if(cell.owner==game.getPlayer()){
            return;
        }

        //para la tierra normal nos expandimos solo con el click
        if (cell.owner == nullptr){
            if (!playerHasNeutralFrontier()){
                return;
            }
            const float ratio =static_cast<float>(attack_percent) / 100.0f;
            game.setPlayerTarget(Target::land(), ratio);
            AudioManager::play(Sfx::Attack);
            return;
        }

        const int index =column + row * static_cast<int>(game.getMap().getWidth());
        context_menu.open(index, GetMousePosition(),GetScreenWidth(), GetScreenHeight());
        AudioManager::play(Sfx::ContextMenu);
        
        return;
    }
}

bool MainGameState::playerHasNeutralFrontier() const
{
    const Map& map = game.getMap();
    const Entity* player = game.getPlayer();

    const int width = static_cast<int>(map.getWidth());
    const int height = static_cast<int>(map.getHeight());

    const int dx[4] = { 0, 0, -1, 1 };
    const int dy[4] = { -1, 1, 0, 0 };

    for (const auto& cell : player->getFrontier()){
        for (int i = 0; i < 4; ++i){
            const int nx = cell.first + dx[i];
            const int ny = cell.second + dy[i];

            if (nx < 0 || ny < 0 || nx >= width || ny >= height){
                continue;
            }

            const TerritoryCell& neighbor = map.getCell(nx, ny);

            if (!neighbor.isWater && neighbor.owner == nullptr){
                return true;
            }
        }
    }

    return false;
}

bool MainGameState::playerIsAdjacentTo(Entity* enemy) const
{
    if (enemy == nullptr) {
        return false;
    }

    const Map& map = game.getMap();
    const Entity* player = game.getPlayer();

    const int width = static_cast<int>(map.getWidth());
    const int height = static_cast<int>(map.getHeight());

    const int dx[4] = { 0, 0, -1, 1 };
    const int dy[4] = { -1, 1, 0, 0 };

    for (const auto& cell : player->getFrontier()){
        for (int i = 0; i < 4; ++i){
            const int nx = cell.first + dx[i];
            const int ny = cell.second + dy[i];

            if (nx < 0 || ny < 0 || nx >= width || ny >= height){
                continue;
            }

            if (map.getCell(nx, ny).owner == enemy){
                return true;
            }
        }
    }

    return false;
}

bool MainGameState::isCellAttackable(int column, int row) const
{
    const Map& map = game.getMap();
    const int width = static_cast<int>(map.getWidth());
    const int height = static_cast<int>(map.getHeight());

    if (column < 0 || row < 0 || column >= width || row >= height){
        return false;
    }

    const TerritoryCell& cell = map.getCell(column, row);
    const Entity* player = game.getPlayer();

    if (cell.isWater || cell.owner == player){
        return false;
    }

    //tierra
    if (cell.owner == nullptr){
        return playerHasNeutralFrontier();
    }

    //enemigo
    return playerIsAdjacentTo(cell.owner);
}

void MainGameState::executeContextDonation(){
    const int index = context_menu.getCellIndex();
    if (index < 0) {
        context_menu.close();
        return;
    }

    const int width = static_cast<int>(game.getMap().getWidth());
    const int column = index % width;
    const int row = index / width;

    const TerritoryCell& cell = game.getMap().getCell(column, row);

    Player* donador = game.getPlayer();
    Entity* receiver = cell.owner;

    if (receiver == nullptr || receiver == donador) {
        context_menu.close();
        return;
    }

    //la cantidad donada es la misma destinada al ataque
    const std::size_t donadorTroops = donador->getTroops();
    const std::size_t amount = static_cast<std::size_t>(static_cast<float>(donadorTroops)* attack_percent / 100.0f);

    //con una tropa no se puede donar para no morir
    if (donadorTroops <= 1 || amount == 0 || amount > donadorTroops) {
        context_menu.close();
        return;
    }

    donador->setTroops(donador->getTroops() - amount);
    //no se limita al máximo ni se redondea
    receiver->setTroops(receiver->getTroops() + amount);
    context_menu.close();
}

void MainGameState::executeContextAlliance(){
    const int index = context_menu.getCellIndex();
    if (index < 0) {
        context_menu.close();
        return;
    }

    const int width = static_cast<int>(game.getMap().getWidth());
    const int column = index % width;
    const int row = index / width;

    const TerritoryCell& cell = game.getMap().getCell(column, row);

    Player* player = game.getPlayer();
    Entity* target = cell.owner;

    if (target == nullptr || target == player) {
        context_menu.close();
        return;
    }

    //un bot tarda aprox 1 seg en responder
    const bool sent = game.getAlliances().sendRequest(
        player,
        target,
        game.getTime(),
        game.getTime() + 1.0f);

    if (sent) {
        showAllianceToast("Solicitud enviada a " + target->getName());
    }

    context_menu.close();
}

void MainGameState::updateAlliancePanelLayout(){
    constexpr float panelW = 330.0f;
    constexpr float panelH = 112.0f;
    constexpr float margin = 24.0f;

    const float x = std::max(8.0f,static_cast<float>(GetScreenWidth()) - panelW - margin);
    const float y = 120.0f;

    alliance_panel_rect = Rectangle{ x, y, panelW, panelH };

    constexpr float buttonW = 138.0f;
    constexpr float buttonH = 28.0f;
    const float buttonY = y + 48.0f;

    alliance_accept_rect =Rectangle{ x + 14.0f, buttonY, buttonW, buttonH };
    alliance_reject_rect =Rectangle{ x + panelW - 14.0f - buttonW, buttonY, buttonW, buttonH };
}

bool MainGameState::handleAlliancePanelInput(){
    Player* player = game.getPlayer();
    Entity* requester = nullptr;
    float expiresAt = 0.0f;

    if (!game.getAlliances().firstPendingTo(player, requester, expiresAt)){
        return false;
    }

    updateAlliancePanelLayout();

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        return false;
    }

    const Vector2 mouse = GetMousePosition();

    if (CheckCollisionPointRec(mouse, alliance_accept_rect)){
        game.getAlliances().accept(player, requester, game.getTime());
        showAllianceToast("Alianza aceptada con " + requester->getName());
        return true;
    }

    if (CheckCollisionPointRec(mouse, alliance_reject_rect)){
        game.getAlliances().reject(player, requester, game.getTime());
        showAllianceToast("Alianza rechazada con " + requester->getName());
        return true;
    }

    //ell click dentro del panel no afecta a la jugabilidad
    if (CheckCollisionPointRec(mouse, alliance_panel_rect)){
        return true;
    }

    return false;
}

void MainGameState::renderAlliancePanel()
{
    Player* player = game.getPlayer();
    Entity* requester = nullptr;
    float expiresAt = 0.0f;

    if (!game.getAlliances().firstPendingTo(player, requester, expiresAt)){
        return;
    }

    updateAlliancePanelLayout();

    const Vector2 mouse = GetMousePosition();
    const float remaining = std::max(0.0f, expiresAt - game.getTime());
    const float ratio = std::max(0.0f, std::min(1.0f, remaining / 15.0f));
    const std::size_t pending = game.getAlliances().countPendingTo(player);

    //panel
    DrawRectangleRounded(Rectangle{ alliance_panel_rect.x + 2.0f,alliance_panel_rect.y + 2.0f,alliance_panel_rect.width,alliance_panel_rect.height },0.08f, 8, Fade(BLACK, 0.35f));
    DrawRectangleRounded(alliance_panel_rect, 0.08f, 8, Color{30, 34, 42, 245});
    DrawRectangleRoundedLines(alliance_panel_rect, 0.08f, 8, GRAY);

    //texto del panel
    DrawText(requester->getName().c_str(),static_cast<int>(alliance_panel_rect.x + 14.0f),static_cast<int>(alliance_panel_rect.y + 10.0f),18, RAYWHITE);
    DrawText("quiere aliarse contigo",static_cast<int>(alliance_panel_rect.x + 14.0f),static_cast<int>(alliance_panel_rect.y + 30.0f),14, LIGHTGRAY);
    DrawText(TextFormat("%.0f s", remaining),static_cast<int>(alliance_panel_rect.x + alliance_panel_rect.width - 52.0f),static_cast<int>(alliance_panel_rect.y + 30.0f),14, LIGHTGRAY);

    //botones
    const bool hoverAccept =CheckCollisionPointRec(mouse, alliance_accept_rect);
    const bool hoverReject =CheckCollisionPointRec(mouse, alliance_reject_rect);
    DrawRectangleRec(alliance_accept_rect,hoverAccept ? Color{45, 120, 60, 255} : Color{40, 70, 50, 255});
    DrawRectangleLinesEx(alliance_accept_rect, 1.0f, GRAY);
    const char* acceptText = "Aceptar";
    DrawText(acceptText,static_cast<int>(alliance_accept_rect.x+ (alliance_accept_rect.width- MeasureText(acceptText, 16)) / 2.0f),static_cast<int>(alliance_accept_rect.y + 6.0f),16, RAYWHITE);
    DrawRectangleRec(alliance_reject_rect,hoverReject ? Color{150, 55, 55, 255} : Color{80, 42, 42, 255});
    DrawRectangleLinesEx(alliance_reject_rect, 1.0f, GRAY);
    const char* rejectText = "Rechazar";
    DrawText(rejectText,static_cast<int>(alliance_reject_rect.x+ (alliance_reject_rect.width- MeasureText(rejectText, 16)) / 2.0f),static_cast<int>(alliance_reject_rect.y + 6.0f),16, RAYWHITE);

    //tiempo
    const Rectangle bar{
        alliance_panel_rect.x + 14.0f,
        alliance_panel_rect.y + 86.0f,
        alliance_panel_rect.width - 28.0f,
        6.0f
    };

    DrawRectangleRec(bar, Color{20, 24, 30, 255});

    Rectangle fill = bar;
    fill.width *= ratio;
    DrawRectangleRec(fill, Color{220, 180, 60, 255});

    //cola de solicitudes
    if (pending > 1){
        DrawText(TextFormat("Pendientes: %d", static_cast<int>(pending)),static_cast<int>(alliance_panel_rect.x + 14.0f),static_cast<int>(alliance_panel_rect.y + 96.0f),12, GRAY);
    }
}

void MainGameState::showAllianceToast(const std::string& message){
    alliance_toast = message;
    alliance_toast_timer = 3.0f;
}

//para dibujar el mensaje de aviso
void MainGameState::renderAllianceToast()
{
    if (alliance_toast_timer <= 0.0f || alliance_toast.empty()) {
        return;
    }

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    const int fontSize = 18;
    const int textWidth = MeasureText(alliance_toast.c_str(), fontSize);

    const float width = static_cast<float>(textWidth + 28);
    const float height = 34.0f;

    const Rectangle rect{
        (static_cast<float>(screenWidth) - width) / 2.0f,
        static_cast<float>(screenHeight) - 110.0f,
        width,
        height
    };

    const float alpha = std::max(0.0f, std::min(1.0f, alliance_toast_timer));

    DrawRectangleRounded(rect, 0.25f, 8, Fade(BLACK, 0.65f * alpha));
    DrawRectangleRoundedLines(rect, 0.25f, 8, Fade(GRAY, alpha));
    DrawText(alliance_toast.c_str(),static_cast<int>(rect.x + (rect.width - textWidth) / 2.0f),static_cast<int>(rect.y + (rect.height - fontSize) / 2.0f),fontSize,Fade(RAYWHITE, alpha));
}

void MainGameState::executeContextAttack()
{
    const int index = context_menu.getCellIndex();
    if (index < 0) {
        context_menu.close();
        return;
    }

    const int width = static_cast<int>(game.getMap().getWidth());
    const int column = index % width;
    const int row = index / width;

    if (attack_percent <= 0 || !isCellAttackable(column, row)){
        context_menu.close();
        return;
    }

    const TerritoryCell& cell = game.getMap().getCell(column, row);
    const float ratio = static_cast<float>(attack_percent) / 100.0f;

    //si ya hay un ataque en curso se cancela
    if (!game.getPlayer()->getTarget().isNone()){
        game.cancelPlayerAttack();
    }

    if (cell.owner == nullptr){
        game.setPlayerTarget(Target::land(), ratio);
        AudioManager::play(Sfx::Attack);
    }
    else if (cell.owner != game.getPlayer()){
        game.setPlayerTarget(Target::enemy(cell.owner), ratio);
        AudioManager::play(Sfx::Attack);
    }

    context_menu.close();
}

void MainGameState::update(float deltaTime)
{
    game.update(deltaTime);
    if (game.isOver()) game_over_timer += deltaTime;

    //avisosde alianza que se generan en el main
    std::vector<Game::AllianceEvent> events;
    game.takeAllianceEvents(events);

    for (const Game::AllianceEvent& event : events){
        switch (event.type){
            case Game::AllianceEvent::Type::Accepted:
                showAllianceToast(event.otherName + " ha aceptado la alianza");
                break;
            case Game::AllianceEvent::Type::Rejected:
                showAllianceToast(event.otherName + " ha rechazado la alianza");
                break;
            case Game::AllianceEvent::Type::Broken:
                showAllianceToast("Alianza rota con " + event.otherName);
                break;
        }
    }

    if (alliance_toast_timer > 0.0f)
    {
        alliance_toast_timer = std::max(0.0f, alliance_toast_timer - deltaTime);
    }

    //el sonido de fin de partida suena una sola vez al aparecer la pantalla
    if (!end_sound_played) {
        if (game.isOver()) {
            AudioManager::play(Sfx::GameOver);
            end_sound_played = true;
        } else if (game.getWinner() != nullptr) {
            AudioManager::play(Sfx::Winner);
            end_sound_played = true;
        }
    }
}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(Color{24, 29, 36, 255});

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const Map& map = game.getMap();
    Player* player = game.getPlayer();
    //Bot* bot = (Bot* )game.getEntities()[1];
    const bool gameOver = game.isOver();
    const auto winner = game.getWinner();
    renderMap();

    {
        const std::vector<Entity*> entities = game.getEntities();
        // Las estadísticas (centroide/tamaño de etiqueta) solo se recalculan si el
        // mapa ha cambiado y como mucho cada STATS_REFRESH segundos.
        static StatsCache cache;
        const double now = GetTime();
        if (cache.generation != game.getMapGeneration()
            || (cache.version != game.getMapVersion() && now - cache.time >= STATS_REFRESH))
        {
            cache.stats = collectTerritoryStats(map, entities);
            cache.generation = game.getMapGeneration();
            cache.version = game.getMapVersion();
            cache.time = now;
        }
        const std::unordered_map<Entity*, TerritoryStats>& stats = cache.stats;

        std::vector<TerritoryLabel> labels;
        labels.reserve(entities.size());
        const TerritoryStats empty;
        for (Entity* entity : entities)
        {
            const auto it = stats.find(entity);
            const bool isAlly = game.getAlliances().areAllied(player, entity);
            labels.push_back(buildLabel(map, entity,it != stats.end() ? it->second : empty,map_camera, isAlly));
        }

        // Solo se muestran las etiquetas que caen dentro de la pantalla.
        const Rectangle screenBounds{
            0.0f, 0.0f,
            static_cast<float>(screenWidth),
            static_cast<float>(screenHeight)
        };
        for (TerritoryLabel& label : labels)
        {
            if (label.visible && !CheckCollisionRecs(label.rect, screenBounds))
            {
                label.visible = false;
            }
        }

        separateLabels(labels, screenBounds);
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

            BeginMode2D(map_camera);
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int px = cx + dx;
                    int py = cy + dy;
                    if (px >= 0 && px < map.getWidth() && py >= 0 && py < map.getHeight()) {
                        const TerritoryCell& cell = map.getCell(px, py);

                        if (!cell.isWater && cell.owner == nullptr) {
                            const Rectangle cellBounds{
                                static_cast<float>(px),
                                static_cast<float>(py),
                                1.0f,
                                1.0f
                            };
                            DrawRectangleRec(cellBounds, previewColor);
                        }
                    }
                }
            }
            EndMode2D();
        }
    }

    // El HUD se dibuja después del mapa para quedar siempre por encima de la cámara.
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

   
    if (game.getPhase() == GamePhase::PLAYING && !gameOver && winner == nullptr && context_menu.isOpen()){
        const int index = context_menu.getCellIndex();
        const int width = static_cast<int>(map.getWidth());
        const int column = index % width;
        const int row = index / width;

        if (column >= 0 && row >= 0 && column < static_cast<int>(map.getWidth()) && row < static_cast<int>(map.getHeight())){
            const TerritoryCell& cell = map.getCell(column, row);
            Entity* targetEntity = cell.owner;
            std::string title =(targetEntity == nullptr) ? "Tierra": targetEntity->getName();

            const AllianceManager& alliances = game.getAlliances();
            const bool alreadyAllied =targetEntity != nullptr && alliances.areAllied(player, targetEntity);
            const bool pending =targetEntity != nullptr && alliances.hasPendingFrom(player, targetEntity);
            const float cooldown =targetEntity != nullptr? alliances.cooldownRemaining(player, targetEntity, game.getTime()): 0.0f;

            const bool canAlly =targetEntity != nullptr&& targetEntity != player&& !alreadyAllied&& !pending&& cooldown <= 0.0f;

            std::string allyLabel = "Alianza";
            if (alreadyAllied) {
                allyLabel = "Ya aliados";
            }
            else if (pending) {
                allyLabel = "Solicitud enviada";
            }
            else if (cooldown > 0.0f){
                allyLabel = TextFormat("Espera %d s", static_cast<int>(std::ceil(cooldown)));
            }

            const std::size_t donadorTroops = player->getTroops();
            const std::size_t donation = static_cast<std::size_t>(static_cast<float>(donadorTroops)* attack_percent / 100.0f);
            //con una tropa no se puede donar para no morir
            const bool canDonate =cell.owner != nullptr && cell.owner != player&& donadorTroops > 1 && donation > 0;
            const bool canAttack =(attack_percent > 0) && isCellAttackable(column, row);
            const int attackTroops = static_cast<int>(static_cast<float>(player->getTroops()) * attack_percent / 100.0f);
            context_menu.render(title.c_str(),attack_percent,attackTroops,canAttack,canDonate ? static_cast<int>(donation) : 0,canAlly, allyLabel.c_str());
        }
    }

    renderAlliancePanel();
    renderAllianceToast();

    if (gameOver)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));

        const char* title = "GAME OVER";
        const int titleSize = 96;
        DrawText(title, (screenWidth - MeasureText(title, titleSize)) / 2,
                 screenHeight / 2 - 120, titleSize, Color{235, 65, 75, 255});

        const int seconds = static_cast<int>(game.getTime());
        const char* survived = TextFormat("Has sobrevivido %02d:%02d",
                                          seconds / 60, seconds % 60);
        DrawText(survived, (screenWidth - MeasureText(survived, 30)) / 2,
                 screenHeight / 2 - 4, 30, LIGHTGRAY);

        if (game_over_timer >= GAME_OVER_INPUT_DELAY
            && std::fmod(game_over_timer, 1.0f) < 0.5f)
        {
            const char* hint = "Pulsa cualquier tecla para volver al menu";
            DrawText(hint, (screenWidth - MeasureText(hint, 26)) / 2,
                     screenHeight / 2 + 70, 26, RAYWHITE);
        }
    }
    else if(winner != nullptr){
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.68f));
        const auto result = TextFormat("GANADOR: %s", winner->getName().c_str());
        DrawText(result, (screenWidth - MeasureText(result, 54)) / 2, screenHeight / 2 - 60, 54, GOLD);
        DrawText(result, (screenWidth - MeasureText(result, 54)) / 2,
                 screenHeight / 2 - 60, 54, GOLD);
        const char* restart = "Pulsa cualquier tecla para salir";
    }

    //se dibuja la interfaz de debug
    if (debug_overlay.isVisible()) {
        rlImGuiBegin();
        debug_overlay.draw(game, config, GetFrameTime());
        rlImGuiEnd();
    }

    EndDrawing();
}

void MainGameState::renderMap()
{
    const Map& map = game.getMap();
    const int columns = static_cast<int>(map.getWidth());
    const int rows = static_cast<int>(map.getHeight());
    const std::vector<TerritoryCell>& cells = map.getCells();

    const bool requires_rebuild = !map_texture_ready
        || map_texture.texture.width != columns
        || map_texture.texture.height != rows
        || map_generation_seen != game.getMapGeneration();

    if (requires_rebuild)
    {
        if (map_texture_ready)
        {
            UnloadRenderTexture(map_texture);
        }

        map_texture = LoadRenderTexture(columns, rows);
        SetTextureFilter(map_texture.texture, TEXTURE_FILTER_POINT);
        map_texture_ready = true;
        map_generation_seen = game.getMapGeneration();
        game.takeChangedCells(changed_cells_buffer);
        changed_cells_buffer.clear();

        BeginTextureMode(map_texture);
        ClearBackground(Color{38, 45, 54, 255});
        for (int row = 0; row < rows; ++row)
        {
            for (int column = 0; column < columns; ++column)
            {
                const std::size_t index =
                    static_cast<std::size_t>(row) * map.getWidth()
                    + static_cast<std::size_t>(column);

                DrawRectangle(column, row, 1, 1, cellColor(cells[index]));
            }
        }
        EndTextureMode();
    }
    else
    {
        // Solo se repintan las celdas que Game ha marcado como cambiadas
        game.takeChangedCells(changed_cells_buffer);
        if (!changed_cells_buffer.empty())
        {
            BeginTextureMode(map_texture);
            for (const std::size_t index : changed_cells_buffer)
            {
                const int column = static_cast<int>(index % map.getWidth());
                const int row = static_cast<int>(index / map.getWidth());
                DrawRectangle(column, row, 1, 1, cellColor(cells[index]));
            }
            EndTextureMode();
            changed_cells_buffer.clear();
        }
    }

    map_camera = buildMapCamera();
    BeginMode2D(map_camera);
    DrawTexturePro(
        map_texture.texture,
        Rectangle{0.0f, 0.0f,
                  static_cast<float>(columns),
                  static_cast<float>(-rows)},
        Rectangle{0.0f, 0.0f,
                  static_cast<float>(columns),
                  static_cast<float>(rows)},
        Vector2{0.0f, 0.0f},
        0.0f,
        WHITE);

    for (Entity* entity : game.getEntities()){
        const Color base = entity->getColor();
        const Color border{static_cast<unsigned char>(base.r * 0.75f), static_cast<unsigned char>(base.g * 0.75f), static_cast<unsigned char>(base.b * 0.75f), 255};

        for (const auto& cell : entity->getFrontier()){
            if (map.getCell(cell.first, cell.second).owner != entity) {
                continue;
            }
            DrawRectangle(cell.first, cell.second, 1, 1, border);
        }
    }
    EndMode2D();
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

Camera2D MainGameState::buildMapCamera() const
{
    const Rectangle bounds = getMapBounds();
    const Map& map = game.getMap();

    Camera2D camera{};
    camera.offset = Vector2{
        bounds.x + bounds.width / 2.0f,
        bounds.y + bounds.height / 2.0f
    };
    camera.target = Vector2{
        static_cast<float>(map.getWidth()) / 2.0f + camera_pan.x,
        static_cast<float>(map.getHeight()) / 2.0f + camera_pan.y
    };
    camera.rotation = 0.0f;
    const float fitZoom = std::min(
        bounds.width / static_cast<float>(map.getWidth()),
        bounds.height / static_cast<float>(map.getHeight()));
    camera.zoom = fitZoom * camera_zoom;
    return camera;
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
    // La cámara del mapa define la conversión de pantalla a celdas.
    const Vector2 world = GetScreenToWorld2D(position, buildMapCamera());
    const Map& map = game.getMap();

    if (world.x < 0.0f || world.y < 0.0f
        || world.x >= static_cast<float>(map.getWidth())
        || world.y >= static_cast<float>(map.getHeight()))
    {
        return {-1, -1};
    }

    return {static_cast<int>(world.x), static_cast<int>(world.y)};
}