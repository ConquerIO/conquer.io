#pragma once
#include <array>
#include "raylib.h"

// Efectos de sonido del juego. Cada evento tiene su carpeta en assets/sounds/<evento>/
// con 10 variantes; la que suena se elige en assets/sounds/seleccion.txt (evento=N).
enum class Sfx {
    UiClick,
    UiHover,
    UiChange,
    UiType,
    GameStart,
    Attack,
    ContextMenu,
    GameOver,
    Winner,
    Count
};

// Se usa como: AudioManager::play(Sfx::UiClick);
// Si no hay dispositivo de audio o falta un fichero, no hace nada.
class AudioManager {
public:
    static void initialize();   // tras InitAudioDevice()
    static void clear();        // antes de CloseAudioDevice()
    static void play(Sfx sfx);
    static void toggleMute();   // silencia / reactiva todo el audio (F2)
    static bool isMuted() { return muted; }

private:
    static bool muted;
    static constexpr int COUNT = static_cast<int>(Sfx::Count);
    static std::array<Sound, COUNT> sounds;
    static std::array<bool, COUNT> loaded;
};
