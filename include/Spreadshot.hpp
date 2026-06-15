#pragma once
#include "Enemigo.hpp"
#include "Animacion.hpp"
#include <vector>

class Spreadshot : public Enemigo {
private:
    Animacion animacion;

    enum class Estado { Idle, Atacando, Hurt, Teleportando, Death };
    Estado estado;

    float tiempoRecarga;
    float cronometro;
    float distanciaSeguridad;

    float tiempoHurt;
    bool invulnerable;

    float tiempoTeleport;
    float cronometroTeleport;
    bool pendienteTeleport;

    bool animMuerteTerminada;
    bool listoParaDisparar;
    std::vector<sf::Vector2f> disparosPendientes;

    static const int FRAME_W = 128;
    static const int FRAME_H = 128;

    static const int FILA_IDLE   = 0;
    static const int FILA_ATTACK = 3;
    static const int FILA_HURT   = 4;
    static const int FILA_DEATH  = 5;

    void cambiarEstado(Estado nuevo);
    void teletransportar();

public:
    Spreadshot(float x, float y);

    void actualizar(float dt) override;
    void dibujar(sf::RenderWindow& ventana) override;
    void recibirDanio(int cantidad) override;
    bool intentaDisparar(sf::Vector2f& dirSalida) override;
    bool listo_para_eliminar() const;
};