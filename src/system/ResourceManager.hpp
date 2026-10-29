#pragma once
#include <unordered_map>
#include "raylib.h"

struct Resource {
    enum Type { IMAGE, FONT } type;
    union {
        Image image;
        Font font;
    };
};

class ResourceManager {
public:
    static ResourceManager* getInstance() { return INSTANCE; }

    static void initialize();
    static void clear();

    static void loadImage(const char* name, Image image);
    static void unloadImage(const char* name);
    static Image* getImage(const char* name);

    static void loadFont(const char* name, Font font);
    static void unloadFont(const char* name);
    static Font* getFont(const char* name);

private:
    ResourceManager();
    std::unordered_map<const char*, Resource> resourceMap;
    static ResourceManager* INSTANCE;
};