#include "ProyectilJugador.hpp"
#include <cmath>
#include <utility>

static constexpr float ANCHO_DESTINO = 64.0f;
static constexpr float ALTO_DESTINO  = 20.0f;

ProyectilJugador::ProyectilJugador(sf::Vector2f posInicial, sf::Vector2f dir,
                                   const std::string& rutaSprite)
    : sprite(textura)
{
    posicion  = posInicial;
    direccion = dir;
    velocidad = 500.0f;
    radio     = 8.0f;
    destruido = false;
    cargado   = false;
    escalaX   = 1.0f;
    escalaY   = 1.0f;
    rotacion  = 0.0f;

    if (!textura.loadFromFile(rutaSprite)) return;
    cargado = true;

    sf::Vector2u tam = textura.getSize();
    sprite.setTexture(textura, true);
    sprite.setOrigin({tam.x / 2.0f, tam.y / 2.0f});

    float sx = ANCHO_DESTINO / static_cast<float>(tam.x);
    float sy = ALTO_DESTINO  / static_cast<float>(tam.y);

    // El proyectil puede salir en diagonal cuando la estrella activa el disparo triple.
    // Antes solo se revisaba si dir.x era positivo/negativo, por eso al disparar
    // hacia arriba o abajo las lanzas diagonales quedaban horizontales.
    escalaX = sx;
    escalaY = sy;
    rotacion = std::atan2(dir.y, dir.x) * 180.0f / 3.14159265f;

    sprite.setScale({escalaX, escalaY});
    sprite.setRotation(sf::degrees(rotacion));
    sprite.setPosition(posicion);
}

ProyectilJugador::ProyectilJugador(ProyectilJugador&& other) noexcept
    : textura(std::move(other.textura))
    , sprite(textura)
{
    posicion  = other.posicion;
    direccion = other.direccion;
    velocidad = other.velocidad;
    radio     = other.radio;
    destruido = other.destruido;
    cargado   = other.cargado;
    escalaX   = other.escalaX;
    escalaY   = other.escalaY;
    rotacion  = other.rotacion;

    if (cargado) {
        sf::Vector2u tam = textura.getSize();
        sprite.setTexture(textura, true);
        sprite.setOrigin({tam.x / 2.0f, tam.y / 2.0f});
        sprite.setScale({escalaX, escalaY});
        sprite.setRotation(sf::degrees(rotacion));
        sprite.setPosition(posicion);
    }
    other.cargado = false;
    other.destruido = true;
}

ProyectilJugador& ProyectilJugador::operator=(ProyectilJugador&& other) noexcept {
    if (this == &other) return *this;
    textura   = std::move(other.textura);
    posicion  = other.posicion;
    direccion = other.direccion;
    velocidad = other.velocidad;
    radio     = other.radio;
    destruido = other.destruido;
    cargado   = other.cargado;
    escalaX   = other.escalaX;
    escalaY   = other.escalaY;
    rotacion  = other.rotacion;

    if (cargado) {
        sf::Vector2u tam = textura.getSize();
        sprite.setTexture(textura, true);
        sprite.setOrigin({tam.x / 2.0f, tam.y / 2.0f});
        sprite.setScale({escalaX, escalaY});
        sprite.setRotation(sf::degrees(rotacion));
        sprite.setPosition(posicion);
    }
    other.cargado = false;
    other.destruido = true;
    return *this;
}

void ProyectilJugador::actualizar(float dt) {
    if (destruido) return;
    posicion += direccion * velocidad * dt;
    sprite.setPosition(posicion);

    if (posicion.x < 40.0f || posicion.x > 760.0f ||
        posicion.y < 40.0f || posicion.y > 560.0f) {
        destruido = true;
    }
}

void ProyectilJugador::dibujar(sf::RenderWindow& ventana) {
    if (!destruido && cargado) ventana.draw(sprite);
}

void ProyectilJugador::destruir() { destruido = true; }

bool ProyectilJugador::estaDestruido() const { return destruido; }
sf::Vector2f ProyectilJugador::getPosicion() const { return posicion; }
float ProyectilJugador::getRadio() const { return radio; }
