#pragma once

enum class DisplayMode {
    Windowed,
    Borderless,
    Fullscreen
};

namespace AppSettings {
    inline DisplayMode displayMode = DisplayMode::Windowed;
    inline float sfxVolume = 1.0f;

    void applyDisplayMode(DisplayMode mode);
}