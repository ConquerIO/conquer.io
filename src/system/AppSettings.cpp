#include "system/AppSettings.hpp"
#include <raylib.h>
#include <algorithm>

void AppSettings::applyDisplayMode(DisplayMode mode){
#ifdef __EMSCRIPTEN__
    (void)mode;
    displayMode = DisplayMode::Windowed;
    return;
#else
    const int monitor = GetCurrentMonitor();
    const int monitorW = GetMonitorWidth(monitor);
    const int monitorH = GetMonitorHeight(monitor);
    const Vector2 monitorPos = GetMonitorPosition(monitor);

    //pantalla completa
    if (mode == DisplayMode::Fullscreen){
        if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) {
            ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
        }

        if (!IsWindowState(FLAG_FULLSCREEN_MODE)) {
            SetWindowState(FLAG_FULLSCREEN_MODE);
        }

        displayMode = mode;
        return;
    }

    if (IsWindowState(FLAG_FULLSCREEN_MODE)) {
        ClearWindowState(FLAG_FULLSCREEN_MODE);
    }

    //ventana a pantalla completa
    if (mode == DisplayMode::Borderless){
        if (!IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) {
            SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
        }

        SetWindowSize(monitorW, monitorH);
        SetWindowPosition(static_cast<int>(monitorPos.x),static_cast<int>(monitorPos.y));

        displayMode = mode;
        return;
    }

    //ventana
    if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) {
        ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    }

    constexpr float designW = 1600.0f;
    constexpr float designH = 900.0f;

    const float scale = std::min(1.0f,std::min(monitorW * 0.90f / designW,monitorH * 0.90f / designH));

    const int windowW = static_cast<int>(designW * scale);
    const int windowH = static_cast<int>(designH * scale);

    SetWindowSize(windowW, windowH);
    SetWindowPosition(static_cast<int>(monitorPos.x) + (monitorW - windowW) / 2,static_cast<int>(monitorPos.y) + (monitorH - windowH) / 2);

    displayMode = mode;
#endif
}