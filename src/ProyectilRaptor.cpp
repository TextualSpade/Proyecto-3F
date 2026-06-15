#include "ProyectilRaptor.hpp"

ProyectilRaptor::ProyectilRaptor(sf::Vector2f posInicial, sf::Vector2f dir) {
    posicion = posInicial;
    direccion = dir;
    velocidad = 200.0f;
    radio = 20.0f;
    destruido = false;
    impacto = false;
    formado = false;
    animacion.cargar("assets/images/raptor_proyectil.png", FRAME_W, FRAME_H);
    animacion.establecer(FILA_VUELO, 3, 0.67f, false);
}

void ProyectilRaptor::actualizar(float dt) {
    if (destruido) return;

    if (impacto) {
        animacion.actualizar(dt);
        if (animacion.termino()) destruido = true;
        return;
    }

    if (!formado) {
        animacion.actualizar(dt);
        if (animacion.termino()) {
            formado = true;
            animacion.establecer(FILA_VUELO_FORMADO, 1, 99.0f, false);
        }
    }

    posicion += direccion * velocidad * dt;
    animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));

    if (posicion.x < 40.0f || posicion.x > 760.0f ||
        posicion.y < 40.0f || posicion.y > 560.0f) {
        destruido = true;
    }
}

void ProyectilRaptor::dibujar(sf::RenderWindow& ventana) {
    if (!destruido) animacion.dibujar(ventana);
}

void ProyectilRaptor::golpear() {
    if (!impacto) {
        impacto = true;
        animacion.establecer(FILA_IMPACTO, 5, 0.08f, false);
    }
}

bool ProyectilRaptor::estaDestruido() const { return destruido; }
sf::Vector2f ProyectilRaptor::getPosicion() const { return posicion; }
float ProyectilRaptor::getRadio() const { return radio; }