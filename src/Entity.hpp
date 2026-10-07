#pragma once
#include <raylib.h>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include <Target.hpp>

class Entity {
public:
    explicit Entity(std::string name, Color color);
    
    const std::string& getName() const {return name;}
    const Color getColor() const {return color;}
    const std::size_t getTroops() const {return troops;}
    const std::size_t getAttackFuel() const {return attack_fuel;}
    const Target& getTarget() const { return target; }
    std::vector<std::pair<int, int>>& getFrontier() { return frontier; }
    const std::vector<std::pair<int, int>>& getFrontier() const { return frontier; }

    void setTarget(const Target& target);
    void setTroops(std::size_t troops);
    void setAttackFuel(std::size_t fuel);
    void beginAttack(float ratio);
protected:
    std::string name;
    Color color;
    Target target;

    std::size_t troops = 500;
    std::size_t attack_fuel = 0;

    std::vector<std::pair<int, int>> frontier;
};