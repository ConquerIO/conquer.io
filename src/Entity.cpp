#include <Entity.hpp>

Entity::Entity(std::string name, Color color): name(name), color(color), targetIndex(-1) {}

void Entity::setTarget(int targetIndex){
    this->targetIndex = targetIndex;
}