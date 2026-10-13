#include <Entity.hpp>

Entity::Entity(std::string name, Color color): name(name), color(color) {}

void Entity::setTarget(const Target& target){
    this->target = target;
}

void Entity::setTroops(std::size_t troops){
    this->troops = troops;
}

void Entity::setAttackFuel(std::size_t fuel){
    this->attack_fuel = fuel;
}

void Entity::beginAttack(float ratio){
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    const std::size_t committed =
        static_cast<std::size_t>(static_cast<float>(troops) * ratio);
    this->attack_fuel += committed;
    this->troops -= committed;
}

void Entity::endAttack(){
    // Devuelve al territorio el combustible que no se haya gastado.
    this->troops += this->attack_fuel;
    this->attack_fuel = 0;
    this->target = Target::none();
}