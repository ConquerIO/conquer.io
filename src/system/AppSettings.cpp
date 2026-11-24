#include "system/AppSettings.hpp"
#include <raylib.h>
#include <algorithm>

void AppSettings::resetToWindowed()
{
#ifdef __EMSCRIPTEN__
    displayMode = DisplayMode::Windowed;
    return;
#else
    ClearWindowState(FLAG_FULLSCREEN_MODE);
    ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    ClearWindowState(FLAG_WINDOW_UNDECORATED);
    ClearWindowState(FLAG_WINDOW_MAXIMIZED);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
#endif
}

void AppSettings::applyTargetDisplayMode(DisplayMode mode)
{
#ifdef __EMSCRIPTEN__
    (void)mode;
    displayMode = DisplayMode::Windowed;
    return;
#else
    if (mode == DisplayMode::Fullscreen) {
        ClearWindowState(FLAG_WINDOW_MAXIMIZED);
        SetWindowState(FLAG_FULLSCREEN_MODE);
    }
    else if (mode == DisplayMode::Maximized) {
        ClearWindowState(FLAG_FULLSCREEN_MODE);
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        SetWindowState(FLAG_WINDOW_MAXIMIZED);
    }
    else {
        ClearWindowState(FLAG_FULLSCREEN_MODE);
        ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
        ClearWindowState(FLAG_WINDOW_MAXIMIZED);
        SetWindowState(FLAG_WINDOW_RESIZABLE);

        const int monitor = GetCurrentMonitor();
        const int monitorW = GetMonitorWidth(monitor);
        const int monitorH = GetMonitorHeight(monitor);
        const Vector2 monitorPos = GetMonitorPosition(monitor);

        constexpr float designW = 1600.0f;
        constexpr float designH = 900.0f;

        const float scale = std::min(1.0f,std::min(monitorW * 0.90f / designW,monitorH * 0.90f / designH));

        const int windowW = static_cast<int>(designW * scale);
        const int windowH = static_cast<int>(designH * scale);

        SetWindowSize(windowW, windowH);
        SetWindowPosition(static_cast<int>(monitorPos.x) + (monitorW - windowW) / 2,static_cast<int>(monitorPos.y) + (monitorH - windowH) / 2);
    }

    displayMode = mode;
#endif
}