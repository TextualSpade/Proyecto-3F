#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "Jugador.hpp"
#include "ProyectilJugador.hpp"
#include "Lagrima.hpp"
#include "Enemigo.hpp"
#include "Blob.hpp"
#include "Raptor.hpp"
#include "Rusher.hpp"
#include "Spreadshot.hpp"
#include "ProyectilRaptor.hpp"
#include "ProyectilSpreadshot.hpp"

struct ConfigPersonaje {
    std::string sheet1;
    std::string sheet2;
    std::string proyectil;
    std::string nombre;
};

static const ConfigPersonaje PERSONAJES[3] = {
    { "assets/images/personaje1_sheet1.png",
      "assets/images/personaje1_sheet2.png",
      "assets/images/personaje1_sword.png",
      "Ing. en Sistemas" },
    { "assets/images/personaje2_sheet1.png",
      "assets/images/personaje2_sheet2.png",
      "assets/images/personaje2_sword.png",
      "Ing. en Sistemas" },
    { "assets/images/personaje3_sheet1.png",
      "assets/images/personaje3_sheet2.png",
      "assets/images/personaje3_sword.png",
      "Ing. en Sistemas" },
};

static constexpr int   FRAME_W     = 302;
static constexpr int   FRAME_H     = 302;
static constexpr int   FILA_SLASH  = 4;
static constexpr int   FRAME_SLASH = 9;
static constexpr float ESCALA_SEL  = 0.45f;

class Juego {
private:
    sf::RenderWindow ventana;
    std::unique_ptr<Jugador> isaac;
    std::vector<ProyectilJugador> proyectilesJugador;
    std::vector<Lagrima> lagrimasEnemigas;
    std::vector<std::unique_ptr<ProyectilRaptor>> proyectilesRaptor;
    std::vector<std::unique_ptr<ProyectilSpreadshot>> proyectilesSpreadshot;
    std::vector<std::unique_ptr<Enemigo>> enemigos;
    sf::Clock reloj;
    sf::Clock relojDisparo;
    float tiempoEntreDisparos;
    int personajeElegido;

    int seleccionarPersonaje() {
        sf::Font fuente;
        bool hayFuente = fuente.openFromFile("assets/fonts/MedievalSharp.ttf");
        if (!hayFuente) hayFuente = fuente.openFromFile("assets/fonts/arial.ttf");

        sf::Texture texPersonajes[3];
        sf::Sprite  sprPersonajes[3] = {
            sf::Sprite(texPersonajes[0]),
            sf::Sprite(texPersonajes[1]),
            sf::Sprite(texPersonajes[2])
        };

        for (int i = 0; i < 3; i++) {
            if (texPersonajes[i].loadFromFile(PERSONAJES[i].sheet1)) {
                sprPersonajes[i].setTexture(texPersonajes[i], true);
                sprPersonajes[i].setTextureRect(sf::IntRect(
                    {FRAME_SLASH * FRAME_W, FILA_SLASH * FRAME_H},
                    {FRAME_W, FRAME_H}
                ));
                sprPersonajes[i].setOrigin({FRAME_W / 2.0f, FRAME_H / 2.0f});
                sprPersonajes[i].setScale({ESCALA_SEL, ESCALA_SEL});
            }
        }

        int seleccion = 0;

        while (ventana.isOpen()) {
            while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
                if (evento->is<sf::Event::Closed>()) {
                    ventana.close();
                    return -1;
                }
                if (const auto* kp = evento->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::Num1) return 0;
                    if (kp->code == sf::Keyboard::Key::Num2) return 1;
                    if (kp->code == sf::Keyboard::Key::Num3) return 2;
                    if (kp->code == sf::Keyboard::Key::Left)  seleccion = (seleccion + 2) % 3;
                    if (kp->code == sf::Keyboard::Key::Right) seleccion = (seleccion + 1) % 3;
                    if (kp->code == sf::Keyboard::Key::Enter) return seleccion;
                }
            }

            ventana.clear(sf::Color(15, 10, 20));

            sf::RectangleShape fondo({800.0f, 600.0f});
            fondo.setFillColor(sf::Color(15, 10, 20));
            ventana.draw(fondo);

            if (hayFuente) {
                sf::Text titulo(fuente, "Elige tu aventurero", 32);
                titulo.setFillColor(sf::Color(210, 170, 50));
                titulo.setOutlineColor(sf::Color(80, 50, 10));
                titulo.setOutlineThickness(2.0f);
                auto tb = titulo.getLocalBounds();
                titulo.setOrigin({tb.size.x / 2.0f, 0.0f});
                titulo.setPosition({400.0f, 50.0f});
                ventana.draw(titulo);
            }

