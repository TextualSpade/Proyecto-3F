#include "Blob.hpp"
#include <cmath>

Blob::Blob(float x, float y)
    : Enemigo({x, y}, 110.0f, 40.0f, 3, 0, sf::Color::Transparent) {
    estadoAnim = EstadoAnim::Walk;
    tiempoParada = 1.5f;
    cronometroParada = 0.0f;
    tiempoHurt = 0.5f;
    cronometroHurt = 0.0f;
    velocidadNormal = velocidad;
    animMuerteTerminada = false;
    animacion.cargar("assets/images/blob_sheet.png", FRAME_W, FRAME_H);
    animacion.establecer(FILA_WALK, 6, 0.1f, true);
}

void Blob::cambiarEstado(EstadoAnim nuevo) {
    if (estadoAnim == nuevo) return;
    estadoAnim = nuevo;
    switch (nuevo) {
        case EstadoAnim::Walk:
            animacion.establecer(FILA_WALK, 6, 0.1f, true);
            break;
        case EstadoAnim::Attack:
            animacion.establecer(FILA_ATTACK, 5, 0.08f, false);
            break;
        case EstadoAnim::Reposo:
            animacion.establecer(FILA_IDLE, 3, 0.2f, true);
            break;
        case EstadoAnim::Hurt:
            animacion.establecer(FILA_HURT, 2, 0.1f, false);
            break;
        case EstadoAnim::Death:
            animacion.establecer(FILA_DEATH, 6, 0.12f, false);
            break;
    }
}

void Blob::actualizar(float dt) {
    if (estadoAnim == EstadoAnim::Death) {
        animacion.actualizar(dt);
        if (animacion.termino()) animMuerteTerminada = true;
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        return;
    }

    if (estaMuerto()) {
        cambiarEstado(EstadoAnim::Death);
        return;
    }

    if (cronometroHurt > 0.0f) {
        cronometroHurt -= dt;
        if (cronometroHurt <= 0.0f) velocidad = velocidadNormal;
    }

    if (estadoAnim == EstadoAnim::Attack) {
        animacion.actualizar(dt);
        if (animacion.termino()) {
            cambiarEstado(EstadoAnim::Reposo);
            cronometroParada = tiempoParada;
        }
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estadoAnim == EstadoAnim::Reposo) {
        cronometroParada -= dt;
        if (cronometroParada <= 0.0f) cambiarEstado(EstadoAnim::Walk);
        animacion.actualizar(dt);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estadoAnim == EstadoAnim::Hurt) {
        animacion.actualizar(dt);
        if (animacion.termino()) cambiarEstado(EstadoAnim::Walk);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    sf::Vector2f dir = objetivo - posicion;
    float longitud = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (longitud > 1.0f) {
        dir /= longitud;
        posicion += dir * velocidad * dt;
    }

    if (posicion.x < 40.0f + radio)  posicion.x = 40.0f + radio;
    if (posicion.x > 760.0f - radio) posicion.x = 760.0f - radio;
    if (posicion.y < 40.0f + radio)  posicion.y = 40.0f + radio;
    if (posicion.y > 560.0f - radio) posicion.y = 560.0f - radio;

    animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
    animacion.actualizar(dt);
    forma.setPosition(posicion);
}

void Blob::dibujar(sf::RenderWindow& ventana) {
    animacion.dibujar(ventana);
}

void Blob::recibirDanio(int cantidad) {
    if (estadoAnim == EstadoAnim::Death) return;
    cambiarEstado(EstadoAnim::Hurt);
    velocidad = velocidadNormal * 0.3f;
    cronometroHurt = tiempoHurt;
    Enemigo::recibirDanio(cantidad);
}

void Blob::tocarJugador() {
    if (estadoAnim == EstadoAnim::Walk) cambiarEstado(EstadoAnim::Attack);
}

bool Blob::listo_para_eliminar() const {
    return animMuerteTerminada;
}
