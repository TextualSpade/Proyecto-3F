#include "Jugador.hpp"
#include <cmath>

Jugador::Jugador(float x, float y) {
    posicion = {x, y};
    velocidad = 300.0f;
    radio = 20.0f;
    vidaMaxima = 3;
    vida = vidaMaxima;
    tiempoInvulnerable = 0.0f;
    ultimaDireccion = {1.0f, 0.0f};
    forma.setRadius(radio);
    forma.setFillColor(sf::Color::Red);
    forma.setOrigin({radio, radio});
    forma.setPosition(posicion);
}

void Jugador::actualizar(float dt) {
    sf::Vector2f movimiento = {0.0f, 0.0f};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movimiento.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movimiento.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movimiento.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movimiento.x += 1.0f;

    if (movimiento.x != 0.0f || movimiento.y != 0.0f) {
        float longitud = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= longitud;
        ultimaDireccion = movimiento;
        posicion += movimiento * velocidad * dt;
    }

    if (posicion.x < 40.0f + radio)  posicion.x = 40.0f + radio;
    if (posicion.x > 760.0f - radio) posicion.x = 760.0f - radio;
    if (posicion.y < 40.0f + radio)  posicion.y = 40.0f + radio;
    if (posicion.y > 560.0f - radio) posicion.y = 560.0f - radio;

    forma.setPosition(posicion);

    if (tiempoInvulnerable > 0.0f) {
        tiempoInvulnerable -= dt;
    }
}

void Jugador::dibujar(sf::RenderWindow& ventana) {
    if (esInvulnerable()) {
        forma.setFillColor(sf::Color(255, 100, 100, 130));
    } else {
        forma.setFillColor(sf::Color::Red);
    }
    ventana.draw(forma);
}

void Jugador::recibirDanio(int cantidad) {
    if (esInvulnerable()) return;
    vida -= cantidad;
    if (vida < 0) vida = 0;
    tiempoInvulnerable = 1.0f;
}

void Jugador::reiniciar(float x, float y) {
    posicion = {x, y};
    forma.setPosition(posicion);
    vida = vidaMaxima;
    tiempoInvulnerable = 0.0f;
    ultimaDireccion = {1.0f, 0.0f};
}

bool Jugador::estaVivo() const { return vida > 0; }
bool Jugador::esInvulnerable() const { return tiempoInvulnerable > 0.0f; }

sf::Vector2f Jugador::getPosicion() const { return posicion; }
sf::Vector2f Jugador::getUltimaDireccion() const { return ultimaDireccion; }
float Jugador::getRadio() const { return radio; }
int Jugador::getVida() const { return vida; }
int Jugador::getVidaMaxima() const { return vidaMaxima; }