            for (int i = 0; i < 3; i++) {
                float cx = 160.0f + i * 240.0f;
                float cy = 280.0f;

                sf::RectangleShape caja({180.0f, 220.0f});
                caja.setOrigin({90.0f, 110.0f});
                caja.setPosition({cx, cy});
                caja.setFillColor(i == seleccion
                    ? sf::Color(60, 45, 15)
                    : sf::Color(30, 25, 35));
                caja.setOutlineThickness(3.0f);
                caja.setOutlineColor(i == seleccion
                    ? sf::Color(210, 170, 50)
                    : sf::Color(70, 60, 80));
                ventana.draw(caja);

                sprPersonajes[i].setPosition({cx, cy - 10.0f});
                ventana.draw(sprPersonajes[i]);

                if (hayFuente) {
                    sf::Text nombre(fuente, PERSONAJES[i].nombre, 15);
                    nombre.setFillColor(i == seleccion
                        ? sf::Color(210, 170, 50)
                        : sf::Color(180, 160, 130));
                    auto nb = nombre.getLocalBounds();
                    nombre.setOrigin({nb.size.x / 2.0f, 0.0f});
                    nombre.setPosition({cx, cy + 120.0f});
                    ventana.draw(nombre);

                    sf::Text tecla(fuente, "[" + std::to_string(i + 1) + "]", 13);
                    tecla.setFillColor(sf::Color(140, 120, 80));
                    auto tk = tecla.getLocalBounds();
                    tecla.setOrigin({tk.size.x / 2.0f, 0.0f});
                    tecla.setPosition({cx, cy + 142.0f});
                    ventana.draw(tecla);
                }
            }

            if (hayFuente) {
                sf::Text instruccion(fuente, "Flechas / Enter   o   teclas 1  2  3", 14);
                instruccion.setFillColor(sf::Color(120, 110, 90));
                auto ib = instruccion.getLocalBounds();
                instruccion.setOrigin({ib.size.x / 2.0f, 0.0f});
                instruccion.setPosition({400.0f, 520.0f});
                ventana.draw(instruccion);
            }

