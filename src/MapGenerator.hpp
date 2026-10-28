#pragma once
#include <raylib.h>

class MapGenerator{
    public:
        MapGenerator(int width, int height, int offsetX, int offsetY, float scale, int octaves, float diff, float weight);
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
        int octaves;//num capas
        float diff;//diferencia entre capas de la escala
        float weight;//el peso de cada capa respecto a la anterior
};