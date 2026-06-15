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
    float tiempoInvulnerable;
    sf::Vector2f ultimaDireccion;

public:
    Jugador(float x, float y);

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);

    void recibirDanio(int cantidad);
    void reiniciar(float x, float y);
    bool estaVivo() const;
    bool esInvulnerable() const;

    sf::Vector2f getPosicion() const;
    sf::Vector2f getUltimaDireccion() const;
    float getRadio() const;
    int getVida() const;
    int getVidaMaxima() const;
};