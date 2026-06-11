#include "Raptor.hpp"
#include <cmath>
#include <cstdlib>

Raptor::Raptor(float x, float y)
    : Enemigo({x, y}, 90.0f, 15.0f, 2, 0, sf::Color(230, 180, 60)) {
    tiempoRecarga = 2.0f;
    cronometro = 0.0f;
    tiempoCambioDir = 1.0f;
    cronometroDir = 0.0f;
    elegirDireccionAleatoria();
}

void Raptor::elegirDireccionAleatoria() {
    float angulo = (std::rand() % 360) * 3.14159265f / 180.0f;
    dirMovimiento = {std::cos(angulo), std::sin(angulo)};
}

void Raptor::actualizar(float dt) {
    cronometroDir += dt;
    if (cronometroDir >= tiempoCambioDir) {
        elegirDireccionAleatoria();
        cronometroDir = 0.0f;
    }

    posicion += dirMovimiento * velocidad * dt;

    bool rebote = false;
    if (posicion.x < 40.0f + radio)  { posicion.x = 40.0f + radio;  rebote = true; }
    if (posicion.x > 760.0f - radio) { posicion.x = 760.0f - radio; rebote = true; }
    if (posicion.y < 40.0f + radio)  { posicion.y = 40.0f + radio;  rebote = true; }
    if (posicion.y > 560.0f - radio) { posicion.y = 560.0f - radio; rebote = true; }
    if (rebote) {
        elegirDireccionAleatoria();
        cronometroDir = 0.0f;
    }

    forma.setPosition(posicion);
    cronometro += dt;
}

bool Raptor::intentaDisparar(sf::Vector2f& dirSalida) {
    if (cronometro < tiempoRecarga) return false;

    sf::Vector2f dir = objetivo - posicion;
    float longitud = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (longitud < 1.0f) return false;

    dirSalida = dir / longitud;
    cronometro = 0.0f;
    return true;
}