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

    virtual float radioProyectil() const { return 6.0f; }
    virtual float velocidadProyectil() const { return 250.0f; }
    virtual sf::Color colorProyectil() const { return sf::Color(255, 140, 0); }

    void setObjetivo(sf::Vector2f posJugador);
    virtual void recibirDanio(int cantidad);
    bool estaMuerto() const;

    sf::Vector2f getPosicion() const;
    float getRadio() const;
    int getVida() const;
};