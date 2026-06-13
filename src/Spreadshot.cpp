#include "Spreadshot.hpp"
#include <cmath>
#include <cstdlib>

namespace {
    const int CANTIDAD_DISPAROS = 10;
    const float ANGULO_ABANICO = 90.0f * 3.14159265f / 180.0f;

    sf::Vector2f rotar(sf::Vector2f v, float angulo) {
        float c = std::cos(angulo);
        float s = std::sin(angulo);
        return { v.x * c - v.y * s, v.x * s + v.y * c };
    }
}

Spreadshot::Spreadshot(float x, float y)
    : Enemigo({x, y}, 0.0f, 16.0f, 3, 0, sf::Color(210, 90, 160)) {
    tiempoRecarga = 3.0f;
    cronometro = 0.0f;
    tiempoTeletransporte = 2.5f;
    cronometroTeleport = 0.0f;
    distanciaSeguridad = 150.0f;
}

void Spreadshot::teletransportar() {
    const float centroX = 400.0f;
    const float centroY = 300.0f;
    const float umbral = 150.0f;

    float x, y;
    int intentos = 0;
    do {
        x = 40.0f + radio + static_cast<float>(std::rand() % static_cast<int>(720.0f - 2.0f * radio));
        y = 40.0f + radio + static_cast<float>(std::rand() % static_cast<int>(520.0f - 2.0f * radio));
        intentos++;
    } while (std::sqrt((x - centroX) * (x - centroX) + (y - centroY) * (y - centroY)) < umbral
             && intentos < 20);

    posicion = {x, y};
    forma.setPosition(posicion);
}

void Spreadshot::actualizar(float dt) {
    cronometro += dt;
    cronometroTeleport += dt;

    sf::Vector2f dir = objetivo - posicion;
    float longitud = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (cronometroTeleport >= tiempoTeletransporte || longitud < distanciaSeguridad) {
        teletransportar();
        cronometroTeleport = 0.0f;
    }
}

bool Spreadshot::intentaDisparar(sf::Vector2f& dirSalida) {
    if (!disparosPendientes.empty()) {
        dirSalida = disparosPendientes.back();
        disparosPendientes.pop_back();
        return true;
    }

    if (cronometro < tiempoRecarga) return false;

    sf::Vector2f dirBase = objetivo - posicion;
    float longitud = std::sqrt(dirBase.x * dirBase.x + dirBase.y * dirBase.y);
    if (longitud < 1.0f) return false;
    dirBase /= longitud;

    float mitadAbanico = ANGULO_ABANICO / 2.0f;

    for (int i = 0; i < CANTIDAD_DISPAROS; i++) {
        float aleatorio = (std::rand() % 2001) / 1000.0f - 1.0f;
        float angulo = aleatorio * mitadAbanico;
        disparosPendientes.push_back(rotar(dirBase, angulo));
    }

    cronometro = 0.0f;
    dirSalida = disparosPendientes.back();
    disparosPendientes.pop_back();
    return true;
}

float Spreadshot::velocidadProyectil() const { return 125.0f; }
sf::Color Spreadshot::colorProyectil() const { return sf::Color(255, 80, 200); }