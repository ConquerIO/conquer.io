#pragma once

#include <raylib.h>
#include <Entity.hpp>

class Bot : public Entity
{
public:
    explicit Bot(const std::string &name , Color color);

    // Indica si toca empezar un ataque nuevo (cada 6-10 s aleatorios).
    bool shouldStartAttack(float deltaTime);
    // Cadencia de las oleadas de expansion durante un ataque.
    bool shouldExpand(float deltaTime, float interval);
    // Fraccion de tropas (0.30-0.70) elegida para el ataque en curso.
    float getAttackRatio() const { return attack_ratio; }

    void reset();

    //getters y setters para los datos del bot
    float getNextAttackDelay() const { 
        return next_attack_delay; 
    }
    float getAttackDecisionTimer() const { 
        return attack_decision_timer; 
    }
    float getExpandTimer() const { 
        return expand_timer; 
    }

private:
    float attack_decision_timer; // tiempo acumulado desde el ultimo ataque
    float next_attack_delay;     // espera aleatoria [6, 10] hasta el proximo ataque
    float expand_timer;          // temporizador de las oleadas de expansion
    float attack_ratio;          // fraccion de tropas [0.30, 0.70] del ataque actual
};
