#include "Lagrima.hpp"

Lagrima::Lagrima(sf::Vector2f posInicial, sf::Vector2f dir,
                 float vel, sf::Color color, float rad) {
    posicion = posInicial;
    direccion = dir;
    velocidad = vel;
    radio = rad;
    destruida = false;
    forma.setRadius(radio);
    forma.setFillColor(color);
    forma.setOrigin({radio, radio});
    forma.setPosition(posicion);
}

void Lagrima::actualizar(float dt) {
    posicion += direccion * velocidad * dt;
    forma.setPosition(posicion);

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