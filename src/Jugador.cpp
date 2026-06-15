#include "Jugador.hpp"
#include <cmath>

static constexpr int   FW          = 302;
static constexpr int   FH          = 302;
static constexpr float ESCALA      = 0.42f;
static constexpr float VEL_IDLE    = 0.08f;
static constexpr float VEL_RUN     = 0.06f;
static constexpr float VEL_SLASH   = 0.05f;
static constexpr float VEL_HURT    = 0.06f;
static constexpr float VEL_DYING   = 0.07f;

// Estas dos posiciones se separan para que no se mezclen sprite, golpe y disparo.
static constexpr sf::Vector2f OFFSET_HITBOX  = {0.0f, 8.0f};
static constexpr sf::Vector2f OFFSET_DISPARO = {0.0f, -20.0f};

Jugador::Jugador(float x, float y,
                 const std::string& sheet1,
                 const std::string& sheet2,
                 const std::string& proyectil)
{
    posicion           = {x, y};
    velocidad          = 300.0f;
    radio              = 18.0f;
    vidaMaxima         = 3;
    vida               = vidaMaxima;
    tiempoInvulnerable = 0.0f;
    ultimaDireccion    = {1.0f, 0.0f};
    usandoSheet2       = false;
    estadoActual       = Estado::Idle;
    miraDerecha        = true;
    rutaProyectil      = proyectil;
    rutaSheet1         = sheet1;
    slashSegundo       = false;
    hurtSegundo        = false;
    dyingSegundo       = false;

    animSheet1.cargar(sheet1, FW, FH);
    animSheet2.cargar(sheet2, FW, FH);
    animSheet1.setEscalaBase(ESCALA);
    animSheet2.setEscalaBase(ESCALA);

    animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
    animSheet1.setPosicion(posicion);
    aplicarFlip();
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
    slashSegundo = false;
    hurtSegundo  = false;
    dyingSegundo = false;

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
    if (usandoSheet2) {
        animSheet2.actualizar(dt);
        animSheet2.setPosicion(posicion);
    } else {
        animSheet1.actualizar(dt);
        animSheet1.setPosicion(posicion);
    }

    switch (estadoActual) {
        case Estado::Slashing:
            if (!slashSegundo && animSheet1.termino()) {
                slashSegundo = true;
                animSheet1.establecerRango(4, 10, 2, VEL_SLASH, false);
                aplicarFlip();
            } else if (slashSegundo && animSheet1.termino()) {
                slashSegundo = false;
                estadoActual = Estado::Idle;
                usandoSheet2 = false;
                animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
                aplicarFlip();
            }
            break;
        case Estado::Hurt:
            if (!hurtSegundo && animSheet2.termino()) {
                hurtSegundo = true;
                animSheet2.establecerRango(2, 10, 2, VEL_HURT, false);
                aplicarFlip();
            } else if (hurtSegundo && animSheet2.termino()) {
                hurtSegundo = false;
                estadoActual = Estado::Idle;
                usandoSheet2 = false;
                animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
                aplicarFlip();
            }
            break;
        case Estado::Dying:
            if (!dyingSegundo && animSheet2.termino()) {
                dyingSegundo = true;
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

    // Límites usando la hitbox real, no el frame completo del spritesheet.
    sf::Vector2f centro = getCentroHitbox();
    if (centro.x < 40.0f + radio)  posicion.x += (40.0f + radio) - centro.x;
    if (centro.x > 760.0f - radio) posicion.x += (760.0f - radio) - centro.x;

    centro = getCentroHitbox();
    if (centro.y < 40.0f + radio)  posicion.y += (40.0f + radio) - centro.y;
    if (centro.y > 560.0f - radio) posicion.y += (560.0f - radio) - centro.y;

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
        float t = tiempoInvulnerable * 8.0f;
        if (static_cast<int>(t) % 2 != 0) return;
    }
    if (usandoSheet2) {
        animSheet2.dibujar(ventana);
    } else {
        animSheet1.dibujar(ventana);
    }
}

void Jugador::dibujarHitbox(sf::RenderWindow& ventana) const {
    sf::CircleShape hitbox(radio);
    hitbox.setOrigin({radio, radio});
    hitbox.setPosition(getCentroHitbox());
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::Red);
    hitbox.setOutlineThickness(2.0f);
    ventana.draw(hitbox);
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
    slashSegundo       = false;
    hurtSegundo        = false;
    dyingSegundo       = false;
    estadoActual       = Estado::Idle;
    animSheet1.establecerRango(0, 0, 10, VEL_IDLE, true);
    animSheet1.setPosicion(posicion);
    aplicarFlip();
}

bool Jugador::estaVivo()       const { return vida > 0; }
bool Jugador::esInvulnerable() const { return tiempoInvulnerable > 0.0f; }
bool Jugador::estaDisparable() const { return estadoActual != Estado::Dying; }

sf::Vector2f Jugador::getPosicion() const { return posicion; }
sf::Vector2f Jugador::getCentroHitbox() const { return posicion + OFFSET_HITBOX; }
sf::Vector2f Jugador::getCentroDisparo() const { return posicion + OFFSET_DISPARO; }
sf::Vector2f Jugador::getUltimaDireccion() const { return ultimaDireccion; }
float Jugador::getRadio() const { return radio; }
int Jugador::getVida() const { return vida; }
int Jugador::getVidaMaxima() const { return vidaMaxima; }
const std::string& Jugador::getRutaProyectil() const { return rutaProyectil; }
const std::string& Jugador::getRutaSheet1() const { return rutaSheet1; }
