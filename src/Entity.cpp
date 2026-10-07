#include <Entity.hpp>

Entity::Entity(std::string name, Color color): name(name), color(color) {}

void Entity::setTarget(const Target& target){
    this->target = target;
}

void Entity::setTroops(std::size_t troops){
    this->troops = troops;
}