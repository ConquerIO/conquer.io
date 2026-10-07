#pragma once
#include <raylib.h>
#include <string>

class Entity {
public:
    explicit Entity(std::string name, Color color);
    
    const std::string& getName() const {return name;}
    const Color getColor() const {return color;}
    int getTargetIndex() const { return targetIndex; }

    void setTarget(int targetIndex);
protected:
    std::string name;
    Color color;
    int targetIndex = -1;
};