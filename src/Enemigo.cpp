#include "Enemigo.hpp"

Enemigo::Enemigo(sf::Vector2f posInicial, float vel, float r,
                 int vidaInicial, int resist, sf::Color color) {
    posicion = posInicial;
    objetivo = posInicial;      // mientras nadie le diga, su objetivo es donde nacio
    velocidad = vel;
    radio = r;
    vida = vidaInicial;
    resistencia = resist;
    forma.setRadius(radio);
    forma.setFillColor(color);
    forma.setOrigin({radio, radio});
    forma.setPosition(posicion);
}

void Enemigo::dibujar(sf::RenderWindow& ventana) {
    ventana.draw(forma);
}

void Enemigo::setObjetivo(sf::Vector2f posJugador) {
    objetivo = posJugador;
}

void Enemigo::recibirDanio(int cantidad) {
    int danioReal = cantidad - resistencia;   // la resistencia absorbe parte
    if (danioReal < 1) danioReal = 1;         // siempre entra minimo 1 de danio
    vida -= danioReal;
    if (vida < 0) vida = 0;
}

bool Enemigo::estaMuerto() const { return vida <= 0; }

sf::Vector2f Enemigo::getPosicion() const { return posicion; }
float Enemigo::getRadio() const { return radio; }
int Enemigo::getVida() const { return vida; }