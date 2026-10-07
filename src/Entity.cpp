#include <Entity.hpp>

Entity::Entity(std::string name, Color color): name(name), color(color), target(nullptr) {}

void Entity::setTarget(Entity* target){
    this->target = target;
}

void Entity::setTroops(std::size_t troops){
    this->troops = troops;
}