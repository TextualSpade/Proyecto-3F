#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>

class Animacion {
private:
    sf::Texture textura;
    std::optional<sf::Sprite> sprite;

    int frameActual;
    int frameInicio;
    int totalFrames;
    float tiempoEntreFrames;
    float cronometro;

    int filaActual;
    int frameWidth;
    int frameHeight;
    bool terminada;
    bool enLoop;

public:
    Animacion();

    bool cargar(const std::string& ruta, int anchoFrame, int altoFrame);
    void establecer(int fila, int numFrames, float velocidad, bool loop = false);
    void establecerRango(int fila, int frameInicio, int numFrames, float velocidad, bool loop = false);
    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);
    void setPosicion(sf::Vector2f pos);
    bool termino() const;
};