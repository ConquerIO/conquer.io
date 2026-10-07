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
    const std::size_t getAttackFuel() const {return attack_fuel;}
    const Target& getTarget() const { return target; }

    void setTarget(const Target& target);
    void setTroops(std::size_t troops);
    void setAttackFuel(std::size_t fuel);
    // Destina una fraccion [0,1] de las tropas del territorio a combustible
    // del ataque en curso. El resto permanece en el territorio.
    void beginAttack(float ratio);
protected:
    std::string name;
    Color color;
    Target target;

    std::size_t troops = 500;
    std::size_t attack_fuel = 0;
};