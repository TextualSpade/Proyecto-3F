#include "Blob.hpp"
#include <cmath>

Blob::Blob(float x, float y)
    : Enemigo({x, y}, 110.0f, 16.0f, 3, 0, sf::Color(180, 60, 200)) {
}

void Blob::actualizar(float dt) {
    // 1) Vector del enemigo hacia Isaac
    sf::Vector2f dir = objetivo - posicion;

    // 2) Magnitud del vector (Pitagoras)
    float longitud = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    // 3) Normalizar (vector unitario) y avanzar a velocidad constante
    if (longitud > 1.0f) {       
        dir /= longitud;
        posicion += dir * velocidad * dt;
    }

    forma.setPosition(posicion);
}