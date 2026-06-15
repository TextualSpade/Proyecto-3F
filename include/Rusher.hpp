#pragma once
#include "Enemigo.hpp"
#include "Animacion.hpp"

class Rusher : public Enemigo {
private:
    Animacion animacion;

    enum class Estado { Apuntando, Cargando, Embistiendo, Golpeando, Aturdido, Hurt, Death };
    Estado estado;

    sf::Vector2f dirEmbestida;
    float velocidadEmbestida;
    float cronometro;
    float tiempoApuntando;
    float tiempoCarga;
    float tiempoAturdido;

    float tiempoHurt;
    float cronometroHurt;
    float velocidadNormal;

    bool animMuerteTerminada;

    static const int FRAME_W = 256;
    static const int FRAME_H = 256;

    static const int FILA_IDLE   = 0;
    static const int FILA_WALK   = 1;
    static const int FILA_ATTACK = 2;
    static const int FILA_HURT   = 3;
    static const int FILA_DEATH  = 4;

    void cambiarEstado(Estado nuevo);

public:
    Rusher(float x, float y);

    void actualizar(float dt) override;
    void dibujar(sf::RenderWindow& ventana) override;
    void recibirDanio(int cantidad) override;
    void golpearJugador();
    bool listo_para_eliminar() const;
};