#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "Jugador.hpp"
#include "Lagrima.hpp"
#include "Enemigo.hpp"
#include "Blob.hpp"
#include "Raptor.hpp"
#include "Rusher.hpp"
#include "Spreadshot.hpp"
#include "ProyectilRaptor.hpp"

class Juego {
private:
    sf::RenderWindow ventana;
    std::unique_ptr<Jugador> isaac;
    std::vector<Lagrima> lagrimas;
    std::vector<Lagrima> lagrimasEnemigas;
    std::vector<std::unique_ptr<ProyectilRaptor>> proyectilesRaptor;
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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::I)) { dirDisparo = {0.0f, -1.0f}; disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K)) { dirDisparo = {0.0f, 1.0f};  disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J)) { dirDisparo = {-1.0f, 0.0f}; disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L)) { dirDisparo = {1.0f, 0.0f};  disparoPresionado = true; }

        if (disparoPresionado) {
            lagrimas.push_back(Lagrima(isaac->getPosicion(), dirDisparo));
            relojDisparo.restart();
        }
    }

    void manejarDisparosEnemigos() {
        for (auto& enemigo : enemigos) {
            sf::Vector2f dirSalida;
            if (enemigo->intentaDisparar(dirSalida)) {
                if (dynamic_cast<Raptor*>(enemigo.get())) {
                    proyectilesRaptor.push_back(
                        std::make_unique<ProyectilRaptor>(enemigo->getPosicion(), dirSalida));
                } else {
                    lagrimasEnemigas.push_back(
                        Lagrima(enemigo->getPosicion(), dirSalida,
                                enemigo->velocidadProyectil(),
                                enemigo->colorProyectil(),
                                enemigo->radioProyectil()));
                }
            }
        }
    }

    bool colisionan(sf::Vector2f posA, float radioA, sf::Vector2f posB, float radioB) {
        float dx = posA.x - posB.x;
        float dy = posA.y - posB.y;
        float sumaRadios = radioA + radioB;
        return (dx * dx + dy * dy) < (sumaRadios * sumaRadios);
    }

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

        for (auto& enemigo : enemigos) {
            if (!enemigo->estaMuerto() &&
                colisionan(enemigo->getPosicion(), enemigo->getRadio(),
                           isaac->getPosicion(), isaac->getRadio())) {
                isaac->recibirDanio(1);
                if (auto* blob = dynamic_cast<Blob*>(enemigo.get())) {
                    blob->tocarJugador();
                }
            }
        }

        for (auto& lagrima : lagrimasEnemigas) {
            if (!lagrima.estaDestruida() &&
                colisionan(lagrima.getPosicion(), lagrima.getRadio(),
                           isaac->getPosicion(), isaac->getRadio())) {
                isaac->recibirDanio(1);
                lagrima.destruir();
            }
        }

        for (auto& proy : proyectilesRaptor) {
            if (!proy->estaDestruido() &&
                colisionan(proy->getPosicion(), proy->getRadio(),
                           isaac->getPosicion(), isaac->getRadio())) {
                isaac->recibirDanio(1);
                proy->golpear();
            }
        }
    }

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

    void reiniciarPartida() {
        lagrimas.clear();
        lagrimasEnemigas.clear();
        proyectilesRaptor.clear();
        enemigos.clear();
        isaac->reiniciar(400.0f, 300.0f);
        enemigos.push_back(std::make_unique<Raptor>(650.0f, 150.0f));
    }

    void actualizar(float dt) {
        isaac->actualizar(dt);
        manejarDisparos();

        for (auto& enemigo : enemigos) {
            enemigo->setObjetivo(isaac->getPosicion());
            enemigo->actualizar(dt);
        }

        manejarDisparosEnemigos();

        for (size_t i = 0; i < lagrimas.size(); ) {
            lagrimas[i].actualizar(dt);
            if (lagrimas[i].estaDestruida()) lagrimas.erase(lagrimas.begin() + i);
            else i++;
        }

        for (size_t i = 0; i < lagrimasEnemigas.size(); ) {
            lagrimasEnemigas[i].actualizar(dt);
            if (lagrimasEnemigas[i].estaDestruida()) lagrimasEnemigas.erase(lagrimasEnemigas.begin() + i);
            else i++;
        }

        for (size_t i = 0; i < proyectilesRaptor.size(); ) {
            proyectilesRaptor[i]->actualizar(dt);
            if (proyectilesRaptor[i]->estaDestruido()) proyectilesRaptor.erase(proyectilesRaptor.begin() + i);
            else i++;
        }

        manejarColisiones();

        for (size_t i = 0; i < enemigos.size(); ) {
            bool eliminar = false;
            if (auto* blob = dynamic_cast<Blob*>(enemigos[i].get())) {
                eliminar = blob->listo_para_eliminar();
            } else if (auto* raptor = dynamic_cast<Raptor*>(enemigos[i].get())) {
                eliminar = raptor->listo_para_eliminar();
            } else {
                eliminar = enemigos[i]->estaMuerto();
            }
            if (eliminar) enemigos.erase(enemigos.begin() + i);
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
        for (auto& lagrima : lagrimasEnemigas) lagrima.dibujar(ventana);
        for (auto& proy : proyectilesRaptor) proy->dibujar(ventana);
        dibujarHUD();
        ventana.display();
    }

public:
    Juego() : ventana(sf::VideoMode({800, 600}), "The Binding of Isaac - Alpha SFML 3") {
        ventana.setFramerateLimit(60);
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        isaac = std::make_unique<Jugador>(400.0f, 300.0f);
        tiempoEntreDisparos = 0.3f;
        enemigos.push_back(std::make_unique<Raptor>(650.0f, 150.0f));
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