#pragma once
#include <SFML/Graphics.hpp>

class Lagrima {
private:
    sf::Vector2f posicion;
    sf::Vector2f direccion;   // hacia donde vuela (fija desde que nace)
    float velocidad;
    float radio;
    sf::CircleShape forma;
    bool destruida;           // bandera: "ya no sirvo, eliminenme"

public:
    // velocidad y color tienen valores por defecto: si no se indican,
    // se crea la lagrima clasica del jugador (rapida y cyan)
    Lagrima(sf::Vector2f posInicial, sf::Vector2f dir,
            float vel = 500.0f, sf::Color color = sf::Color::Cyan);

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);

    void destruir();
    bool estaDestruida() const;

    sf::Vector2f getPosicion() const;
    float getRadio() const;
};