#include <DebugOverlay.hpp>

#include <Game.hpp>
#include <GameConfig.hpp>
#include <Player.hpp>
#include <TerritoryCell.hpp>

#include <raylib.h>
#include <rlgl.h>
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
constexpr float PANEL_MARGIN = 12.0f;
//esta constante es lo de la opacidad
constexpr float PANEL_ALPHA = 0.55f;    
constexpr float GAME_PANEL_WIDTH = 410.0f;
constexpr float TECH_PANEL_WIDTH = 390.0f;
constexpr float LABEL_WIDTH = 145.0f;
constexpr int MAX_BOTS_SHOWN = 4;

//buffer estatico
template <typename... Args>
std::string format(const char* fmt, Args... args)
{
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), fmt, args...);
    return std::string(buffer);
}

ImVec4 toImVec4(Color color)
{
    return ImVec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f);
}

std::string colorHex(Color color)
{
    return format("#%02X%02X%02X", color.r, color.g, color.b);
}

//para establecer el formaro que se imprime los bytes
std::string formatBytes(std::size_t bytes)
{
    constexpr double KB = 1024.0;
    constexpr double MB = KB * 1024.0;
    constexpr double GB = MB * 1024.0;

    if (bytes < static_cast<std::size_t>(KB)){
        return format("%zu B", bytes);
    }

    if (bytes < static_cast<std::size_t>(MB)){
        return format("%.1f KB", static_cast<double>(bytes) / KB);

    }
    if (bytes < static_cast<std::size_t>(GB)){
        return format("%.1f MB", static_cast<double>(bytes) / MB);
    }
    return format("%.2f GB", static_cast<double>(bytes) / GB);
}

//para el formato del reloj 
std::string formatClock(double seconds)
{
    const int total = static_cast<int>(seconds);
    return format("%02d:%02d", total / 60, total % 60);
}

//para el formato de la dificultad
std::string difficultyName(BotDifficulty difficulty)
{
    switch (difficulty)
    {
        case BotDifficulty::VERY_EASY: return "VERY_EASY";
        case BotDifficulty::EASY:      return "EASY";
        case BotDifficulty::MEDIUM:    return "MEDIUM";
        case BotDifficulty::HARD:      return "HARD";
        case BotDifficulty::EXTREME:   return "EXTREME";
    }
    return "?";
}

//para la etiqueta del nombre de donde atacas
std::string targetName(const Target& target)
{
    if (target.isNone()) {
        return "ninguno";
    }
    if (target.isLand()) {
        return "tierra";
    }

    Entity* entity = target.getEntity();
    return entity != nullptr ? format("enemigo: %s", entity->getName().c_str()) : std::string("enemigo");
}

//version GL
std::string openGlName()
{
    switch (rlGetVersion())
    {
        case RL_OPENGL_11: return "1.1";
        case RL_OPENGL_21: return "2.1";
        case RL_OPENGL_33: return "3.3";
        case RL_OPENGL_43: return "4.3";
        case RL_OPENGL_ES_20: return "ES 2.0";
        case RL_OPENGL_ES_30: return "ES 3.0";
        default: return "?";
    }
}

//para formato de la arquitectura
std::string architectureName()
{
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#else
    return "otra";
#endif
}

//para sacar la version del compilador
std::string compilerName()
{
    std::string version = __VERSION__;
    const std::size_t space = version.find(' ');
    if (space != std::string::npos) version.resize(space);
    return version;
}

//se hace el panel iniciar a mostrar
void beginPanel(const char* id, ImVec2 position, float width)
{
    ImGui::SetNextWindowPos(position, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(PANEL_ALPHA);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 4.0f));

    ImGui::Begin(id, nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs
        | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing);
}

void endPanel()
{
    ImGui::End();
    ImGui::PopStyleVar(2);
}

void section(const char* title)
{
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.48f, 0.80f, 1.0f, 1.0f), "%s", title);
    ImGui::Separator();
}


void kv(const char* label, const char* value)
{
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine(LABEL_WIDTH);
    ImGui::TextUnformatted(value);
}

void kv(const char* label, const std::string& value)
{
    kv(label, value.c_str());
}


void kvColor(const char* label, const char* id, Color color)
{
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine(LABEL_WIDTH);
    ImGui::ColorButton(id, toImVec4(color),ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,ImVec2(14.0f, 14.0f));
    ImGui::SameLine();
    ImGui::TextUnformatted(colorHex(color).c_str());
}
}

