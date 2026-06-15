#pragma once
#include "Enemigo.hpp"
#include "Animacion.hpp"

class Blob : public Enemigo {
private:
    Animacion animacion;

    enum class EstadoAnim { Walk, Attack, Reposo, Hurt, Death };
    EstadoAnim estadoAnim;

    float tiempoParada;
    float cronometroParada;

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

    void cambiarEstado(EstadoAnim nuevo);

public:
    Blob(float x, float y);

    void actualizar(float dt) override;
    void dibujar(sf::RenderWindow& ventana) override;
    void recibirDanio(int cantidad) override;
    void tocarJugador();
    bool listo_para_eliminar() const;
};