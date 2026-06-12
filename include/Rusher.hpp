#pragma once
#include "Enemigo.hpp"

class Rusher : public Enemigo {
private:
    enum class Estado { Apuntando, Embistiendo, Aturdido };

    Estado estado;
    sf::Vector2f dirEmbestida;
    float velocidadEmbestida;
    float cronometro;
    float tiempoApuntando;
    float tiempoAturdido;

public:
    Rusher(float x, float y);

    void actualizar(float dt) override;
};