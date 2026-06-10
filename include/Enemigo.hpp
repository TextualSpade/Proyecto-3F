#pragma once
#include <SFML/Graphics.hpp>


class Enemigo {
protected:                      // visible para las clases hijas
    sf::Vector2f posicion;
    sf::Vector2f objetivo;      // posicion de Isaac (se la pasa el Juego)
    float velocidad;
    float radio;
    int vida;
    int resistencia;            // danio que absorbe en cada golpe
    sf::CircleShape forma;

public:
    Enemigo(sf::Vector2f posInicial, float vel, float r,
            int vidaInicial, int resist, sf::Color color);
    virtual ~Enemigo() = default;

    virtual void actualizar(float dt) = 0;   // virtual PURO -> clase abstracta
    virtual void dibujar(sf::RenderWindow& ventana);

    void setObjetivo(sf::Vector2f posJugador);
    void recibirDanio(int cantidad);
    bool estaMuerto() const;

    sf::Vector2f getPosicion() const;
    float getRadio() const;
    int getVida() const;
};