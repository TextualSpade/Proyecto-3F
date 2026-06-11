#pragma once
#include <SFML/Graphics.hpp>

class Enemigo {
protected:
    sf::Vector2f posicion;
    sf::Vector2f objetivo;
    float velocidad;
    float radio;
    int vida;
    int resistencia;
    sf::CircleShape forma;

public:
    Enemigo(sf::Vector2f posInicial, float vel, float r,
            int vidaInicial, int resist, sf::Color color);
    virtual ~Enemigo() = default;

    virtual void actualizar(float dt) = 0;
    virtual void dibujar(sf::RenderWindow& ventana);

    virtual bool intentaDisparar(sf::Vector2f& dirSalida) { return false; }

    void setObjetivo(sf::Vector2f posJugador);
    void recibirDanio(int cantidad);
    bool estaMuerto() const;

    sf::Vector2f getPosicion() const;
    float getRadio() const;
    int getVida() const;
};