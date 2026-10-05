#include <Map.hpp>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <raylib.h>

Map::Map(size_t rows, size_t columns): rows(rows), columns(columns), cells(std::vector<TerritoryCell>(rows * columns)) {}

TerritoryCell& Map::getCell(size_t x, size_t y) {
    return cells[y * columns + x];
}

const TerritoryCell& Map::getCell(size_t x, size_t y) const {
    return cells[y * columns + x];
}

TerritoryCell& Map::getCellFromIndex(size_t index) {
    return cells[index];
}

const TerritoryCell& Map::getCellFromIndex(size_t index) const {
    return cells[index];
}

 
Map Map::fromImage(const char* filename){
    Image image = LoadImage(filename);
    
    if (!image.data) {
        throw std::runtime_error("No se pudo cargar la imagen: " + std::string(filename));
    }
    
    int width = image.width;
    int height = image.height;
    
    Image imageRGBA = ImageCopy(image);
    ImageFormat(&imageRGBA, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    
    uint8_t* pixels = (uint8_t*)imageRGBA.data;
    
    Map map(height, width);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixelIndex = (y * width + x) * 4;
            
            uint8_t r = pixels[pixelIndex + 0];
            uint8_t g = pixels[pixelIndex + 1];
            uint8_t b = pixels[pixelIndex + 2];
            
            bool isWater = (r == 0 && g == 0 && b == 0);
            
            TerritoryCell& cell = map.getCell(x, y);
            
            cell.owner = isWater ? Owner::Water : Owner::Land;
        }
    }
    
    UnloadImage(image);
    UnloadImage(imageRGBA);

    return map;
}