#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class ProyectilJugador {
private:
    sf::Texture textura;
    sf::Sprite sprite;
    sf::Vector2f posicion;
    sf::Vector2f direccion;
    float velocidad;
    float radio;
    bool destruido;
    bool cargado;

    float escalaX;
    float escalaY;
    float rotacion;

public:
    ProyectilJugador(sf::Vector2f posInicial, sf::Vector2f dir,
                     const std::string& rutaSprite);

    ProyectilJugador(const ProyectilJugador&) = delete;
    ProyectilJugador& operator=(const ProyectilJugador&) = delete;
    ProyectilJugador(ProyectilJugador&&) noexcept;
    ProyectilJugador& operator=(ProyectilJugador&&) noexcept;

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);
    void destruir();

    bool estaDestruido() const;
    sf::Vector2f getPosicion() const;
    float getRadio() const;
};
