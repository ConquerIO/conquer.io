#include <StateMachine.hpp>
#include <MenuState.hpp>
#include <raylib.h>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <chrono>

int main()
{

    std::srand(
        static_cast<unsigned>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count()
        )
    );


    InitWindow(2560, 1440, "Conquer.io");

    {
        const int monitor = GetCurrentMonitor();
        const int monitorWidth = GetMonitorWidth(monitor);
        const int monitorHeight = GetMonitorHeight(monitor);
        if (monitorWidth > 0 && monitorHeight > 0)
        {
            constexpr float designWidth = 2560.0f;
            constexpr float designHeight = 1440.0f;
            const float maxWidth = monitorWidth * 0.95f;
            const float maxHeight = monitorHeight * 0.90f;
            const float scale = std::min(1.0f, std::min(maxWidth / designWidth,
                                                        maxHeight / designHeight));
            if (scale < 1.0f)
            {
                SetWindowSize(static_cast<int>(designWidth * scale),
                              static_cast<int>(designHeight * scale));
            }
        }
    }

    SetTargetFPS(60);

    float delta_time = 0.0f;

    StateMachine state_machine = StateMachine();
    state_machine.add_state(std::make_unique<MenuState>(), false);
    state_machine.handle_state_changes(delta_time);

    while (!state_machine.is_game_ending() && !WindowShouldClose())
    {
        delta_time = GetFrameTime();
        state_machine.handle_state_changes(delta_time);
        state_machine.getCurrentState()->handleInput();
        state_machine.getCurrentState()->update(delta_time);
        state_machine.getCurrentState()->render();
    }

    CloseWindow();

    return 0;
}