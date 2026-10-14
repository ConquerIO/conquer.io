#pragma once
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

        bool show_customization = false;
        GameConfig config;

        std::vector<std::string> available_maps;

        // indice del mapa seleccionado en la lista (-1 = procedural)
        int selected_map_index = -1;

        // submetodos para organizar el código
        void handleMainMenuInput();
        void handleCustomizationInput();
        void renderMainMenu();
        void renderCustomizationOverlay();

        // escanear la carpeta maps/ para encontrar PNGs
        void scanAvailableMaps();

};
