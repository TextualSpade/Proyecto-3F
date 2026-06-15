#pragma once
#include <SFML/Graphics.hpp>
#include "Animacion.hpp"

class ProyectilRaptor {
private:
    sf::Vector2f posicion;
    sf::Vector2f direccion;
    float velocidad;
    float radio;
    bool destruido;
    bool impacto;
    Animacion animacion;

    static const int FRAME_W = 128;
    static const int FRAME_H = 128;
    static const int FILA_VUELO   = 0;
    static const int FILA_IMPACTO = 1;

public:
    ProyectilRaptor(sf::Vector2f posInicial, sf::Vector2f dir);

    ProyectilRaptor(const ProyectilRaptor&) = delete;
    ProyectilRaptor& operator=(const ProyectilRaptor&) = delete;
    ProyectilRaptor(ProyectilRaptor&&) = default;
    ProyectilRaptor& operator=(ProyectilRaptor&&) = default;

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);
    void golpear();
    bool estaDestruido() const;
    sf::Vector2f getPosicion() const;
    float getRadio() const;
};