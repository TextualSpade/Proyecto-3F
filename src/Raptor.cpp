#include "Raptor.hpp"
#include <cmath>
#include <cstdlib>

Raptor::Raptor(float x, float y)
    : Enemigo({x, y}, 90.0f, 20.0f, 2, 0, sf::Color::Transparent) {
    estadoAnim = EstadoAnim::Walk;
    tiempoRecarga = 1.0f;
    cronometro = 0.0f;
    tiempoHurt = 0.5f;
    cronometroHurt = 0.0f;
    velocidadNormal = velocidad;
    animMuerteTerminada = false;
    listoParaDisparar = false;
    tiempoCambioDir = 1.5f;
    cronometroDir = 0.0f;
    elegirDireccionAleatoria();
    animacion.cargar("assets/images/raptor_sheet.png", FRAME_W, FRAME_H);
    animacion.establecer(FILA_WALK, 4, 0.1f, true);
}

void Raptor::elegirDireccionAleatoria() {
    float angulo = (std::rand() % 360) * 3.14159265f / 180.0f;
    dirMovimiento = {std::cos(angulo), std::sin(angulo)};
}

void Raptor::cambiarEstado(EstadoAnim nuevo) {
    if (estadoAnim == nuevo) return;
    estadoAnim = nuevo;
    switch (nuevo) {
        case EstadoAnim::Idle:
            animacion.establecer(FILA_IDLE, 3, 0.15f, true);
            break;
        case EstadoAnim::Walk:
            animacion.establecer(FILA_WALK, 4, 0.1f, true);
            break;
        case EstadoAnim::Attack:
            animacion.establecer(FILA_ATTACK, 3, 0.1f, false);
            listoParaDisparar = false;
            break;
        case EstadoAnim::Hurt:
            animacion.establecer(FILA_HURT, 2, 0.1f, false);
            break;
        case EstadoAnim::Death:
            animacion.establecer(FILA_DEATH, 4, 0.12f, false);
            break;
    }
}

void Raptor::actualizar(float dt) {
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

    if (estadoAnim == EstadoAnim::Hurt) {
        animacion.actualizar(dt);
        if (animacion.termino()) cambiarEstado(EstadoAnim::Walk);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estadoAnim == EstadoAnim::Attack) {
        animacion.actualizar(dt);
        if (animacion.termino()) {
            listoParaDisparar = true;
            cronometro = 0.0f;
            cambiarEstado(EstadoAnim::Walk);
        }
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    cronometro += dt;
    if (cronometro >= tiempoRecarga) {
        cambiarEstado(EstadoAnim::Attack);
        return;
    }

    cronometroDir += dt;
    if (cronometroDir >= tiempoCambioDir) {
        elegirDireccionAleatoria();
        cronometroDir = 0.0f;
    }

    posicion += dirMovimiento * velocidad * dt;

    bool rebote = false;
    if (posicion.x < 40.0f + radio)  { posicion.x = 40.0f + radio;  rebote = true; }
    if (posicion.x > 760.0f - radio) { posicion.x = 760.0f - radio; rebote = true; }
    if (posicion.y < 40.0f + radio)  { posicion.y = 40.0f + radio;  rebote = true; }
    if (posicion.y > 560.0f - radio) { posicion.y = 560.0f - radio; rebote = true; }
    if (rebote) {
        elegirDireccionAleatoria();
        cronometroDir = 0.0f;
    }

    cambiarEstado(EstadoAnim::Walk);
    animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
    animacion.actualizar(dt);
    forma.setPosition(posicion);
}

void Raptor::dibujar(sf::RenderWindow& ventana) {
    animacion.dibujar(ventana);
}

void Raptor::recibirDanio(int cantidad) {
    if (estadoAnim == EstadoAnim::Death) return;
    cambiarEstado(EstadoAnim::Hurt);
    velocidad = velocidadNormal * 0.3f;
    cronometroHurt = tiempoHurt;
    Enemigo::recibirDanio(cantidad);
}

bool Raptor::intentaDisparar(sf::Vector2f& dirSalida) {
    if (!listoParaDisparar) return false;
    listoParaDisparar = false;
    sf::Vector2f dir = objetivo - posicion;
    float longitud = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (longitud < 1.0f) return false;
    dirSalida = dir / longitud;
    return true;
}

bool Raptor::listo_para_eliminar() const {
    return animMuerteTerminada;
}