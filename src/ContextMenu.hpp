#pragma once
#include <raylib.h>

class ContextMenu {

public:
    enum class Action {
        None,
        Attack,
        Donate,
        Alliance
    };

    void open(int cellIndex, Vector2 mouse, int screenWidth, int screenHeight);
    void close();

    bool isOpen() const { 
        return is_open; 
    }
    int getCellIndex() const { 
        return cellIndex; 
    }

    //dice la accion del menu que se pulsa
    Action handleInput(bool canAttack, bool canDonate);
    //datos para dibujar el menu
    void render(const char* title,int attackPercent,int attackTroops,bool canAttack, int donateTroops) const;

private:
    bool is_open = false;
    int cellIndex = -1;

    Vector2 origin{};
    Rectangle bounds{};
    Rectangle attackItem{};
    Rectangle donateItem{};
    Rectangle allianceItem{};

    int hoveredItem = -1;
};