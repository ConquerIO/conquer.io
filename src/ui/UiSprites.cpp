#include "ui/UiSprites.hpp"
#include "system/ResourceManager.hpp"
#include "system/Logger.hpp"
#include <raylib.h>
#include <string>

namespace
{
    std::string assetPath(const char* relative){
        std::string base = GetApplicationDirectory();
        if (!base.empty() && base.back() != '/') {
            base += '/';
        }
        return base + relative;
    }

    void loadSprite(const char* key, const char* relativePath){
        if (ResourceManager::getTexture(key) != nullptr) {
            return;
        }
        const std::string path = assetPath(relativePath);
        Texture2D texture = LoadTexture(path.c_str());
        if (texture.id == 0) {
            Logger::warn(("UiSprites: no se pudo cargar " + path).c_str());
            return;
        }
        Logger::info(("UiSprites: cargada " + path).c_str());
        SetTextureFilter(texture, TEXTURE_FILTER_POINT);
        ResourceManager::loadTexture(key, texture);
    }
}

void UiSprites::load(){
    loadSprite(Sword,"assets/images/espada_velocidad.png");
    loadSprite(Handshake,"assets/images/apreton_manos.png");
    loadSprite(HandshakeSmall, "assets/images/manos_apreton.png");
    loadSprite(Money,"assets/images/bolsa_dinero.png");
    loadSprite(Crown,"assets/images/corona.png");
    loadSprite(Ship,"assets/images/barco_perfil_8x8.png");
    loadSprite(ZoomIn,  "assets/images/mas.png");
    loadSprite(ZoomOut, "assets/images/menos.png");
}