            ventana.display();
        }
        return 0;
    }

    void procesarEventos() {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) ventana.close();
        }
    }

    void manejarDisparos() {
        if (!isaac->estaDisparable()) return;
        if (relojDisparo.getElapsedTime().asSeconds() < tiempoEntreDisparos) return;

        sf::Vector2f dirDisparo{0.0f, 0.0f};
        bool disparoPresionado = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::I)) { dirDisparo = {0.0f, -1.0f}; disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K)) { dirDisparo = {0.0f,  1.0f}; disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J)) { dirDisparo = {-1.0f, 0.0f}; disparoPresionado = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L)) { dirDisparo = {1.0f,  0.0f}; disparoPresionado = true; }

        if (disparoPresionado) {
            proyectilesJugador.emplace_back(
                isaac->getPosicion(), dirDisparo,
                isaac->getRutaProyectil());
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
                } else if (dynamic_cast<Spreadshot*>(enemigo.get())) {
                    proyectilesSpreadshot.push_back(
                        std::make_unique<ProyectilSpreadshot>(enemigo->getPosicion(), dirSalida));
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
        float sr = radioA + radioB;
        return (dx * dx + dy * dy) < (sr * sr);
    }

    void manejarColisiones() {
        for (auto& proy : proyectilesJugador) {
            for (auto& enemigo : enemigos) {
                if (!proy.estaDestruido() && !enemigo->estaMuerto() &&
                    colisionan(proy.getPosicion(), proy.getRadio(),
                               enemigo->getPosicion(), enemigo->getRadio())) {
                    enemigo->recibirDanio(1);
                    proy.destruir();
                }
            }
        }

        for (auto& enemigo : enemigos) {
            if (!enemigo->estaMuerto() &&
                colisionan(enemigo->getPosicion(), enemigo->getRadio(),
                           isaac->getPosicion(), isaac->getRadio())) {
                isaac->recibirDanio(1);
                if (auto* blob = dynamic_cast<Blob*>(enemigo.get())) blob->tocarJugador();
                if (auto* rusher = dynamic_cast<Rusher*>(enemigo.get())) rusher->golpearJugador();
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

        for (auto& proy : proyectilesSpreadshot) {
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
            corazon.setFillColor(i < isaac->getVida()
                ? sf::Color(220, 40, 40)
                : sf::Color(80, 80, 80));
            ventana.draw(corazon);
        }
    }

    void reiniciarPartida() {
        proyectilesJugador.clear();
        lagrimasEnemigas.clear();
        proyectilesRaptor.clear();
        proyectilesSpreadshot.clear();
        enemigos.clear();
        isaac->reiniciar(400.0f, 300.0f);
        enemigos.push_back(std::make_unique<Blob>(150.0f, 150.0f));
        enemigos.push_back(std::make_unique<Raptor>(650.0f, 150.0f));
        enemigos.push_back(std::make_unique<Rusher>(650.0f, 450.0f));
        enemigos.push_back(std::make_unique<Spreadshot>(150.0f, 450.0f));
    }

    void actualizar(float dt) {
        isaac->actualizar(dt);
        manejarDisparos();

        for (auto& enemigo : enemigos) {
            enemigo->setObjetivo(isaac->getPosicion());
            enemigo->actualizar(dt);
        }

        manejarDisparosEnemigos();

        for (size_t i = 0; i < proyectilesJugador.size(); ) {
            proyectilesJugador[i].actualizar(dt);
            if (proyectilesJugador[i].estaDestruido())
                proyectilesJugador.erase(proyectilesJugador.begin() + i);
            else i++;
        }

        for (size_t i = 0; i < lagrimasEnemigas.size(); ) {
            lagrimasEnemigas[i].actualizar(dt);
            if (lagrimasEnemigas[i].estaDestruida())
                lagrimasEnemigas.erase(lagrimasEnemigas.begin() + i);
            else i++;
        }

        for (size_t i = 0; i < proyectilesRaptor.size(); ) {
            proyectilesRaptor[i]->actualizar(dt);
            if (proyectilesRaptor[i]->estaDestruido())
                proyectilesRaptor.erase(proyectilesRaptor.begin() + i);
            else i++;
        }

        for (size_t i = 0; i < proyectilesSpreadshot.size(); ) {
            proyectilesSpreadshot[i]->actualizar(dt);
            if (proyectilesSpreadshot[i]->estaDestruido())
                proyectilesSpreadshot.erase(proyectilesSpreadshot.begin() + i);
            else i++;
        }

        manejarColisiones();

        for (size_t i = 0; i < enemigos.size(); ) {
            bool eliminar = false;
            if (auto* b  = dynamic_cast<Blob*>(enemigos[i].get()))       eliminar = b->listo_para_eliminar();
            else if (auto* r  = dynamic_cast<Raptor*>(enemigos[i].get())) eliminar = r->listo_para_eliminar();
            else if (auto* ru = dynamic_cast<Rusher*>(enemigos[i].get())) eliminar = ru->listo_para_eliminar();
            else if (auto* s  = dynamic_cast<Spreadshot*>(enemigos[i].get())) eliminar = s->listo_para_eliminar();
            else eliminar = enemigos[i]->estaMuerto();
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
        for (auto& proy : proyectilesJugador) proy.dibujar(ventana);
        for (auto& lagrima : lagrimasEnemigas) lagrima.dibujar(ventana);
        for (auto& proy : proyectilesRaptor) proy->dibujar(ventana);
        for (auto& proy : proyectilesSpreadshot) proy->dibujar(ventana);
        dibujarHUD();
        ventana.display();
    }

public:
    Juego() : ventana(sf::VideoMode({800, 600}), "Aventuras en el Edificio de Software") {
        ventana.setFramerateLimit(60);
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        tiempoEntreDisparos = 0.3f;

        personajeElegido = seleccionarPersonaje();
        if (personajeElegido < 0) return;

        const ConfigPersonaje& cfg = PERSONAJES[personajeElegido];
        isaac = std::make_unique<Jugador>(400.0f, 300.0f,
                                          cfg.sheet1, cfg.sheet2, cfg.proyectil);

        enemigos.push_back(std::make_unique<Blob>(150.0f, 150.0f));
        enemigos.push_back(std::make_unique<Raptor>(650.0f, 150.0f));
        enemigos.push_back(std::make_unique<Rusher>(650.0f, 450.0f));
        enemigos.push_back(std::make_unique<Spreadshot>(150.0f, 450.0f));
    }

    void ejecutar() {
        if (personajeElegido < 0) return;
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
