#include <Entity.hpp>
#include <cmath>

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

    // P = tropas totales en el momento de lanzar el ataque (antes de comprometer).
    const std::size_t total = troops;
    const std::size_t committed =static_cast<std::size_t>(static_cast<float>(total) * ratio);
    this->attack_fuel += committed;
    this->troops -= committed;

    attack_resistence=1; //esta es la resistencia de la tierra
    //si el objetivo se trata de un enemigo 
    if(target.isEnemy() && target.getEntity() !=nullptr){
        const float e= static_cast<float>(target.getEntity()->getTroops());
        //se redondea como en matematicas
        long r= std::lround(e / static_cast<float>(total));  
        if (r < 1){
            r = 1;      
        }  
        attack_resistence=static_cast<std::size_t>(r);                         
    }
}

void Entity::endAttack(){
    // Devuelve al territorio el combustible que no se haya gastado.
    this->troops += this->attack_fuel;
    this->attack_fuel = 0;
    this->target = Target::none();
}