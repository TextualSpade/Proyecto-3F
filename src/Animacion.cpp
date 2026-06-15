#include "Animacion.hpp"

Animacion::Animacion() {
    frameActual = 0;
    totalFrames = 1;
    tiempoEntreFrames = 0.1f;
    cronometro = 0.0f;
    filaActual = 0;
    frameWidth = 0;
    frameHeight = 0;
    terminada = false;
    enLoop = false;
}

bool Animacion::cargar(const std::string& ruta, int anchoFrame, int altoFrame) {
    if (!textura.loadFromFile(ruta)) return false;
    frameWidth = anchoFrame;
    frameHeight = altoFrame;
    sprite.emplace(textura);
    sprite->setTextureRect(sf::IntRect({0, 0}, {frameWidth, frameHeight}));
    return true;
}

void Animacion::establecer(int fila, int numFrames, float velocidad, bool loop) {
    if (!sprite.has_value()) return;
    filaActual = fila;
    totalFrames = numFrames;
    tiempoEntreFrames = velocidad;
    frameActual = 0;
    cronometro = 0.0f;
    terminada = false;
    enLoop = loop;
    sprite->setTextureRect(sf::IntRect({0, filaActual * frameHeight}, {frameWidth, frameHeight}));
}

void Animacion::actualizar(float dt) {
    if (!sprite.has_value() || terminada) return;
    cronometro += dt;
    if (cronometro >= tiempoEntreFrames) {
        cronometro = 0.0f;
        if (frameActual < totalFrames - 1) {
            frameActual++;
        } else {
            if (enLoop) {
                frameActual = 0;
            } else {
                terminada = true;
            }
        }
        sprite->setTextureRect(sf::IntRect(
            {frameActual * frameWidth, filaActual * frameHeight},
            {frameWidth, frameHeight}
        ));
    }
}

void Animacion::dibujar(sf::RenderWindow& ventana) {
    if (sprite.has_value()) ventana.draw(*sprite);
}

void Animacion::setPosicion(sf::Vector2f pos) {
    if (sprite.has_value()) sprite->setPosition(pos);
}

bool Animacion::termino() const { return terminada; }

void Animacion::refrescar() {
    if (frameWidth == 0) return;
    sprite.emplace(textura);
    sprite->setTextureRect(sf::IntRect(
        {frameActual * frameWidth, filaActual * frameHeight},
        {frameWidth, frameHeight}
    ));
}