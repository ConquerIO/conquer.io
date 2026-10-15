#pragma once
#include <raylib.h>
#include <GameState.hpp>
#include <GameConfig.hpp>
#include <string>
#include <vector>

class MenuState : public GameState
{
    public:
        MenuState();
        ~MenuState() = default;

        void init() override;
        void handleInput() override;
        void update(float deltaTime) override;
        void render() override;

        void pause() override {}
        void resume() override {}

    private:
        std::string player_name;        // Nombre introducido por el jugador
        int selected_color;             // Índice del color seleccionado
        int frame_counter;              // Contador de frames (para el cursor parpadeante)

        static constexpr int MAX_NAME_LENGTH = 15;
        static constexpr int NUM_COLORS      = 6;

        float backspace_timer;              // Temporizador para borrado continuo
        bool  backspace_repeating;          // Estamos en fase de repetición?


        // VARIABLES para el panel de personalizacion

        bool show_customization = false;
        GameConfig config;
        std::vector<std::string> available_maps;
        int selected_map_index = -1; // indice del mapa seleccionado en la lista (-1 = procedural)
        
        // 0 = ninguna caja con foco, 1 = foco en tropas jugador, 2 = foco en tropas bots
        int active_troop_box = 0;
        std::string player_troops_str = "500";
        std::string bot_troops_str    = "250";

        // límites enteros estrictos
        static constexpr std::size_t MIN_PLAYER_TROOPS = 100;
        static constexpr std::size_t MAX_PLAYER_TROOPS = 2000;
        static constexpr std::size_t MIN_BOT_TROOPS = 50;
        static constexpr std::size_t MAX_BOT_TROOPS = 1000;

        // submetodos para organizar el código
        void handleMainMenuInput();
        void handleCustomizationInput();
        void renderMainMenu();
        void renderCustomizationOverlay();
        // escanear la carpeta maps/ para encontrar PNGs
        void scanAvailableMaps();
        Rectangle drawCheckbox(int x, int y, const char* label, bool checked);
};
