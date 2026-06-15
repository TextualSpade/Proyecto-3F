#pragma once
#include "Enemigo.hpp"
#include "Animacion.hpp"

class Raptor : public Enemigo {
private:
    Animacion animacion;

    enum class EstadoAnim { Idle, Walk, Attack, Hurt, Death };
    EstadoAnim estadoAnim;

    float tiempoRecarga;
    float cronometro;

    float tiempoHurt;
    float cronometroHurt;
    float velocidadNormal;

    sf::Vector2f dirMovimiento;
    float tiempoCambioDir;
    float cronometroDir;

    bool animMuerteTerminada;
    bool listoParaDisparar;

    static const int FRAME_W = 128;
    static const int FRAME_H = 128;

    static const int FILA_IDLE   = 0;
    static const int FILA_WALK   = 1;
    static const int FILA_ATTACK = 2;
    static const int FILA_HURT   = 4;
    static const int FILA_DEATH  = 5;

    void cambiarEstado(EstadoAnim nuevo);
    void elegirDireccionAleatoria();

public:
    Raptor(float x, float y);

    void actualizar(float dt) override;
    void dibujar(sf::RenderWindow& ventana) override;
    void recibirDanio(int cantidad) override;
    bool intentaDisparar(sf::Vector2f& dirSalida) override;
    bool listo_para_eliminar() const;
};