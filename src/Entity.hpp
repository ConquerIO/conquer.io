#pragma once
#include <raylib.h>
#include <string>
#include <Target.hpp>

class Entity {
public:
    explicit Entity(std::string name, Color color);
    
    const std::string& getName() const {return name;}
    const Color getColor() const {return color;}
    const std::size_t getTroops() const {return troops;}
    const Target& getTarget() const { return target; }

    void setTarget(const Target& target);
    void setTroops(std::size_t troops);
protected:
    std::string name;
    Color color;
    Target target;

    std::size_t troops;
};