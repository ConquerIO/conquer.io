#pragma once
#include "Entity.hpp"

class Boat {
public:
    const Entity* getOwner() const {return owner;}
    Vector2 getPosition() const {return position;}
    const Vector2 getTarget() const {return target;}
    unsigned long getTroops() const {return troops;}
    void setTroops(unsigned long);
private:
    const Entity* owner;
    Vector2 position;
    const Vector2 target;
    unsigned long troops;
};