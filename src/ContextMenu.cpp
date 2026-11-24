#include "ContextMenu.hpp"
#include "ui/UiSprites.hpp"
#include "system/ResourceManager.hpp"
#include <algorithm>


namespace{
    bool drawMenuIcon(const char* textureKey, Rectangle item, Color tint){
        Texture2D* texture = ResourceManager::getTexture(textureKey);
        if (texture == nullptr || texture->id == 0) {
            return false;
        }

        constexpr float iconSize = 22.0f;

        DrawTexturePro(
            *texture,
            Rectangle{0.0f, 0.0f,static_cast<float>(texture->width),static_cast<float>(texture->height)},
            Rectangle{item.x + 8.0f,item.y + (item.height - iconSize) * 0.5f,iconSize,iconSize},
            Vector2{0.0f, 0.0f},
            0.0f,
            tint
        );

        return true;
    }
}

void ContextMenu::open(int cellIndex, Vector2 mouse, int screenWidth, int screenHeight){
    is_open = true;
    this->cellIndex = cellIndex;
    origin = mouse;
    hoveredItem = -1;

    //ancho panel
    constexpr float W = 190.0f;
    //alto de cada boton
    constexpr float ITEM_H = 34.0f;
    //margen
    constexpr float PADDING = 6.0f;
    //distancia del click a la esquina
    constexpr float OFFSET = 4.0f;
    //alto cabecera
    constexpr float HEADER_H = 22.0f;

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
ContextMenu::Action ContextMenu::handleInput(bool canAttack, bool canDonate, bool canAlly){
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
            if(canDonate){
                return Action::Donate;
            }
        }

        if(CheckCollisionPointRec(mouse, allianceItem)){
            if(canAlly){
                return Action::Alliance;
            }
        }

        if(!CheckCollisionPointRec(mouse, bounds)){
            close();
            return Action::None;
        }

        return Action::None;
    }

    //se cierra con el escape o con click derecho
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ESCAPE)){
        close();
        return Action::None;
    }

    return Action::None;
}


//datos para dibujar el menu
void ContextMenu::render(const char* title,int attackPercent,int attackTroops,bool canAttack,int donateTroops, bool canAlly, const char* allyLabel) const{
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
    DrawText(title != nullptr ? title : "",static_cast<int>(bounds.x + 8.0f),static_cast<int>(bounds.y + 5.0f),14, LIGHTGRAY);
    //linea que separa los items del titulo
    const float separatorY = attackItem.y - 3.0f;
    DrawLine(static_cast<int>(bounds.x + 6.0f),static_cast<int>(separatorY),static_cast<int>(bounds.x + bounds.width - 6.0f),static_cast<int>(separatorY),Color{70, 76, 86, 255});   

    //atacar
    const bool hoverAttack =canAttack && CheckCollisionPointRec(mouse, attackItem);
    DrawRectangleRec(attackItem,hoverAttack ? Color{45, 120, 60, 255} : Color{40, 46, 56, 255});
    const bool hasSword = drawMenuIcon(UiSprites::Sword,attackItem,canAttack ? WHITE : Fade(WHITE, 0.35f));
    DrawText("Atacar",static_cast<int>(attackItem.x + 8.0f + (hasSword ? 28.0f : 0.0f)),static_cast<int>(attackItem.y + 7.0f),16,canAttack ? RAYWHITE : Fade(RAYWHITE, 0.35f));
        
    //donar
    const bool canDonate = donateTroops > 0;
    const bool hoverDonate =canDonate && CheckCollisionPointRec(mouse, donateItem);
    DrawRectangleRec(donateItem, hoverDonate ? Color{45, 90, 140, 255} : Color{40, 46, 56, 255});
    const bool hasMoney = drawMenuIcon(UiSprites::Money,donateItem,canDonate ? WHITE : Fade(WHITE, 0.35f));
    DrawText("Donar",static_cast<int>(donateItem.x + 8.0f + (hasMoney ? 28.0f : 0.0f)),static_cast<int>(donateItem.y + 8.0f),16,canDonate ? RAYWHITE : Fade(RAYWHITE, 0.35f));

    //alianza
    const bool hoverAlliance =canAlly && CheckCollisionPointRec(mouse, allianceItem);
    DrawRectangleRec(allianceItem,hoverAlliance ? Color{90, 70, 140, 255} : Color{40, 46, 56, 255});
    const bool hasHandshake = drawMenuIcon(UiSprites::Handshake,allianceItem,canAlly ? WHITE : Fade(WHITE, 0.35f));
    DrawText(allyLabel != nullptr ? allyLabel : "Alianza",static_cast<int>(allianceItem.x + 8.0f + (hasHandshake ? 28.0f : 0.0f)),static_cast<int>(allianceItem.y + 8.0f),16,canAlly ? RAYWHITE : Fade(RAYWHITE, 0.35f));
}
