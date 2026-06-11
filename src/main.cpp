#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>
#include "Jugador.hpp"
#include "Lagrima.hpp"
#include "Enemigo.hpp"
#include "Blob.hpp"

class Juego {
private:
    sf::RenderWindow ventana;
    std::unique_ptr<Jugador> isaac;
    std::vector<Lagrima> lagrimas;
    std::vector<std::unique_ptr<Enemigo>> enemigos;
    sf::Clock reloj;
    sf::Clock relojDisparo;
    float tiempoEntreDisparos;

    void procesarEventos() {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) ventana.close();
        }
    }

    void manejarDisparos() {
        if (relojDisparo.getElapsedTime().asSeconds() < tiempoEntreDisparos) return;
        sf::Vector2f dirDisparo{0.0f, 0.0f};
        bool disparoPresionado = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    { dirDisparo = {0.0f, -1.0f}; disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  { dirDisparo = {0.0f, 1.0f};  disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  { dirDisparo = {-1.0f, 0.0f}; disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) { dirDisparo = {1.0f, 0.0f};  disparoPresionado = true; }

        if (disparoPresionado) {
            lagrimas.push_back(Lagrima(isaac->getPosicion(), dirDisparo));
            relojDisparo.restart();
        }
    }

    // Dos circulos chocan si la distancia entre centros < suma de radios
    bool colisionan(sf::Vector2f posA, float radioA, sf::Vector2f posB, float radioB) {
        float dx = posA.x - posB.x;
        float dy = posA.y - posB.y;
        float sumaRadios = radioA + radioB;
        return (dx * dx + dy * dy) < (sumaRadios * sumaRadios);
    }

    // Revisa cada lagrima contra cada enemigo
    void manejarColisiones() {
        for (auto& lagrima : lagrimas) {
            for (auto& enemigo : enemigos) {
                if (!lagrima.estaDestruida() && !enemigo->estaMuerto() &&
                    colisionan(lagrima.getPosicion(), lagrima.getRadio(),
                               enemigo->getPosicion(), enemigo->getRadio())) {
                    enemigo->recibirDanio(1);
                    lagrima.destruir();
                }
            }
        }

        // Enemigos contra Isaac (danio por contacto)
        for (auto& enemigo : enemigos) {
            if (!enemigo->estaMuerto() &&
                colisionan(enemigo->getPosicion(), enemigo->getRadio(),
                           isaac->getPosicion(), isaac->getRadio())) {
                isaac->recibirDanio(1);
            }
        }
    }

    // Corazones de vida: cuadros rojos (vida actual) y grises (vida perdida)
    void dibujarHUD() {
        for (int i = 0; i < isaac->getVidaMaxima(); i++) {
            sf::RectangleShape corazon({22.0f, 22.0f});
            corazon.setPosition({50.0f + i * 30.0f, 8.0f});
            if (i < isaac->getVida()) {
                corazon.setFillColor(sf::Color(220, 40, 40));
            } else {
                corazon.setFillColor(sf::Color(80, 80, 80));
            }
            ventana.draw(corazon);
        }
    }

    // Cuando Isaac muere: todo vuelve a empezar
    void reiniciarPartida() {
        lagrimas.clear();
        enemigos.clear();
        isaac->reiniciar(400.0f, 300.0f);
        enemigos.push_back(std::make_unique<Blob>(150.0f, 150.0f));
    }

    void actualizar(float dt) {
        isaac->actualizar(dt);
        manejarDisparos();

        for (auto& enemigo : enemigos) {
            enemigo->setObjetivo(isaac->getPosicion());
            enemigo->actualizar(dt);
        }

        for (size_t i = 0; i < lagrimas.size(); ) {
            lagrimas[i].actualizar(dt);
            if (lagrimas[i].estaDestruida()) lagrimas.erase(lagrimas.begin() + i);
            else i++;
        }

        manejarColisiones();

        // Sacar de la lista a los enemigos que murieron
        for (size_t i = 0; i < enemigos.size(); ) {
            if (enemigos[i]->estaMuerto()) enemigos.erase(enemigos.begin() + i);
            else i++;
        }

        if (!isaac->estaVivo()) reiniciarPartida();
    }

    void renderizar() {
        ventana.clear(sf::Color(35, 35, 35));
        sf::RectangleShape paredes({720.0f, 520.0f});
        paredes.setPosition({40.0f, 40.0f});
        paredes.setFillColor(sf::Color::Transparent);
        paredes.setOutlineThickness(5.0f);
        paredes.setOutlineColor(sf::Color(70, 70, 70));
        ventana.draw(paredes);
        isaac->dibujar(ventana);
        for (auto& enemigo : enemigos) enemigo->dibujar(ventana);
        for (auto& lagrima : lagrimas) lagrima.dibujar(ventana);
        dibujarHUD();
        ventana.display();
    }

public:
    Juego() : ventana(sf::VideoMode({800, 600}), "The Binding of Isaac - Alpha SFML 3") {
        ventana.setFramerateLimit(60);
        isaac = std::make_unique<Jugador>(400.0f, 300.0f);
        tiempoEntreDisparos = 0.3f;
        enemigos.push_back(std::make_unique<Blob>(150.0f, 150.0f));
    }

    void ejecutar() {
        while (ventana.isOpen()) {
            float dt = reloj.restart().asSeconds();
            procesarEventos();
            actualizar(dt);
            renderizar();
        }
    }
};

int main() {
    Juego miJuego;
    miJuego.ejecutar();
    return 0;
}