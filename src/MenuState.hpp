#pragma once
#include <raylib.h>
#include <GameState.hpp>
#include <GameConfig.hpp>
#include <string>
#include <vector>
#include "ui/UiLayout.hpp"
#include "ui/UiTheme.hpp"
#include "ui/UiWidgets.hpp"

struct MainMenuLayout { //elementos de la pantalla de menu
    Rectangle logo{};
    Rectangle nameField{};
    Rectangle colorSelector{};
    Rectangle primaryButton{};
    Rectangle settingsButton{};
    Rectangle exitButton{};
    Rectangle credits{};
    Rectangle version{};
};

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
        UiLayout ui_layout;
        UiTheme ui_theme;
        MainMenuLayout menu_layout;

        //elementos del menu
        TextField name_field;
        Button create_button;
        Button settings_button;
        Button exit_button;
        Button settings_back_button;

        // para saber que mostrar en cada momento
        bool show_customization = false;
        bool show_settings = false;

        // funciones para sacar la textura del logotipo y fuente para el menu (desde resourcemanager)
        bool loadLogo();
        bool loadFontResource(const char* key, const char* relativePath);
        bool loadMenuFonts();
        
        // escanear la carpeta maps/ para encontrar PNGs
        std::string assetPath(const std::string& relativePath) const;
        void scanAvailableMaps();
        
        //actualiza los valores de escalado y atributos de los elementos
        void updateLayout();
        
        int selected_color;
        static constexpr int NUM_COLORS = 6;

        // -- variables para el panel de personalizacion --
        GameConfig config;
        std::vector<std::string> available_maps;
        int selected_map_index = -1; // indice del mapa seleccionado en la lista (-1 = procedural)
        
        // 0 = ninguna caja con foco, 1 = foco en tropas jugador, 2 = foco en tropas bots, 3 = foco en nº de bots
        int active_troop_box = 0;
        std::string player_troops_str = "500";
        std::string bot_troops_str    = "350";
        
        std::string num_bots_str = "10";                                                                                                                                                                                                
        
        //limites para el numero de bots
        static constexpr std::size_t MIN_BOTS = 1;                                                                                                                                                                                     
        static constexpr std::size_t MAX_BOTS = 15;   

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
        void handleSettingsInput();
        void renderSettingsOverlay();
        
        Rectangle drawCheckbox(int x, int y, const char* label, bool checked);
};
