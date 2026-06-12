#include "Rusher.hpp"
#include <cmath>

Rusher::Rusher(float x, float y)
    : Enemigo({x, y}, 60.0f, 18.0f, 4, 0, sf::Color(80, 170, 90)) {
    estado = Estado::Apuntando;
    dirEmbestida = {0.0f, 0.0f};
    velocidadEmbestida = 450.0f;
    cronometro = 0.0f;
    tiempoApuntando = 1.2f;
    tiempoAturdido = 0.8f;
}

void Rusher::actualizar(float dt) {
    cronometro += dt;

    if (estado == Estado::Apuntando) {
        sf::Vector2f dir = objetivo - posicion;
        float longitud = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (longitud > 1.0f) {
            dir /= longitud;
            posicion += dir * velocidad * dt;
        }
        forma.setFillColor(sf::Color(80, 170, 90));

        if (cronometro >= tiempoApuntando && longitud > 1.0f) {
            dirEmbestida = dir;
            estado = Estado::Embistiendo;
            cronometro = 0.0f;
        }
    }
    else if (estado == Estado::Embistiendo) {
        posicion += dirEmbestida * velocidadEmbestida * dt;
        forma.setFillColor(sf::Color(140, 255, 150));

        bool choque = false;
        if (posicion.x < 40.0f + radio)  { posicion.x = 40.0f + radio;  choque = true; }
        if (posicion.x > 760.0f - radio) { posicion.x = 760.0f - radio; choque = true; }
        if (posicion.y < 40.0f + radio)  { posicion.y = 40.0f + radio;  choque = true; }
        if (posicion.y > 560.0f - radio) { posicion.y = 560.0f - radio; choque = true; }

        if (choque) {
            estado = Estado::Aturdido;
            cronometro = 0.0f;
        }
    }
    else if (estado == Estado::Aturdido) {
        forma.setFillColor(sf::Color(50, 100, 60));

        if (cronometro >= tiempoAturdido) {
            estado = Estado::Apuntando;
            cronometro = 0.0f;
        }
    }

    forma.setPosition(posicion);
}