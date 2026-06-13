#pragma once
#include "Enemigo.hpp"
#include <vector>

class Spreadshot : public Enemigo {
private:
    float tiempoRecarga;
    float cronometro;

    float tiempoTeletransporte;
    float cronometroTeleport;
    float distanciaSeguridad;

    std::vector<sf::Vector2f> disparosPendientes;

    void teletransportar();

public:
    Spreadshot(float x, float y);

    void actualizar(float dt) override;
    bool intentaDisparar(sf::Vector2f& dirSalida) override;

    float velocidadProyectil() const override;
    sf::Color colorProyectil() const override;
};