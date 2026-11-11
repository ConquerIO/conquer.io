#include <StateMachine.hpp>
#include <MenuState.hpp>
#include <raylib.h>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <chrono>
#include <atomic>
#include <thread>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "rlImGui.h"
#include "imgui.h"
#include "system/Logger.hpp"
#include "system/ResourceManager.hpp"
#include "system/AudioManager.hpp"

void initializeRandomSeed(){
    std::srand(
        static_cast<unsigned>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count()
        )
    );
}

namespace {
float delta_time = 0.0f;
std::unique_ptr<StateMachine> state_machine;

#ifndef __EMSCRIPTEN__
bool game_running()
{
    return !state_machine->is_game_ending() && !WindowShouldClose();
}
#endif

void frame()
{
#ifdef __EMSCRIPTEN__
    // Red de seguridad: si algo pide terminar, se para el bucle en vez de tocar una pila de estados vacía
    if (state_machine->is_game_ending()) {
        emscripten_cancel_main_loop();
        return;
    }
#endif
    delta_time = GetFrameTime();
    state_machine->handle_state_changes(delta_time);
    if (IsKeyPressed(KEY_F2)) AudioManager::toggleMute();
    state_machine->getCurrentState()->handleInput();
    state_machine->getCurrentState()->update(delta_time);
    state_machine->getCurrentState()->render();
}
}

int main()
{   
    Logger::debug("Inicializando seed...");
    initializeRandomSeed();
    Logger::debug("Seed inicializada correctamente");

    Logger::info("Inicializando ventana (2560x1440)...");
#ifdef __EMSCRIPTEN__
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#endif
    InitWindow(2560, 1440, "Conquer.io v0.1.0-alpha");
    ResourceManager::initialize();

    Logger::info("Inicializando audio...");
    InitAudioDevice();
    AudioManager::initialize();

#ifndef __EMSCRIPTEN__
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
    //para centrar la ventana en la pantalla
    const int monitor = GetCurrentMonitor();
    const Vector2 monitorPos = GetMonitorPosition(monitor);
    const int monitorW = GetMonitorWidth(monitor);
    const int monitorH = GetMonitorHeight(monitor);

    const int winW = GetScreenWidth();
    const int winH = GetScreenHeight();

    SetWindowPosition(
        static_cast<int>(monitorPos.x) + (monitorW - winW) / 2,
        static_cast<int>(monitorPos.y) + (monitorH - winH) / 2
    );

#endif

    SetExitKey(KEY_NULL);

    Logger::debug("Inicializando librerías de debug");
    rlImGuiSetup(true);
    //esta es para que no cree nada en el directorio de trabajo
    ImGui::GetIO().IniFilename = nullptr;
#ifndef __EMSCRIPTEN__
    // En web el navegador marca el ritmo con requestAnimationFrame
    SetTargetFPS(60);
#endif

    Logger::info("Inicializando máquina de estados...");
    state_machine = std::make_unique<StateMachine>();
    state_machine->add_state(std::make_unique<MenuState>(), false);
    state_machine->handle_state_changes(delta_time);

#ifdef __EMSCRIPTEN__
    // En web el bucle lo gestiona el navegador; esta llamada no retorna
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
#else
    while (game_running())
    {
        frame();
    }

    Logger::info("Terminando ejecución...");
    Logger::debug("Liberando recursos de la memoria debug...");
    rlImGuiShutdown();
    Logger::debug("Terminando ventana...");
    ResourceManager::clear();
    // La ventana se cierra antes que el audio, para que la X responda al instante
    CloseWindow();

    // Cerrar el dispositivo de audio puede bloquearse si el sistema de sonido esta
    // atascado, asi que se hace en un hilo con tiempo maximo para no dejar el proceso colgado
    Logger::debug("Liberando audio...");
    std::atomic<bool> audio_cerrado{false};
    std::thread audio_thread([&audio_cerrado] {
        AudioManager::clear();
        CloseAudioDevice();
        audio_cerrado = true;
    });

    const auto limite = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (!audio_cerrado && std::chrono::steady_clock::now() < limite) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (audio_cerrado) {
        audio_thread.join();
    } else {
        Logger::warn("El audio tarda demasiado en cerrarse: se fuerza la salida");
        std::_Exit(0);
    }

    return 0;
#endif
}
