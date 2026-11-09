#include "AudioManager.hpp"
#include "Logger.hpp"
#include <fstream>
#include <map>
#include <string>

namespace {

constexpr int VARIANTS = 10;

const char* const NAMES[] = {
    "ui_click", "ui_hover", "ui_change", "ui_type", "game_start",
    "attack", "context_menu", "game_over", "winner",
};

std::string soundsDir() {
    std::string base = GetApplicationDirectory();
    if (!base.empty() && base.back() != '/') base += '/';
    return base + "assets/sounds/";
}

// Lee seleccion.txt (líneas "evento=N"; '#' son comentarios).
std::map<std::string, int> readSelection(const std::string& path) {
    std::map<std::string, int> selection;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        const std::size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        try {
            selection[line.substr(0, eq)] = std::stoi(line.substr(eq + 1));
        } catch (...) {
            // línea inválida: se ignora y se usa la variante 1
        }
    }
    return selection;
}

} // namespace

std::array<Sound, AudioManager::COUNT> AudioManager::sounds{};
std::array<bool, AudioManager::COUNT> AudioManager::loaded{};
bool AudioManager::muted = false;

void AudioManager::toggleMute() {
    if (!IsAudioDeviceReady()) return;
    muted = !muted;
    SetMasterVolume(muted ? 0.0f : 1.0f);
    Logger::info(muted ? "Sonido silenciado" : "Sonido activado");
}

void AudioManager::initialize() {
    if (!IsAudioDeviceReady()) {
        Logger::warn("Dispositivo de audio no disponible: el juego seguirá sin sonido");
        return;
    }

    const std::string dir = soundsDir();
    const auto selection = readSelection(dir + "seleccion.txt");

    int count = 0;
    for (int i = 0; i < COUNT; ++i) {
        const std::string name = NAMES[i];
        int variant = 1;
        const auto it = selection.find(name);
        if (it != selection.end() && it->second >= 1 && it->second <= VARIANTS) {
            variant = it->second;
        }

        const std::string path = dir + name + "/" + name + "_" + std::to_string(variant) + ".wav";
        if (!FileExists(path.c_str())) {
            Logger::warn(("Falta el sonido: " + path).c_str());
            continue;
        }
        sounds[i] = LoadSound(path.c_str());
        loaded[i] = IsSoundValid(sounds[i]);
        if (loaded[i]) ++count;
    }
    Logger::info(("Audio inicializado: " + std::to_string(count) + "/" + std::to_string(COUNT) + " sonidos cargados").c_str());
}

void AudioManager::clear() {
    for (int i = 0; i < COUNT; ++i) {
        if (loaded[i]) UnloadSound(sounds[i]);
        loaded[i] = false;
    }
}

void AudioManager::play(Sfx sfx) {
    const int i = static_cast<int>(sfx);
    if (i < 0 || i >= COUNT || !loaded[i]) return;
    PlaySound(sounds[i]);
}
