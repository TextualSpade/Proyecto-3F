#include "Spreadshot.hpp"
#include <cmath>
#include <cstdlib>

namespace {
    const int CANTIDAD_DISPAROS = 3;
    const float ANGULO_ABANICO = 60.0f * 3.14159265f / 180.0f;

    sf::Vector2f rotar(sf::Vector2f v, float angulo) {
        float c = std::cos(angulo);
        float s = std::sin(angulo);
        return { v.x * c - v.y * s, v.x * s + v.y * c };
    }
}

Spreadshot::Spreadshot(float x, float y)
    : Enemigo({x, y}, 0.0f, 20.0f, 3, 0, sf::Color::Transparent) {
    estado = Estado::Idle;
    tiempoRecarga = 3.0f;
    cronometro = 0.0f;
    distanciaSeguridad = 150.0f;
    tiempoHurt = 0.5f;
    invulnerable = false;
    tiempoTeleport = 0.4f;
    cronometroTeleport = 0.0f;
    animMuerteTerminada = false;
    listoParaDisparar = false;
    animacion.cargar("assets/images/spreadshot_sheet.png", FRAME_W, FRAME_H);
    animacion.establecer(FILA_IDLE, 3, 0.15f, true);
    teletransportar();
}

void Spreadshot::teletransportar() {
    const float centroX = 400.0f;
    const float centroY = 300.0f;
    const float umbral = 150.0f;

    float x, y;
    int intentos = 0;
    do {
        x = (40.0f + FRAME_W / 2.0f) + static_cast<float>(std::rand() % static_cast<int>(720.0f - FRAME_W));
        y = (40.0f + FRAME_H / 2.0f) + static_cast<float>(std::rand() % static_cast<int>(520.0f - FRAME_H));
        intentos++;
    } while (std::sqrt((x-centroX)*(x-centroX) + (y-centroY)*(y-centroY)) < umbral && intentos < 20);

    posicion = {x, y};
    forma.setPosition(posicion);
}

void Spreadshot::cambiarEstado(Estado nuevo) {
    estado = nuevo;
    switch (nuevo) {
        case Estado::Idle:
            animacion.establecer(FILA_IDLE, 3, 0.15f, true);
            break;
        case Estado::Atacando:
            animacion.establecer(FILA_ATTACK, 4, 0.1f, false);
            listoParaDisparar = false;
            break;
        case Estado::Hurt:
            animacion.establecer(FILA_HURT, 2, 0.1f, false);
            invulnerable = true;
            break;
        case Estado::Teleportando:
            animacion.establecerRango(FILA_DEATH, 2, 1, 99.0f, false);
            cronometroTeleport = 0.0f;
            break;
        case Estado::Death:
            animacion.establecer(FILA_DEATH, 6, 0.12f, false);
            break;
    }
}

void Spreadshot::actualizar(float dt) {
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

    if (estado == Estado::Teleportando) {
        cronometroTeleport += dt;
        if (cronometroTeleport >= tiempoTeleport) {
            teletransportar();
            invulnerable = false;
            cronometro = 0.0f;
            cambiarEstado(Estado::Idle);
        }
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        return;
    }

    if (estado == Estado::Hurt) {
        animacion.actualizar(dt);
        if (animacion.termino()) cambiarEstado(Estado::Teleportando);
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    if (estado == Estado::Atacando) {
        animacion.actualizar(dt);
        if (animacion.termino()) {
            listoParaDisparar = true;
            cambiarEstado(Estado::Teleportando);
        }
        animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
        forma.setPosition(posicion);
        return;
    }

    sf::Vector2f dirHuida = posicion - objetivo;
    float longitud = std::sqrt(dirHuida.x * dirHuida.x + dirHuida.y * dirHuida.y);
    if (longitud < distanciaSeguridad) {
        cambiarEstado(Estado::Teleportando);
        return;
    }

    cronometro += dt;
    if (cronometro >= tiempoRecarga) {
        cambiarEstado(Estado::Atacando);
        cronometro = 0.0f;
    }

    animacion.actualizar(dt);
    animacion.setPosicion(posicion - sf::Vector2f(FRAME_W / 2.0f, FRAME_H / 2.0f));
    forma.setPosition(posicion);
}

void Spreadshot::dibujar(sf::RenderWindow& ventana) {
    animacion.dibujar(ventana);
}

void Spreadshot::recibirDanio(int cantidad) {
    if (estado == Estado::Death || invulnerable) return;
    cambiarEstado(Estado::Hurt);
    Enemigo::recibirDanio(cantidad);
}

bool Spreadshot::intentaDisparar(sf::Vector2f& dirSalida) {
    if (!disparosPendientes.empty()) {
        dirSalida = disparosPendientes.back();
        disparosPendientes.pop_back();
        return true;
    }

    if (!listoParaDisparar) return false;
    listoParaDisparar = false;

    sf::Vector2f dirBase = objetivo - posicion;
    float longitud = std::sqrt(dirBase.x * dirBase.x + dirBase.y * dirBase.y);
    if (longitud < 1.0f) return false;
    dirBase /= longitud;

    float incremento = ANGULO_ABANICO / (CANTIDAD_DISPAROS - 1);
    float anguloInicial = -ANGULO_ABANICO / 2.0f;

    for (int i = 0; i < CANTIDAD_DISPAROS; i++) {
        float angulo = anguloInicial + incremento * i;
        disparosPendientes.push_back(rotar(dirBase, angulo));
    }

    dirSalida = disparosPendientes.back();
    disparosPendientes.pop_back();
    return true;
}

bool Spreadshot::listo_para_eliminar() const {
    return animMuerteTerminada;
}