//dibuja la pantalla con los dos paneles
void DebugOverlay::draw(const Game& game, const GameConfig& config, float deltaTime)
{
    fps_history[fps_head] = static_cast<float>(GetFPS());
    fps_head = (fps_head + 1) % FPS_HISTORY;
    if (fps_count < FPS_HISTORY) ++fps_count;

    if (!visible) return;

    system.update(deltaTime);

    drawGamePanel(game, config);
    drawTechPanel(game);
}

//dibuja el panel de la partida
void DebugOverlay::drawGamePanel(const Game& game, const GameConfig& config)
{
    const Map& map = game.getMap();
    Player* player = game.getPlayer();
    const std::vector<Entity*> entities = game.getEntities();
    const MapType map_type = config.map_type;
    const std::string map_file = config.map_filename;

    std::size_t land_cells = 0;
    std::size_t water_cells = 0;
    std::size_t neutral_cells = 0;
    std::unordered_map<Entity*, std::size_t> owned_cells;
    owned_cells.reserve(entities.size());


    //saca las celdas que son agua, tierra y de un usuario
    for (const TerritoryCell& cell : map.getCells())
    {
        if (cell.isWater)
        {
            ++water_cells;
            continue;
        }

        ++land_cells;

        if (cell.owner == nullptr)
        {
            ++neutral_cells;
            continue;
        }

        ++owned_cells[cell.owner];
    }

    const auto player_cells_it = owned_cells.find(player);
    const std::size_t player_cells = player_cells_it != owned_cells.end() ? player_cells_it->second : 0;

    //porcentaje de celdas respecto al total
    const double player_percent = land_cells == 0 ? 0.0 : 100.0 * static_cast<double>(player_cells) / static_cast<double>(land_cells);

    const std::size_t initial_bots = std::min<std::size_t>(config.numero_bots, 15);
    const std::size_t alive_bots = entities.empty() ? 0 : entities.size() - 1;
    const std::size_t eliminated_bots =
        initial_bots > alive_bots ? initial_bots - alive_bots : 0;

    const float next_tick = std::max(0.0f, Game::TICK_INTERVAL - game.getTickTimer());

    beginPanel("##debug_game", ImVec2(PANEL_MARGIN, PANEL_MARGIN),GAME_PANEL_WIDTH);

    section("PARTIDA");
    kv("Entidades", format("%zu  (1 jugador + %zu bots)", entities.size(), alive_bots));
    kv("Mapa", format("%zu x %zu", map.getWidth(), map.getHeight()));
    kv("Tipo", (map_type == MapType::MAP_FILE && !map_file.empty())
        ? format("Archivo: %s", map_file.c_str())
        : std::string("Procedural (generado)"));
    kv("Celdas", format("%zu  (tierra %zu · agua %zu)",
                        map.getCells().size(), land_cells, water_cells));
    kv("Neutrales", format("%zu celdas", neutral_cells));
    kv("Tropas iniciales", format("%zu jug. / %zu bots",
                                  config.initial_troops, config.bot_troops));
    kv("Interés", format("%.1f %%/s  (limite %.0f)",
                         game.getInterest(player) * 100.0f,
                         game.getTroopLimit(player)));
    kv("Territorio", format("%.2f %%  (%zu / %zu celdas)",
                            player_percent, player_cells, land_cells));
    kv("Dificultad bots", format("%s  (sin efecto aun)",
                                 difficultyName(config.difficulty).c_str()));

    section("JUGADOR");
    kv("Nombre", player->getName());
    kvColor("Color", "##player_color", player->getColor());
    kv("Tropas", format("%zu  (fuel %zu)",
                        player->getTroops(), player->getAttackFuel()));
    kv("Objetivo", targetName(player->getTarget()));
    kv("Celdas / frontera", format("%zu / %zu",
                                   player_cells, player->getFrontier().size()));

    section("BOTS");
    if (alive_bots == 0)
    {
        kv("Estado", "ninguno vivo");
    }
    else
    {
        const std::size_t shown = std::min<std::size_t>(alive_bots, MAX_BOTS_SHOWN);
        for (std::size_t i = 0; i < shown; ++i)
        {
            Entity* bot = entities[i + 1];
            const std::string label = format("Bot %zu", i + 1);
            const std::string id = format("##bot_color_%zu", i);

            ImGui::TextDisabled("%s", label.c_str());
            ImGui::SameLine(LABEL_WIDTH);
            ImGui::ColorButton(id.c_str(), toImVec4(bot->getColor()),
                               ImGuiColorEditFlags_NoTooltip
                               | ImGuiColorEditFlags_NoDragDrop,
                               ImVec2(14.0f, 14.0f));
            ImGui::SameLine();
            const auto bot_cells = owned_cells.find(bot);
            //t es tropas, c es celdas
            ImGui::Text("%zu t · %zu c", bot->getTroops(),bot_cells != owned_cells.end() ? bot_cells->second : 0);
        }

        //si no caben se indica que hay mas 
        if (alive_bots > shown)
        {
            ImGui::TextDisabled("... y %zu bots mas", alive_bots - shown);
        }
    }
    kv("Eliminados", format("%zu / %zu", eliminated_bots, initial_bots));

    section("ESTADO");
    kv("Tiempo", formatClock(game.getTime()));
    kv("Fase", game.getPhase() == GamePhase::SPAWN ? "SPAWN" : "PLAYING");
    if (game.getPhase() == GamePhase::SPAWN)
    {
        kv("Inicio en", format("%.1f s", game.getSpawnTimer()));
    }
    else
    {
        kv("Próximo tick", format("%.2f s", next_tick));
    }
    if (game.isOver())
    {
        kv("Resultado", "GAME OVER");
    }
    else if (game.getWinner() != nullptr)
    {
        kv("Ganador", game.getWinner()->getName());
    }

    endPanel();
}

