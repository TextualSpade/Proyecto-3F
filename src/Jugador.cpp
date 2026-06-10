#include "Jugador.hpp"

Jugador::Jugador(float x, float y) {
    posicion = {x, y};
    velocidad = 300.0f;
    radio = 20.0f;
    vida = 3;
    forma.setRadius(radio);
    forma.setFillColor(sf::Color::Red);
    forma.setOrigin({radio, radio});
    forma.setPosition(posicion);
}

void Jugador::actualizar(float dt) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) posicion.y -= velocidad * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) posicion.y += velocidad * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) posicion.x -= velocidad * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) posicion.x += velocidad * dt;

    // Limites de la sala (paredes)
    if (posicion.x < 40.0f + radio)  posicion.x = 40.0f + radio;
    if (posicion.x > 760.0f - radio) posicion.x = 760.0f - radio;
    if (posicion.y < 40.0f + radio)  posicion.y = 40.0f + radio;
    if (posicion.y > 560.0f - radio) posicion.y = 560.0f - radio;

    forma.setPosition(posicion);
}

void Jugador::dibujar(sf::RenderWindow& ventana) {
    ventana.draw(forma);
}

void Jugador::recibirDanio(int cantidad) {
    vida -= cantidad;
    if (vida < 0) vida = 0;
}

bool Jugador::estaVivo() const { return vida > 0; }

sf::Vector2f Jugador::getPosicion() const { return posicion; }
float Jugador::getRadio() const { return radio; }
int Jugador::getVida() const { return vida; }