#include "Lagrima.hpp"

Lagrima::Lagrima(sf::Vector2f posInicial, sf::Vector2f dir,
                 float vel, sf::Color color) {
    posicion = posInicial;
    direccion = dir;
    velocidad = vel;
    radio = 6.0f;
    destruida = false;
    forma.setRadius(radio);
    forma.setFillColor(color);
    forma.setOrigin({radio, radio});
    forma.setPosition(posicion);
}

void Lagrima::actualizar(float dt) {
    // posicion = posicion + direccion * velocidad * tiempo
    posicion += direccion * velocidad * dt;
    forma.setPosition(posicion);

    // Al tocar las paredes de la sala se marca para eliminarse
    if (posicion.x < 40.0f || posicion.x > 760.0f ||
        posicion.y < 40.0f || posicion.y > 560.0f) {
        destruida = true;
    }
}

void Lagrima::dibujar(sf::RenderWindow& ventana) {
    ventana.draw(forma);
}

void Lagrima::destruir() { destruida = true; }

bool Lagrima::estaDestruida() const { return destruida; }

sf::Vector2f Lagrima::getPosicion() const { return posicion; }
float Lagrima::getRadio() const { return radio; }