//dibuja el panel de rendimiento
void DebugOverlay::drawTechPanel(const Game& game)
{
    const Map& map = game.getMap();
    const std::vector<Entity*> entities = game.getEntities();

    const ImGuiIO& io = ImGui::GetIO();
    const float x = std::max(PANEL_MARGIN,
                             io.DisplaySize.x - TECH_PANEL_WIDTH - PANEL_MARGIN);

    beginPanel("##debug_tech", ImVec2(x, PANEL_MARGIN), TECH_PANEL_WIDTH);

    section("RENDIMIENTO");
    kv("FPS", format("%d", GetFPS()));
    kv("Frame", format("%.2f ms", GetFrameTime() * 1000.0f));

    if (fps_count > 1)
    {
        const int offset =
            (fps_head - fps_count + FPS_HISTORY) % FPS_HISTORY;
        ImGui::PlotLines("##fps_graph", fps_history, fps_count, offset,
                         nullptr, 0.0f, FLT_MAX, ImVec2(-FLT_MIN, 42.0f));
    }

    kv("Ventana", format("%d x %d", GetScreenWidth(), GetScreenHeight()));
    kv("Render", format("%d x %d", GetRenderWidth(), GetRenderHeight()));
    kv("DPI / Hz", format("%.2f / %d Hz",
                          GetWindowScaleDPI().x,
                          GetMonitorRefreshRate(GetCurrentMonitor())));
    kv("OpenGL", openGlName());

    section("SISTEMA");
    if (system.available())
    {
        kv("CPU proceso", format("%.1f %%", system.processCpuPercent()));
        kv("CPU sistema", format("%.1f %%", system.systemCpuPercent()));
        kv("Núcleos", format("%d", SystemStats::cpuCoreCount()));
        kv("RAM proceso", format("%s / %s pico",
                                 formatBytes(system.processRssBytes()).c_str(),
                                 formatBytes(system.processPeakRssBytes()).c_str()));
        kv("RAM sistema", format("%s / %s libres",
                                 formatBytes(system.systemAvailableBytes()).c_str(),
                                 formatBytes(system.systemTotalBytes()).c_str()));
        kv("Hilos", format("%d", system.threadCount()));
        kv("Fallos pág.", format("%lu / %lu",
                                 system.minorFaults(), system.majorFaults()));
    }
    else
    {
        kv("Metricas", "no disponibles (solo Linux)");
    }

    section("MAPA (DEBUG)");
    std::size_t frontier_total = 0;
    for (Entity* entity : entities)
    {
        frontier_total += entity->getFrontier().size();
    }
    kv("Celdas", format("%zu", map.getCells().size()));
    kv("Memoria celdas",
       formatBytes(map.getCells().size() * sizeof(TerritoryCell)));
    kv("Frontera total", format("%zu celdas", frontier_total));
    kv("Entidades vivas", format("%zu", entities.size()));

    section("ENTORNO");
    kv("raylib", RAYLIB_VERSION);
    kv("Dear ImGui", IMGUI_VERSION);
    kv("Compilador", compilerName());
    kv("Build", __DATE__ " " __TIME__);
    kv("Arquitectura", architectureName());

    endPanel();
}
