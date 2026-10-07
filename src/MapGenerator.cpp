#include "MapGenerator.hpp"

MapGenerator::MapGenerator(int width, int height, int offsetX, int offsetY, float scale){
    this->width=width;
    this->height=height;
    this->offsetX=offsetX;
    this->offsetY=offsetY;
    this->scale=scale;
}


Image MapGenerator::generate() const{
    Image noiseMap=GenImagePerlinNoise(width,height,offsetX, offsetY,scale);
    return noiseMap;
}

int MapGenerator::getWidth() const{
    return width;
}
int MapGenerator::getHeight() const{
    return height;
}
int MapGenerator::getoffSetX() const{
    return offsetX;
}
int MapGenerator::getoffSetY() const{
    return offsetY;
}
float MapGenerator::getScale() const{
    return scale;
}