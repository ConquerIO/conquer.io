#include <Map.hpp>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <raylib.h>

using namespace std;

namespace
{
std::size_t checkedCellCount(std::size_t rows, std::size_t columns)
{
    if (rows == 0 || columns == 0)
    {
        throw std::invalid_argument("Las dimensiones del mapa deben ser mayores que cero.");
    }
    if (columns > std::numeric_limits<std::size_t>::max() / rows)
    {
        throw std::length_error("Las dimensiones del mapa exceden el tamaño permitido.");
    }
    return rows * columns;
}
}

Map::Map(std::size_t rows, std::size_t columns)
    : rows(rows), columns(columns), cells(checkedCellCount(rows, columns))
{
}

TerritoryCell& Map::getCell(std::size_t x, std::size_t y) {
    return cells[y * columns + x];
}

const TerritoryCell& Map::getCell(std::size_t x, std::size_t y) const {
    return cells[y * columns + x];
}

TerritoryCell& Map::getCellFromIndex(std::size_t index) {
    return cells[index];
}

const TerritoryCell& Map::getCellFromIndex(std::size_t index) const {
    return cells[index];
}

Map Map::fromImage(const Image& image){    
    int width = image.width;
    int height = image.height;
    
    Image imageRGBA = ImageCopy(image);
    ImageFormat(&imageRGBA, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    
    uint8_t* pixels = (uint8_t*)imageRGBA.data;
    
    Map map(static_cast<std::size_t>(height), static_cast<std::size_t>(width));

    for (std::size_t y = 0; y < map.getHeight(); ++y) {
        for (std::size_t x = 0; x < map.getWidth(); ++x) {
            const std::size_t pixelIndex = (y * map.getWidth() + x) * 4;
            
            uint8_t r = pixels[pixelIndex + 0];
            uint8_t g = pixels[pixelIndex + 1];
            uint8_t b = pixels[pixelIndex + 2];
            
            bool isWater = (r == 0 && g == 0 && b == 0);
            
            TerritoryCell& cell = map.getCell(x, y);
            
            cell.owner = isWater ? Owner::Water : Owner::Land;
        }
    }
    
    UnloadImage(imageRGBA);

    return map;
}

 
Map Map::fromImage(const char* filename){
    Image image = LoadImage(filename);
    
    if (!image.data) {
        throw std::runtime_error("No se pudo cargar la imagen: " + std::string(filename));
    }
    
    Map map=Map::fromImage(image);
    UnloadImage(image);
    return map;

}

Map Map::fromPerlinImage(const Image& image, float threshold){    
    int width = image.width;
    int height = image.height;
    
    Image imageRGBA = ImageCopy(image);
    ImageFormat(&imageRGBA, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    
    uint8_t* pixels = (uint8_t*)imageRGBA.data;
    
    Map map(static_cast<std::size_t>(height), static_cast<std::size_t>(width));

    for (std::size_t y = 0; y < map.getHeight(); ++y) {
        for (std::size_t x = 0; x < map.getWidth(); ++x) {
            const std::size_t pixelIndex = (y * map.getWidth() + x) * 4;
            
            uint8_t r = pixels[pixelIndex + 0];
            float value=r/255.0f;
            bool isWater = value<threshold;
            
            TerritoryCell& cell = map.getCell(x, y);
            
            cell.owner = isWater ? Owner::Water : Owner::Land;
        }
    }
    
    UnloadImage(imageRGBA);

    return map;
}

Map Map::generateMap(){
    int height=100;
    int width=100;

    int offsetX = std::rand() % 10000;
    int offsetY = std::rand() % 10000;

    float scale=10.0f;
    //esto cuanto mas grande sea mas agua hay sobre tierra (0.5 -> 50% agua y 50% tierra)
    float threshold=0.375f;

    MapGenerator generator(width, height, offsetX, offsetY, scale);
    Image noise=generator.generate();
    Map map= Map::fromPerlinImage(noise, threshold);
    UnloadImage(noise);
    return map;
}

