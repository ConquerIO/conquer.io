#include "MapGenerator.hpp"
#include "system/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

MapGenerator::MapGenerator(int width, int height, int offsetX, int offsetY, float scale, int octaves, float diff, float weight){
    this->width=width;
    this->height=height;
    this->offsetX=offsetX;
    this->offsetY=offsetY;
    this->scale=scale;
    this->octaves=octaves;
    this->diff=diff;
    this->weight=weight;
}


Image MapGenerator::generate() const{
    Logger::info("Generando mapa por capas...");

    const std::size_t pixelCount =static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    //suma ponderada de las capas
    std::vector<float> heightmap(pixelCount, 0.0f);
    //peso de la primera capa
    float amplitude = 1.0f;

    for (int i = 0; i < octaves; i++){
        //va aumentando la escala con el valor de diff, ej si tenemos 4 la siguiente es 8 (diff=2)
        const float octaveScale = std::max(1.0f,scale * std::pow(diff, static_cast<float>(i)));
        //para que 
        const int octaveOffsetX = offsetX + i * 1000;
        const int octaveOffsetY = offsetY + i * 1000;

        //se genera la capa con un byte por pixel
        Image layer = GenImagePerlinNoise(width, height, octaveOffsetX, octaveOffsetY, octaveScale);
        ImageFormat(&layer, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);

        //se leen los pixeles y se suman el peso de la capa para cada uno
        const unsigned char* pixels =static_cast<const unsigned char*>(layer.data);
        for (std::size_t p = 0; p < pixelCount; ++p)
        {
            heightmap[p] += static_cast<float>(pixels[p]) * amplitude;
        }

        //se actualiza el peso
        amplitude *= weight;
        UnloadImage(layer);
    }

    //normalizar
    float minValue = heightmap[0];
    float maxValue = heightmap[0];
    for (float value : heightmap){
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
    }

    const float range = maxValue - minValue;

    //si >0 se normaliza sino se pone 0.5 para evitar error por division entre cero
    for (float& value : heightmap){
        value = (range > 0.0f) ? (value - minValue) / range: 0.5f;
    }

    //se genera el mapa
    Image result = GenImageColor(width, height, BLACK);
    ImageFormat(&result, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    unsigned char* out = static_cast<unsigned char*>(result.data);
    for (std::size_t p = 0; p < pixelCount; p++){
        out[p] = static_cast<unsigned char>(heightmap[p] * 255.0f);
    }

    return result;
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