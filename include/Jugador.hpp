#pragma once
#include <SFML/Graphics.hpp>

class Jugador {
private:
    sf::Vector2f posicion;
    float velocidad;
    float radio;
    sf::CircleShape forma;
    int vida;
    int vidaMaxima;
    float tiempoInvulnerable;   // segundos restantes de inmunidad tras un golpe

public:
    Jugador(float x, float y);

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);

    void recibirDanio(int cantidad);
    bool estaVivo() const;
    bool esInvulnerable() const;

    sf::Vector2f getPosicion() const;
    float getRadio() const;
    int getVida() const;
    int getVidaMaxima() const;
};