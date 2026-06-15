#include "Jugador.hpp"
#include <cmath>

static constexpr int   FW          = 302;
static constexpr int   FH          = 302;
static constexpr float ESCALA      = 0.20f;
static constexpr float VEL_IDLE    = 0.08f;
static constexpr float VEL_RUN     = 0.06f;
static constexpr float VEL_SLASH   = 0.05f;
static constexpr float VEL_HURT    = 0.06f;
static constexpr float VEL_DYING   = 0.07f;

Jugador::Jugador(float x, float y,
                 const std::string& sheet1,
                 const std::string& sheet2,
                 const std::string& proyectil)
{
    posicion         = {x, y};
    velocidad        = 300.0f;
    radio            = 20.0f;
    vidaMaxima       = 3;
    vida             = vidaMaxima;
    tiempoInvulnerable = 0.0f;
    ultimaDireccion  = {1.0f, 0.0f};
    usandoSheet2     = false;
    estadoActual     = Estado::Idle;
    miraDerecha      = true;
    rutaProyectil    = proyectil;

    animSheet1.cargar(sheet1, FW, FH);
    animSheet2.cargar(sheet2, FW, FH);
    animSheet1.setEscalaBase(ESCALA);
    animSheet2.setEscalaBase(ESCALA);

    animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
    animSheet1.setPosicion(posicion);
}

void Jugador::aplicarFlip() {
    float e = miraDerecha ? 1.0f : -1.0f;
    animSheet1.setEscala({e, 1.0f});
    animSheet2.setEscala({e, 1.0f});
}

void Jugador::cambiarEstado(Estado nuevo) {
    if (estadoActual == nuevo) return;
    if (estadoActual == Estado::Dying) return;

    estadoActual = nuevo;
    usandoSheet2 = false;

    switch (nuevo) {
        case Estado::Idle:
            animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
            break;
        case Estado::Running:
            animSheet1.establecerRango(2, 0, 10, VEL_RUN, true);
            break;
        case Estado::Slashing:
            animSheet1.establecerRango(4, 0, 10, VEL_SLASH, false);
            break;
        case Estado::Hurt:
            usandoSheet2 = true;
            animSheet2.establecerRango(1, 0, 10, VEL_HURT, false);
            break;
        case Estado::Dying:
            usandoSheet2 = true;
            animSheet2.establecerRango(3, 0, 10, VEL_DYING, false);
            break;
    }
    aplicarFlip();
}

void Jugador::actualizarAnimacion(float dt) {
    Animacion& anim = usandoSheet2 ? animSheet2 : animSheet1;
    anim.actualizar(dt);
    anim.setPosicion(posicion);

    switch (estadoActual) {
        case Estado::Slashing:
            if (animSheet1.termino()) {
                animSheet1.establecerRango(4, 10, 2, VEL_SLASH, false);
                aplicarFlip();
            }
            if (animSheet1.termino()) {
                estadoActual = Estado::Idle;
                animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
                aplicarFlip();
            }
            break;
        case Estado::Hurt:
            if (animSheet2.termino()) {
                animSheet2.establecerRango(2, 10, 2, VEL_HURT, false);
                aplicarFlip();
            }
            if (animSheet2.termino()) {
                estadoActual = Estado::Idle;
                usandoSheet2 = false;
                animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
                aplicarFlip();
            }
            break;
        case Estado::Dying:
            if (animSheet2.termino()) {
                animSheet2.establecerRango(4, 10, 5, VEL_DYING, false);
                aplicarFlip();
            }
            break;
        default:
            break;
    }
}

void Jugador::actualizar(float dt) {
    if (estadoActual == Estado::Dying) {
        actualizarAnimacion(dt);
        return;
    }

    sf::Vector2f movimiento = {0.0f, 0.0f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movimiento.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movimiento.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movimiento.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movimiento.x += 1.0f;

    bool disparando = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::I) ||
                      sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K) ||
                      sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J) ||
                      sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L);

    bool moviendose = (movimiento.x != 0.0f || movimiento.y != 0.0f);

    if (moviendose) {
        float longitud = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= longitud;
        if (movimiento.x != 0.0f) {
            bool nuevaDerecha = (movimiento.x > 0.0f);
            if (nuevaDerecha != miraDerecha) {
                miraDerecha = nuevaDerecha;
                aplicarFlip();
            }
        }
        ultimaDireccion = movimiento;
        posicion += movimiento * velocidad * dt;
    }

    if (posicion.x < 40.0f + radio)  posicion.x = 40.0f + radio;
    if (posicion.x > 760.0f - radio) posicion.x = 760.0f - radio;
    if (posicion.y < 40.0f + radio)  posicion.y = 40.0f + radio;
    if (posicion.y > 560.0f - radio) posicion.y = 560.0f - radio;

    if (tiempoInvulnerable > 0.0f) tiempoInvulnerable -= dt;

    if (estadoActual != Estado::Hurt) {
        if (disparando) {
            cambiarEstado(Estado::Slashing);
        } else if (moviendose) {
            cambiarEstado(Estado::Running);
        } else {
            cambiarEstado(Estado::Idle);
        }
    }

    actualizarAnimacion(dt);
}

void Jugador::dibujar(sf::RenderWindow& ventana) {
    if (esInvulnerable()) {
        float t = tiempoInvulnerable * 10.0f;
        if (static_cast<int>(t) % 2 == 0) return;
    }
    Animacion& anim = usandoSheet2 ? animSheet2 : animSheet1;
    anim.dibujar(ventana);
}

void Jugador::recibirDanio(int cantidad) {
    if (esInvulnerable() || estadoActual == Estado::Dying) return;
    vida -= cantidad;
    if (vida < 0) vida = 0;
    tiempoInvulnerable = 1.0f;
    if (vida == 0) {
        cambiarEstado(Estado::Dying);
    } else {
        cambiarEstado(Estado::Hurt);
    }
}

void Jugador::reiniciar(float x, float y) {
    posicion           = {x, y};
    vida               = vidaMaxima;
    tiempoInvulnerable = 0.0f;
    ultimaDireccion    = {1.0f, 0.0f};
    miraDerecha        = true;
    usandoSheet2       = false;
    estadoActual       = Estado::Idle;
    animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
    animSheet1.setPosicion(posicion);
    aplicarFlip();
}

bool Jugador::estaVivo()      const { return vida > 0; }
bool Jugador::esInvulnerable() const { return tiempoInvulnerable > 0.0f; }
bool Jugador::estaDisparable() const { return estadoActual != Estado::Dying; }

sf::Vector2f Jugador::getPosicion()        const { return posicion; }
sf::Vector2f Jugador::getUltimaDireccion() const { return ultimaDireccion; }
float        Jugador::getRadio()           const { return radio; }
int          Jugador::getVida()            const { return vida; }
int          Jugador::getVidaMaxima()      const { return vidaMaxima; }
const std::string& Jugador::getRutaProyectil() const { return rutaProyectil; }
const std::string& Jugador::getRutaSheet1()    const { return rutaSheet1; }
