#pragma once
#include <raylib.h>

class MapGenerator{
    public:
        MapGenerator(int width, int height, int offsetX, int offsetY, float scale);
        Image generate() const;
        int getWidth() const;
        int getHeight() const;
        int getoffSetX() const;
        int getoffSetY() const;
        float getScale() const;

    private:
        int width;
        int height;
        int offsetX;
        int offsetY;
        float scale;
};