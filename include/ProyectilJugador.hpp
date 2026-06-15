#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>

class ProyectilJugador {
private:
    sf::Texture textura;
    std::optional<sf::Sprite> sprite;
    sf::Vector2f posicion;
    sf::Vector2f direccion;
    float velocidad;
    float radio;
    bool destruido;

public:
    ProyectilJugador(sf::Vector2f posInicial, sf::Vector2f dir,
                     const std::string& rutaSprite);

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);
    void destruir();

    bool estaDestruido() const;
    sf::Vector2f getPosicion() const;
    float getRadio() const;
};
