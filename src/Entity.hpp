#pragma once
#include <raylib.h>
#include <string>

class Entity {
public:
    explicit Entity(std::string name, Color color);
    
    const std::string& getName() const {return name;}
    const Color getColor() const {return color;}
    const std::size_t getTroops() const {return troops;}
    int getTargetIndex() const { return targetIndex; }

    void setTarget(int targetIndex);
    void setTroops(std::size_t troops);
protected:
    std::string name;
    Color color;
    int targetIndex = -1;

    std::size_t troops;
};