#pragma once
#include <raylib.h>
#include <string>

class Entity {
public:
    explicit Entity(std::string name, Color color);
    
    const std::string& getName() const {return name;}
    const Color getColor() const {return color;}
    const std::size_t getTroops() const {return troops;}
    Entity* getTarget() const { return target; }

    void setTarget(Entity* target);
    void setTroops(std::size_t troops);
protected:
    std::string name;
    Color color;
    Entity* target = nullptr;

    std::size_t troops;
};