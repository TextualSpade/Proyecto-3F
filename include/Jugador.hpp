#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Animacion.hpp"

class Jugador {
public:
    enum class Estado { Idle, Running, Slashing, Hurt, Dying };

private:
    sf::Vector2f posicion;
    float velocidad;
    float multiplicadorVelocidad;
    float radio;
    int vida;
    int vidaMaxima;
    float tiempoInvulnerable;
    sf::Vector2f ultimaDireccion;

    Animacion animSheet1;
    Animacion animSheet2;
    bool usandoSheet2;
    bool slashSegundo;
    bool hurtSegundo;
    bool dyingSegundo;

    Estado estadoActual;
    bool miraDerecha;

    std::string rutaProyectil;
    std::string rutaSheet1;

    void aplicarFlip();
    void cambiarEstado(Estado nuevo);
    void actualizarAnimacion(float dt);

public:
    Jugador(float x, float y,
            const std::string& sheet1,
            const std::string& sheet2,
            const std::string& proyectil);

    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);
    void dibujarHitbox(sf::RenderWindow& ventana) const;

    void recibirDanio(int cantidad);
    void reiniciar(float x, float y);
    void setPosicion(float x, float y);
    void setMultiplicadorVelocidad(float multiplicador);
    void sanar(int cantidad);
    bool estaVivo() const;
    bool esInvulnerable() const;
    bool estaDisparable() const;

    sf::Vector2f getPosicion() const;
    sf::Vector2f getCentroHitbox() const;
    sf::Vector2f getCentroDisparo() const;
    sf::Vector2f getUltimaDireccion() const;
    float getRadio() const;
    int getVida() const;
    int getVidaMaxima() const;
    const std::string& getRutaProyectil() const;
    const std::string& getRutaSheet1() const;
};
