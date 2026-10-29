#include "ResourceManager.hpp"

ResourceManager* ResourceManager::INSTANCE = nullptr;

ResourceManager::ResourceManager() {}

void ResourceManager::initialize() {
    INSTANCE = new ResourceManager();
}

void ResourceManager::clear() {
    if (!INSTANCE) return;
    for (auto& pair : INSTANCE->resourceMap) {
        if (pair.second.type == Resource::IMAGE) {
            UnloadImage(pair.second.image);
        } else if (pair.second.type == Resource::FONT) {
            UnloadFont(pair.second.font);
        }
    }
    INSTANCE->resourceMap.clear();
    delete INSTANCE;
    INSTANCE = nullptr;
}

void ResourceManager::loadImage(const char* name, Image image) {
    if (!INSTANCE) return;
    Resource r;
    r.type = Resource::IMAGE;
    r.image = image;
    auto it = INSTANCE->resourceMap.find(name);
    if (it != INSTANCE->resourceMap.end()) {
        if (it->second.type == Resource::IMAGE) {
            UnloadImage(it->second.image);
        } else if (it->second.type == Resource::FONT) {
            UnloadFont(it->second.font);
        }
        INSTANCE->resourceMap.erase(it);
    }
    INSTANCE->resourceMap.insert({name, r});
}

void ResourceManager::unloadImage(const char* name) {
    if (!INSTANCE) return;
    auto it = INSTANCE->resourceMap.find(name);
    if (it != INSTANCE->resourceMap.end()) {
        if (it->second.type == Resource::IMAGE) {
            UnloadImage(it->second.image);
        }
        INSTANCE->resourceMap.erase(it);
    }
}

Image* ResourceManager::getImage(const char* name) {
    if (!INSTANCE) return nullptr;
    auto it = INSTANCE->resourceMap.find(name);
    if (it != INSTANCE->resourceMap.end() && it->second.type == Resource::IMAGE) {
        return &it->second.image;
    }
    return nullptr;
}

void ResourceManager::loadFont(const char* name, Font font) {
    if (!INSTANCE) return;
    Resource r;
    r.type = Resource::FONT;
    r.font = font;
    auto it = INSTANCE->resourceMap.find(name);
    if (it != INSTANCE->resourceMap.end()) {
        if (it->second.type == Resource::IMAGE) {
            UnloadImage(it->second.image);
        } else if (it->second.type == Resource::FONT) {
            UnloadFont(it->second.font);
        }
        INSTANCE->resourceMap.erase(it);
    }
    INSTANCE->resourceMap.insert({name, r});
}

void ResourceManager::unloadFont(const char* name) {
    if (!INSTANCE) return;
    auto it = INSTANCE->resourceMap.find(name);
    if (it != INSTANCE->resourceMap.end()) {
        if (it->second.type == Resource::FONT) {
            UnloadFont(it->second.font);
        }
        INSTANCE->resourceMap.erase(it);
    }
}

Font* ResourceManager::getFont(const char* name) {
    if (!INSTANCE) return nullptr;
    auto it = INSTANCE->resourceMap.find(name);
    if (it != INSTANCE->resourceMap.end() && it->second.type == Resource::FONT) {
        return &it->second.font;
    }
    return nullptr;
}