#include "Rusher.hpp"
#include <cmath>

Rusher::Rusher(float x, float y)
    : Enemigo({x, y}, 30.0f, 40.0f, 12, 0, sf::Color::Transparent) {
    estado = Estado::Apuntando;
    dirEmbestida = {0.0f, 0.0f};
    velocidadEmbestida = 540.0f;
    cronometro = 0.0f;
    tiempoApuntando = 1.5f;
    tiempoCarga = 0.7f;
    tiempoAturdido = 1.0f;
    tiempoHurt = 0.5f;
    cronometroHurt = 0.0f;
    velocidadNormal = velocidad;
    animMuerteTerminada = false;
    animacion.cargar("assets/images/rusher_sheet.png", FRAME_W, FRAME_H);
    animacion.establecer(FILA_WALK, 6, 0.1f, true);
}

void Rusher::cambiarEstado(Estado nuevo) {
    estado = nuevo;
    switch (nuevo) {
        case Estado::Apuntando:
            animacion.establecer(FILA_WALK, 6, 0.1f, true);
            cronometro = 0.0f;
            break;
        case Estado::Cargando:
            animacion.establecerRango(FILA_ATTACK, 0, 2, 0.35f, false);
            cronometro = 0.0f;
            break;
        case Estado::Embistiendo:
            animacion.establecerRango(FILA_ATTACK, 0, 2, 0.1f, false);
            break;
        case Estado::Golpeando:
            animacion.establecerRango(FILA_ATTACK, 2, 2, 0.1f, false);
            break;
        case Estado::Aturdido:
            animacion.establecer(FILA_IDLE, 3, 0.15f, true);
            cronometro = 0.0f;
            break;
        case Estado::Hurt:
            animacion.establecer(FILA_HURT, 2, 0.1f, false);
            break;
        case Estado::Death:
            animacion.establecer(FILA_DEATH, 6, 0.12f, false);
            break;
    }
}

void Rusher::actualizar(float dt) {
    if (estado == Estado::Death) {
        animacion.actualizar(dt);
        if (animacion.termino()) animMuerteTerminada = true;
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        return;
    }

    if (estaMuerto()) {
        cambiarEstado(Estado::Death);
        return;
    }

    if (cronometroHurt > 0.0f) {
        cronometroHurt -= dt;
        if (cronometroHurt <= 0.0f) velocidad = velocidadNormal;
    }

    if (estado == Estado::Hurt) {
        animacion.actualizar(dt);
        if (animacion.termino()) cambiarEstado(Estado::Apuntando);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estado == Estado::Golpeando) {
        animacion.actualizar(dt);
        if (animacion.termino()) cambiarEstado(Estado::Aturdido);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estado == Estado::Apuntando) {
        sf::Vector2f dir = objetivo - posicion;
        float longitud = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (longitud > 1.0f) {
            dir /= longitud;
            posicion += dir * velocidad * dt;
        }
        cronometro += dt;
        if (cronometro >= tiempoApuntando) {
            sf::Vector2f d = objetivo - posicion;
            if (std::abs(d.x) > std::abs(d.y)) {
                dirEmbestida = {d.x > 0 ? 1.0f : -1.0f, 0.0f};
            } else {
                dirEmbestida = {0.0f, d.y > 0 ? 1.0f : -1.0f};
            }
            cambiarEstado(Estado::Cargando);
        }
        animacion.actualizar(dt);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estado == Estado::Cargando) {
        cronometro += dt;
        if (cronometro >= tiempoCarga) {
            cambiarEstado(Estado::Embistiendo);
        }
        animacion.actualizar(dt);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estado == Estado::Embistiendo) {
        posicion += dirEmbestida * velocidadEmbestida * dt;

        if (!animacion.termino()) animacion.actualizar(dt);

        bool choque = false;
        if (posicion.x < 40.0f + radio)  { posicion.x = 40.0f + radio;  choque = true; }
        if (posicion.x > 760.0f - radio) { posicion.x = 760.0f - radio; choque = true; }
        if (posicion.y < 40.0f + radio)  { posicion.y = 40.0f + radio;  choque = true; }
        if (posicion.y > 560.0f - radio) { posicion.y = 560.0f - radio; choque = true; }

        if (choque) cambiarEstado(Estado::Aturdido);

        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estado == Estado::Aturdido) {
        cronometro += dt;
        if (cronometro >= tiempoAturdido) cambiarEstado(Estado::Apuntando);
        animacion.actualizar(dt);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }
}

void Rusher::dibujar(sf::RenderWindow& ventana) {
    animacion.dibujar(ventana);
}

void Rusher::recibirDanio(int cantidad) {
    if (estado == Estado::Death) return;
    if (estado == Estado::Embistiendo || estado == Estado::Cargando) return;
    cambiarEstado(Estado::Hurt);
    velocidad = velocidadNormal * 0.3f;
    cronometroHurt = tiempoHurt;
    Enemigo::recibirDanio(cantidad);
}

void Rusher::golpearJugador() {
    if (estado == Estado::Embistiendo) cambiarEstado(Estado::Golpeando);
}

bool Rusher::listo_para_eliminar() const {
    return animMuerteTerminada;
}