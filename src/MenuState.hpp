#pragma once
#include <GameState.hpp>
#include <string>

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
        bool  backspace_repeating;          // ¿Estamos en fase de repetición?
};
