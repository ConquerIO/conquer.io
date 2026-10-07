#pragma once

/*
    Hay una dependencia cíclica entre Entity y Target,
    por lo que se declara aquí (forward-declaration) para
    evitar que el compilador entre en bucle de errores.
*/
class Entity;

class Target {
public:
    enum class Kind : unsigned char { None, Land, Enemy };

    Target() = default;

    static Target none() { return Target(); }
    static Target land() { return Target(Kind::Land, nullptr); }
    static Target enemy(Entity* entity) { return Target(Kind::Enemy, entity); }

    Kind getKind() const { return kind; }

    bool isNone() const { return kind == Kind::None; }
    bool isLand() const { return kind == Kind::Land; }
    bool isEnemy() const { return kind == Kind::Enemy; }
    Entity* getEntity() const { return kind == Kind::Enemy ? entity : nullptr; }

private:
    Target(Kind kind, Entity* entity) : kind(kind), entity(entity) {}

    Kind kind = Kind::None;
    Entity* entity = nullptr;
};
