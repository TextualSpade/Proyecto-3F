#include "ProyectilJugador.hpp"
#include <cmath>

static constexpr float ANCHO_DESTINO = 64.0f;
static constexpr float ALTO_DESTINO  = 20.0f;

ProyectilJugador::ProyectilJugador(sf::Vector2f posInicial, sf::Vector2f dir,
                                   const std::string& rutaSprite)
{
    posicion  = posInicial;
    direccion = dir;
    velocidad = 500.0f;
    radio     = 32.0f;
    destruido = false;

    if (!textura.loadFromFile(rutaSprite)) return;
    textura.setSmooth(false);
    sprite.emplace(textura);
    sf::Vector2u tam = textura.getSize();
    sprite->setOrigin({tam.x / 2.0f, tam.y / 2.0f});

    float sx = ANCHO_DESTINO / static_cast<float>(tam.x);
    float sy = ALTO_DESTINO  / static_cast<float>(tam.y);

    if (dir.x < 0.0f) {
        sprite->setScale({-sx, sy});
        sprite->setRotation(sf::degrees(0.0f));
    } else if (dir.x > 0.0f) {
        sprite->setScale({sx, sy});
        sprite->setRotation(sf::degrees(0.0f));
    } else if (dir.y < 0.0f) {
        sprite->setScale({sx, sy});
        sprite->setRotation(sf::degrees(-90.0f));
    } else {
        sprite->setScale({sx, sy});
        sprite->setRotation(sf::degrees(90.0f));
    }

    sprite->setPosition(posicion);
}

void ProyectilJugador::actualizar(float dt) {
    if (destruido) return;
    posicion += direccion * velocidad * dt;
    if (sprite.has_value()) sprite->setPosition(posicion);

    if (posicion.x < 40.0f || posicion.x > 760.0f ||
        posicion.y < 40.0f || posicion.y > 560.0f) {
        destruido = true;
    }
}

void ProyectilJugador::dibujar(sf::RenderWindow& ventana) {
    if (!destruido && sprite.has_value()) ventana.draw(*sprite);
}

void ProyectilJugador::destruir() { destruido = true; }

bool ProyectilJugador::estaDestruido() const { return destruido; }
sf::Vector2f ProyectilJugador::getPosicion() const { return posicion; }
float ProyectilJugador::getRadio() const { return radio; }