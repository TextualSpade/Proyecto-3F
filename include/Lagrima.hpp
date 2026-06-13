#pragma once
#include <SFML/Graphics.hpp>

class Lagrima {
private:
    sf::Vector2f posicion;
    sf::Vector2f direccion;
    float velocidad;
    float radio;
    sf::CircleShape forma;
    bool destruida;

public:
    Lagrima(sf::Vector2f posInicial, sf::Vector2f dir,
            float vel = 500.0f, sf::Color color = sf::Color::Cyan,
            float rad = 6.0f);

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);

    void destruir();
    bool estaDestruida() const;

    sf::Vector2f getPosicion() const;
    float getRadio() const;
};