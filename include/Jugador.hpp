#pragma once
#include <SFML/Graphics.hpp>


class Jugador {
private:
    sf::Vector2f posicion;
    float velocidad;
    float radio;
    sf::CircleShape forma;
    int vida;

public:
    Jugador(float x, float y);       

    void actualizar(float dt);             
    void dibujar(sf::RenderWindow& ventana);

    void recibirDanio(int cantidad);
    bool estaVivo() const;

    sf::Vector2f getPosicion() const;       // getters
    float getRadio() const;
    int getVida() const;
};