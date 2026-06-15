#pragma once
#include <SFML/Graphics.hpp>
#include "Animacion.hpp"

class ProyectilSpreadshot {
private:
    sf::Vector2f posicion;
    sf::Vector2f direccion;
    float velocidad;
    float radio;
    int rebotesRestantes;
    bool destruido;
    bool impacto;
    Animacion animacion;

    static const int FRAME_W = 256;
    static const int FRAME_H = 256;
    static const int FILA_VUELO   = 0;
    static const int FILA_IMPACTO = 1;

public:
    ProyectilSpreadshot(sf::Vector2f posInicial, sf::Vector2f dir);

    ProyectilSpreadshot(const ProyectilSpreadshot&) = delete;
    ProyectilSpreadshot& operator=(const ProyectilSpreadshot&) = delete;
    ProyectilSpreadshot(ProyectilSpreadshot&&) = default;
    ProyectilSpreadshot& operator=(ProyectilSpreadshot&&) = default;

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);
    void golpear();
    bool estaDestruido() const;
    sf::Vector2f getPosicion() const;
    float getRadio() const;
};