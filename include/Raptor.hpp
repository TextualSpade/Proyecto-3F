#pragma once
#include "Enemigo.hpp"

class Raptor : public Enemigo {
private:
    float tiempoRecarga;
    float cronometro;
    sf::Vector2f dirMovimiento;
    float tiempoCambioDir;
    float cronometroDir;

    void elegirDireccionAleatoria();

public:
    Raptor(float x, float y);

    void actualizar(float dt) override;
    bool intentaDisparar(sf::Vector2f& dirSalida) override;
};