#include "ContextMenu.hpp"
#include <algorithm>


void ContextMenu::open(int cellIndex, Vector2 mouse, int screenWidth, int screenHeight){
    is_open = true;
    this->cellIndex = cellIndex;
    origin = mouse;
    hoveredItem = -1;

    constexpr float W = 210.0f;
    constexpr float ITEM_H = 36.0f;
    constexpr float PADDING = 8.0f;
    constexpr float OFFSET = 14.0f;
    constexpr float HEADER_H = 26.0f;

    const float height = PADDING * 2.0f + HEADER_H + ITEM_H * 3.0f;

    float x = mouse.x + OFFSET;
    float y = mouse.y + OFFSET;

    //si no cabe se coloca izq/arriba
    if (x + W > static_cast<float>(screenWidth) - 8.0f){
        x = mouse.x - W - OFFSET;
    }
    if (y + height > static_cast<float>(screenHeight) - 8.0f){
        y = mouse.y - height - OFFSET;
    }


    x = std::max(8.0f, std::min(x, static_cast<float>(screenWidth) - W - 8.0f));
    y = std::max(8.0f, std::min(y, static_cast<float>(screenHeight) - height - 8.0f));

    bounds = Rectangle{ x, y, W, height};

    //para separaciones
    const float itemX = x + PADDING;
    const float itemW = W - PADDING * 2.0f;
    const float firstY = y + PADDING + HEADER_H;

    attackItem= Rectangle{ itemX, firstY, itemW, ITEM_H };
    donateItem= Rectangle{ itemX, firstY + ITEM_H, itemW, ITEM_H };
    allianceItem=Rectangle{ itemX, firstY + ITEM_H * 2.0f, itemW, ITEM_H };
}


void ContextMenu::close(){
    is_open=false;
    cellIndex=-1;
    hoveredItem=-1;
}

//dice la accion del menu que se pulsa
ContextMenu::Action ContextMenu::handleInput(bool canAttack){
    if(!is_open){
        return Action::None;
    }
    const Vector2 mouse= GetMousePosition();

    //el canAttack es para indicarle si el que le ha dado esta en la frontera y puede atacarle
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        if(CheckCollisionPointRec(mouse, attackItem)){
            if(canAttack){
                return Action::Attack;
            }
        }

        if(CheckCollisionPointRec(mouse, donateItem)){
            return Action::Donate;
        }

        if(CheckCollisionPointRec(mouse, allianceItem)){
            return Action::Alliance;
        }

        if(!CheckCollisionPointRec(mouse, bounds)){
            close();
            return Action::None;
        }

        return Action::None;
    }

    //se cierra con el escape tambien
    if (IsKeyPressed(KEY_ESCAPE)){
        close();
        return Action::None;
    }

    return Action::None;
}


//datos para dibujar el menu
void ContextMenu::render(const char* title,int attackPercent,int attackTroops,bool canAttack) const{
    if(!is_open){
        return;
    }

    const Vector2 mouse = GetMousePosition();

    //se pinta la sombra
    DrawRectangleRounded(Rectangle{ bounds.x + 2.0f, bounds.y + 2.0f,bounds.width, bounds.height },0.08f, 8, Fade(BLACK, 0.35f));
    //panel
    DrawRectangleRounded(bounds, 0.08f, 8, Color{30, 34, 42, 245});
    DrawRectangleRoundedLines(bounds, 0.08f, 8, GRAY);
    //cabecera
    DrawText(title != nullptr ? title : "",static_cast<int>(bounds.x + 10.0f),static_cast<int>(bounds.y + 8.0f),18, LIGHTGRAY);
    //linea que separa los items del titulo
    DrawLine(static_cast<int>(bounds.x + 8.0f),static_cast<int>(bounds.y + 8.0f + 26.0f - 4.0f),static_cast<int>(bounds.x + bounds.width - 8.0f),static_cast<int>(bounds.y + 8.0f + 26.0f - 4.0f),Color{70, 76, 86, 255});
    
    //atacar
    const bool hoverAttack =canAttack && CheckCollisionPointRec(mouse, attackItem);
    DrawRectangleRec(attackItem,hoverAttack ? Color{45, 120, 60, 255} : Color{40, 46, 56, 255});
    const char* attackLabel = "Atacar";
    DrawText(attackLabel,static_cast<int>(attackItem.x + 10.0f),static_cast<int>(attackItem.y + 8.0f),20,canAttack ? RAYWHITE : Fade(RAYWHITE, 0.35f));

    //donar
    DrawRectangleRec(donateItem, Color{40, 46, 56, 255});
    DrawText("Donar",static_cast<int>(donateItem.x + 10.0f),static_cast<int>(donateItem.y + 8.0f),18, Fade(RAYWHITE, 0.35f));

    //alianza
    DrawRectangleRec(allianceItem, Color{40, 46, 56, 255});
    DrawText("Alianza",static_cast<int>(allianceItem.x + 10.0f),static_cast<int>(allianceItem.y + 8.0f),18, Fade(RAYWHITE, 0.35f));

}
