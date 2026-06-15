#include "ProyectilSpreadshot.hpp"

ProyectilSpreadshot::ProyectilSpreadshot(sf::Vector2f posInicial, sf::Vector2f dir) {
    posicion = posInicial;
    direccion = dir;
    velocidad = 125.0f;
    radio = 30.0f;
    rebotesRestantes = 2;
    destruido = false;
    impacto = false;
    animacion.cargar("assets/images/spreadshot_proyectil.png", FRAME_W, FRAME_H);
    animacion.establecer(FILA_VUELO, 8, 0.1875f, false);
}

void ProyectilSpreadshot::actualizar(float dt) {
    if (destruido) return;

    if (impacto) {
        animacion.actualizar(dt);
        if (animacion.termino()) destruido = true;
        return;
    }

    posicion += direccion * velocidad * dt;
    if (!animacion.termino()) animacion.actualizar(dt);

    bool rebote = false;
    if (posicion.x < 40.0f + radio) {
        posicion.x = 40.0f + radio;
        direccion.x = -direccion.x;
        rebote = true;
    }
    if (posicion.x > 760.0f - radio) {
        posicion.x = 760.0f - radio;
        direccion.x = -direccion.x;
        rebote = true;
    }
    if (posicion.y < 40.0f + radio) {
        posicion.y = 40.0f + radio;
        direccion.y = -direccion.y;
        rebote = true;
    }
    if (posicion.y > 560.0f - radio) {
        posicion.y = 560.0f - radio;
        direccion.y = -direccion.y;
        rebote = true;
    }

    if (rebote) {
        if (rebotesRestantes > 0) {
            rebotesRestantes--;
        } else {
            destruido = true;
        }
    }

    animacion.setPosicion(posicion);
}

void ProyectilSpreadshot::dibujar(sf::RenderWindow& ventana) {
    if (!destruido) animacion.dibujar(ventana);
}

void ProyectilSpreadshot::golpear() {
    if (!impacto) {
        impacto = true;
        animacion.establecer(FILA_IMPACTO, 5, 0.08f, false);
    }
}

bool ProyectilSpreadshot::estaDestruido() const { return destruido; }
sf::Vector2f ProyectilSpreadshot::getPosicion() const { return posicion; }
float ProyectilSpreadshot::getRadio() const { return radio; }