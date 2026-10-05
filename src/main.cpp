#include <StateMachine.hpp>
#include <MenuState.hpp>
#include <raylib.h>
#include <memory>

int main()
{
    InitWindow(2560, 1440, "Conquer.io");
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