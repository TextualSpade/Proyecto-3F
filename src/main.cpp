#include <SFML/Graphics.hpp>
#include <vector>
#include <deque>
#include <memory>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
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
      "Coming soon..." },
    { "assets/images/personaje3_sheet1.png",
      "assets/images/personaje3_sheet2.png",
      "assets/images/personaje3_sword.png",
      "Coming soon..." },
};

static constexpr int   FRAME_W     = 302;
static constexpr int   FRAME_H     = 302;
static constexpr int   FILA_SLASH  = 4;
static constexpr int   FRAME_SLASH = 9;
static constexpr float ESCALA_SEL  = 0.45f;


struct DatosSala {
    std::string nombre;
    std::string rutaFondo;
    sf::Texture texturaFondo;

    int arriba = -1;
    int abajo = -1;
    int izquierda = -1;
    int derecha = -1;

    bool visitada = false;
    bool limpiada = false;

    int mapaColumna = -1;
    int mapaFila = -1;
    sf::Color colorMapa = sf::Color(70, 70, 70);
};

static constexpr float LIMITE_IZQ = 40.0f;
static constexpr float LIMITE_DER = 760.0f;
static constexpr float LIMITE_ARR = 40.0f;
static constexpr float LIMITE_ABA = 560.0f;
static constexpr float CENTRO_X = 400.0f;
static constexpr float CENTRO_Y = 300.0f;
static constexpr float ANCHO_PUERTA = 110.0f;


class Juego {
private:
    sf::RenderWindow ventana;
    sf::Texture texturaCorazon;
    bool hayTexturaCorazon;
    std::unique_ptr<Jugador> isaac;
    std::deque<ProyectilJugador> proyectilesJugador;
    std::vector<Lagrima> lagrimasEnemigas;
    std::vector<std::unique_ptr<ProyectilRaptor>> proyectilesRaptor;
    std::vector<std::unique_ptr<ProyectilSpreadshot>> proyectilesSpreadshot;
    std::vector<std::unique_ptr<Enemigo>> enemigos;
    sf::Clock reloj;
    sf::Clock relojDisparo;
    float tiempoEntreDisparos;
    int personajeElegido;
    std::vector<DatosSala> salas;
    int salaActual;
    bool bloqueoCambioSala;

    int seleccionarPersonaje() {
        sf::Font fuente;
        bool hayFuente = fuente.openFromFile("assets/fonts/MedievalSharp.ttf");
        if (!hayFuente) hayFuente = fuente.openFromFile("assets/fonts/arial.ttf");

        sf::Texture texPersonajes[3];
        sf::Sprite sprPersonajes[3] = {
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

        // Solo el personaje 1 está disponible por ahora.
        // Los personajes 2 y 3 se muestran como bloqueados.
        const int seleccion = 0;

        while (ventana.isOpen()) {
            while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
                if (evento->is<sf::Event::Closed>()) {
                    ventana.close();
                    return -1;
                }

                if (const auto* kp = evento->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::Num1 ||
                        kp->code == sf::Keyboard::Key::Enter) {
                        return 0;
                    }
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
                bool disponible = (i == 0);

                sf::RectangleShape caja({180.0f, 220.0f});
                caja.setOrigin({90.0f, 110.0f});
                caja.setPosition({cx, cy});
                caja.setFillColor(disponible
                    ? sf::Color(60, 45, 15)
                    : sf::Color(25, 25, 30));
                caja.setOutlineThickness(3.0f);
                caja.setOutlineColor(disponible
                    ? sf::Color(210, 170, 50)
                    : sf::Color(80, 80, 85));
                ventana.draw(caja);

                sprPersonajes[i].setPosition({cx, cy - 15.0f});
                sprPersonajes[i].setColor(disponible
                    ? sf::Color::White
                    : sf::Color(90, 90, 90, 150));
                ventana.draw(sprPersonajes[i]);

                if (hayFuente) {
                    sf::Text nombre(fuente, PERSONAJES[i].nombre, disponible ? 15 : 18);
                    nombre.setFillColor(disponible
                        ? sf::Color(210, 170, 50)
                        : sf::Color(160, 160, 160));
                    nombre.setOutlineColor(sf::Color(20, 20, 20));
                    nombre.setOutlineThickness(disponible ? 0.0f : 1.0f);
                    auto nb = nombre.getLocalBounds();
                    nombre.setOrigin({nb.size.x / 2.0f, 0.0f});
                    nombre.setPosition({cx, cy + 118.0f});
                    ventana.draw(nombre);

                    sf::Text tecla(fuente, disponible ? "[1]" : "Bloqueado", 13);
                    tecla.setFillColor(disponible
                        ? sf::Color(140, 120, 80)
                        : sf::Color(100, 100, 100));
                    auto tk = tecla.getLocalBounds();
                    tecla.setOrigin({tk.size.x / 2.0f, 0.0f});
                    tecla.setPosition({cx, cy + 142.0f});
                    ventana.draw(tecla);
                }
            }

            if (hayFuente) {
                sf::Text instruccion(fuente, "Presiona Enter o 1 para comenzar", 14);
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
                isaac->getCentroDisparo(), dirDisparo,
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

    void dibujarCirculoDebug(sf::Vector2f centro, float radio, sf::Color color) {
        sf::CircleShape circulo(radio);
        circulo.setOrigin({radio, radio});
        circulo.setPosition(centro);
        circulo.setFillColor(sf::Color::Transparent);
        circulo.setOutlineColor(color);
        circulo.setOutlineThickness(2.0f);
        ventana.draw(circulo);
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
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                isaac->recibirDanio(1);
                if (auto* blob = dynamic_cast<Blob*>(enemigo.get())) blob->tocarJugador();
                if (auto* rusher = dynamic_cast<Rusher*>(enemigo.get())) rusher->golpearJugador();
            }
        }

        for (auto& lagrima : lagrimasEnemigas) {
            if (!lagrima.estaDestruida() &&
                colisionan(lagrima.getPosicion(), lagrima.getRadio(),
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                isaac->recibirDanio(1);
                lagrima.destruir();
            }
        }

        for (auto& proy : proyectilesRaptor) {
            if (!proy->estaDestruido() &&
                colisionan(proy->getPosicion(), proy->getRadio(),
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                isaac->recibirDanio(1);
                proy->golpear();
            }
        }

        for (auto& proy : proyectilesSpreadshot) {
            if (!proy->estaDestruido() &&
                colisionan(proy->getPosicion(), proy->getRadio(),
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                isaac->recibirDanio(1);
                proy->golpear();
            }
        }
    }


    void configurarSalas() {
        salas.clear();
        salas.resize(10);

        salas[0].nombre = "Sala morada izquierda";
        salas[0].rutaFondo = "assets/images/rooms/room_left_purple.jpeg";

        salas[1].nombre = "Sala verde";
        salas[1].rutaFondo = "assets/images/rooms/room_green.jpeg";
        salas[1].mapaColumna = 0;
        salas[1].mapaFila = 1;
        salas[1].colorMapa = sf::Color(70, 170, 80);
        salas[1].derecha = 4;

        salas[2].nombre = "Sala turquesa";
        salas[2].rutaFondo = "assets/images/rooms/room_teal.jpeg";
        salas[2].mapaColumna = 0;
        salas[2].mapaFila = 2;
        salas[2].colorMapa = sf::Color(40, 180, 175);
        salas[2].derecha = 5;

        salas[3].nombre = "Sala rosa";
        salas[3].rutaFondo = "assets/images/rooms/room_pink.jpeg";
        salas[3].mapaColumna = 1;
        salas[3].mapaFila = 0;
        salas[3].colorMapa = sf::Color(210, 100, 170);
        salas[3].abajo = 4;
        salas[3].derecha = 6;

        salas[4].nombre = "Sala central naranja";
        salas[4].rutaFondo = "assets/images/rooms/room_orange.jpeg";
        salas[4].mapaColumna = 1;
        salas[4].mapaFila = 1;
        salas[4].colorMapa = sf::Color(220, 130, 45);
        salas[4].arriba = 3;
        salas[4].abajo = 5;
        salas[4].izquierda = 1;
        salas[4].derecha = 7;

        salas[5].nombre = "Sala azul";
        salas[5].rutaFondo = "assets/images/rooms/room_blue.jpeg";
        salas[5].mapaColumna = 1;
        salas[5].mapaFila = 2;
        salas[5].colorMapa = sf::Color(75, 130, 220);
        salas[5].arriba = 4;
        salas[5].izquierda = 2;
        salas[5].derecha = 8;

        salas[6].nombre = "Sala blanca";
        salas[6].rutaFondo = "assets/images/rooms/room_white.jpeg";
        salas[6].mapaColumna = 2;
        salas[6].mapaFila = 0;
        salas[6].colorMapa = sf::Color(230, 230, 210);
        salas[6].izquierda = 3;

        salas[7].nombre = "Sala roja";
        salas[7].rutaFondo = "assets/images/rooms/room_red.jpeg";
        salas[7].mapaColumna = 2;
        salas[7].mapaFila = 1;
        salas[7].colorMapa = sf::Color(190, 55, 55);
        salas[7].izquierda = 4;
        salas[7].abajo = 8;

        salas[8].nombre = "Sala tablero";
        salas[8].rutaFondo = "assets/images/rooms/room_checker.jpeg";
        salas[8].mapaColumna = 2;
        salas[8].mapaFila = 2;
        salas[8].colorMapa = sf::Color(170, 170, 170);
        salas[8].arriba = 7;
        salas[8].abajo = 9;
        salas[8].izquierda = 5;

        salas[9].nombre = "Sala boss";
        salas[9].rutaFondo = "assets/images/rooms/room_boss.jpeg";
        salas[9].mapaColumna = 2;
        salas[9].mapaFila = 3;
        salas[9].colorMapa = sf::Color(125, 70, 165);
        salas[9].arriba = 8;

        for (auto& sala : salas) {
            sala.texturaFondo.loadFromFile(sala.rutaFondo);
        }
    }

    void limpiarEntidadesDeSala() {
        proyectilesJugador.clear();
        lagrimasEnemigas.clear();
        proyectilesRaptor.clear();
        proyectilesSpreadshot.clear();
        enemigos.clear();
    }

    void cargarEnemigosSala() {
        limpiarEntidadesDeSala();

        if (salas[salaActual].limpiada) {
            return;
        }

        switch (salaActual) {
            case 1:
                enemigos.push_back(std::make_unique<Blob>(220.0f, 220.0f));
                enemigos.push_back(std::make_unique<Blob>(580.0f, 390.0f));
                break;
            case 2:
                enemigos.push_back(std::make_unique<Rusher>(620.0f, 300.0f));
                break;
            case 3:
                enemigos.push_back(std::make_unique<Spreadshot>(400.0f, 240.0f));
                break;
            case 5:
                enemigos.push_back(std::make_unique<Raptor>(620.0f, 180.0f));
                enemigos.push_back(std::make_unique<Blob>(240.0f, 420.0f));
                break;
            case 6:
                enemigos.push_back(std::make_unique<Spreadshot>(400.0f, 230.0f));
                enemigos.push_back(std::make_unique<Blob>(220.0f, 380.0f));
                break;
            case 7:
                enemigos.push_back(std::make_unique<Raptor>(620.0f, 170.0f));
                enemigos.push_back(std::make_unique<Rusher>(220.0f, 400.0f));
                break;
            case 8:
                enemigos.push_back(std::make_unique<Blob>(250.0f, 210.0f));
                enemigos.push_back(std::make_unique<Blob>(550.0f, 210.0f));
                enemigos.push_back(std::make_unique<Raptor>(400.0f, 410.0f));
                break;
            case 9:
                enemigos.push_back(std::make_unique<Spreadshot>(400.0f, 210.0f));
                enemigos.push_back(std::make_unique<Raptor>(230.0f, 430.0f));
                enemigos.push_back(std::make_unique<Rusher>(570.0f, 430.0f));
                break;
            default:
                break;
        }
    }

    void cambiarSala(int nuevaSala, sf::Vector2f nuevaPosicionJugador) {
        if (nuevaSala < 0 || nuevaSala >= static_cast<int>(salas.size())) {
            return;
        }

        salaActual = nuevaSala;
        salas[salaActual].visitada = true;
        isaac->setPosicion(nuevaPosicionJugador.x, nuevaPosicionJugador.y);
        cargarEnemigosSala();
        bloqueoCambioSala = true;
    }

    void intentarCambioSala() {
        if (salas.empty()) {
            return;
        }

        bool presionandoPuerta =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);

        if (!presionandoPuerta) {
            bloqueoCambioSala = false;
            return;
        }

        if (bloqueoCambioSala) {
            return;
        }

        const DatosSala& sala = salas[salaActual];
        sf::Vector2f centro = isaac->getCentroHitbox();

        if (sala.arriba != -1 &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
            centro.y <= LIMITE_ARR + 35.0f &&
            std::abs(centro.x - CENTRO_X) <= ANCHO_PUERTA) {
            cambiarSala(sala.arriba, {CENTRO_X, LIMITE_ABA - 45.0f});
            return;
        }

        if (sala.abajo != -1 &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) &&
            centro.y >= LIMITE_ABA - 35.0f &&
            std::abs(centro.x - CENTRO_X) <= ANCHO_PUERTA) {
            cambiarSala(sala.abajo, {CENTRO_X, LIMITE_ARR + 55.0f});
            return;
        }

        if (sala.izquierda != -1 &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) &&
            centro.x <= LIMITE_IZQ + 35.0f &&
            std::abs(centro.y - CENTRO_Y) <= ANCHO_PUERTA) {
            cambiarSala(sala.izquierda, {LIMITE_DER - 55.0f, CENTRO_Y});
            return;
        }

        if (sala.derecha != -1 &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) &&
            centro.x >= LIMITE_DER - 35.0f &&
            std::abs(centro.y - CENTRO_Y) <= ANCHO_PUERTA) {
            cambiarSala(sala.derecha, {LIMITE_IZQ + 55.0f, CENTRO_Y});
            return;
        }
    }

    void dibujarFondoSala() {
        if (salas.empty()) {
            return;
        }

        sf::Texture& textura = salas[salaActual].texturaFondo;
        if (textura.getSize().x > 0 && textura.getSize().y > 0) {
            sf::Sprite fondo(textura);
            float escalaX = 800.0f / static_cast<float>(textura.getSize().x);
            float escalaY = 600.0f / static_cast<float>(textura.getSize().y);
            fondo.setScale({escalaX, escalaY});
            fondo.setPosition({0.0f, 0.0f});
            ventana.draw(fondo);
        } else {
            sf::RectangleShape fondo({800.0f, 600.0f});
            fondo.setFillColor(sf::Color(25, 25, 30));
            ventana.draw(fondo);
        }
    }

    void dibujarPuerta(float x, float y, float ancho, float alto) {
        sf::RectangleShape puerta({ancho, alto});
        puerta.setPosition({x, y});
        puerta.setFillColor(enemigos.empty()
            ? sf::Color(80, 170, 255, 120)
            : sf::Color(220, 120, 40, 120));
        puerta.setOutlineThickness(2.0f);
        puerta.setOutlineColor(sf::Color(15, 15, 20, 180));
        ventana.draw(puerta);
    }

    void dibujarPuertasDisponibles() {
        if (salas.empty()) {
            return;
        }

        const DatosSala& sala = salas[salaActual];

        if (sala.arriba != -1) {
            dibujarPuerta(CENTRO_X - 45.0f, LIMITE_ARR - 5.0f, 90.0f, 20.0f);
        }
        if (sala.abajo != -1) {
            dibujarPuerta(CENTRO_X - 45.0f, LIMITE_ABA - 15.0f, 90.0f, 20.0f);
        }
        if (sala.izquierda != -1) {
            dibujarPuerta(LIMITE_IZQ - 5.0f, CENTRO_Y - 45.0f, 20.0f, 90.0f);
        }
        if (sala.derecha != -1) {
            dibujarPuerta(LIMITE_DER - 15.0f, CENTRO_Y - 45.0f, 20.0f, 90.0f);
        }
    }


    void dibujarConexionMiniMapa(float x1, float y1, float x2, float y2, bool visible) {
        sf::Vector2f inicio{x1, y1};
        sf::Vector2f fin{x2, y2};
        sf::Vector2f delta = fin - inicio;
        float longitud = std::sqrt(delta.x * delta.x + delta.y * delta.y);

        if (longitud <= 0.0f) {
            return;
        }

        sf::RectangleShape linea({longitud, 5.0f});
        linea.setOrigin({0.0f, 2.5f});
        linea.setPosition(inicio);
        linea.setRotation(sf::degrees(std::atan2(delta.y, delta.x) * 180.0f / 3.14159265f));
        linea.setFillColor(visible ? sf::Color(120, 120, 120, 220) : sf::Color(0, 0, 0, 255));
        ventana.draw(linea);
    }

    sf::Vector2f posicionMiniSala(int indiceSala, float baseX, float baseY, float paso) const {
        return {
            baseX + salas[indiceSala].mapaColumna * paso,
            baseY + salas[indiceSala].mapaFila * paso
        };
    }

    void dibujarMiniMapa() {
        if (salas.empty()) {
            return;
        }

        const float baseX = 640.0f;
        const float baseY = 60.0f;
        const float tam = 26.0f;
        const float paso = 38.0f;

        sf::RectangleShape panel({145.0f, 185.0f});
        panel.setPosition({620.0f, 38.0f});
        panel.setFillColor(sf::Color(10, 10, 14, 190));
        panel.setOutlineThickness(2.0f);
        panel.setOutlineColor(sf::Color(90, 90, 100, 210));
        ventana.draw(panel);

        for (int i = 0; i < static_cast<int>(salas.size()); i++) {
            if (salas[i].mapaColumna < 0 || salas[i].mapaFila < 0) {
                continue;
            }

            sf::Vector2f pos = posicionMiniSala(i, baseX, baseY, paso);
            sf::Vector2f centro{pos.x + tam / 2.0f, pos.y + tam / 2.0f};

            auto dibujarConexion = [&](int vecino) {
                if (vecino == -1 || vecino < 0 || vecino >= static_cast<int>(salas.size())) {
                    return;
                }
                if (salas[vecino].mapaColumna < 0 || salas[vecino].mapaFila < 0) {
                    return;
                }
                sf::Vector2f posVecino = posicionMiniSala(vecino, baseX, baseY, paso);
                sf::Vector2f centroVecino{posVecino.x + tam / 2.0f, posVecino.y + tam / 2.0f};
                bool visible = salas[i].visitada && salas[vecino].visitada;
                dibujarConexionMiniMapa(centro.x, centro.y, centroVecino.x, centroVecino.y, visible);
            };

            if (salas[i].derecha != -1) {
                dibujarConexion(salas[i].derecha);
            }
            if (salas[i].abajo != -1) {
                dibujarConexion(salas[i].abajo);
            }
        }

        for (int i = 0; i < static_cast<int>(salas.size()); i++) {
            if (salas[i].mapaColumna < 0 || salas[i].mapaFila < 0) {
                continue;
            }

            sf::Vector2f pos = posicionMiniSala(i, baseX, baseY, paso);
            sf::RectangleShape cuarto({tam, tam});
            cuarto.setPosition(pos);
            cuarto.setFillColor(salas[i].visitada ? salas[i].colorMapa : sf::Color::Black);
            cuarto.setOutlineThickness(2.0f);
            cuarto.setOutlineColor(salas[i].visitada ? sf::Color(235, 235, 235, 220)
                                                     : sf::Color(30, 30, 35, 255));
            ventana.draw(cuarto);

            if (i == salaActual) {
                sf::CircleShape iconoJugador(6.0f);
                iconoJugador.setOrigin({6.0f, 6.0f});
                iconoJugador.setPosition({pos.x + tam / 2.0f, pos.y + tam / 2.0f});
                iconoJugador.setFillColor(sf::Color(255, 230, 40));
                iconoJugador.setOutlineThickness(2.0f);
                iconoJugador.setOutlineColor(sf::Color::Black);
                ventana.draw(iconoJugador);
            }
        }
    }

    void dibujarHUD() {
        for (int i = 0; i < isaac->getVidaMaxima(); i++) {
            if (hayTexturaCorazon) {
                sf::Sprite corazon(texturaCorazon);
                corazon.setPosition({50.0f + i * 36.0f, 6.0f});
                corazon.setScale({1.0f, 1.0f});

                if (i >= isaac->getVida()) {
                    corazon.setColor(sf::Color(70, 70, 70, 140));
                }

                ventana.draw(corazon);
            } else {
                sf::RectangleShape corazon({22.0f, 22.0f});
                corazon.setPosition({50.0f + i * 30.0f, 8.0f});
                corazon.setFillColor(i < isaac->getVida()
                    ? sf::Color(220, 40, 40)
                    : sf::Color(80, 80, 80));
                ventana.draw(corazon);
            }
        }
    }

    void reiniciarPartida() {
        proyectilesJugador.clear();
        lagrimasEnemigas.clear();
        proyectilesRaptor.clear();
        proyectilesSpreadshot.clear();
        enemigos.clear();

        salaActual = 6;
        for (auto& sala : salas) {
            sala.visitada = false;
            sala.limpiada = false;
        }
        salas[salaActual].visitada = true;

        isaac->reiniciar(400.0f, 300.0f);
        cargarEnemigosSala();
    }

    void actualizar(float dt) {
        isaac->actualizar(dt);
        manejarDisparos();

        for (auto& enemigo : enemigos) {
            enemigo->setObjetivo(isaac->getCentroHitbox());
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

        if (enemigos.empty()) {
            salas[salaActual].limpiada = true;
        }

        intentarCambioSala();

        if (!isaac->estaVivo()) reiniciarPartida();
    }

    void renderizar() {
        ventana.clear(sf::Color(0, 0, 0));
        dibujarFondoSala();
        dibujarPuertasDisponibles();

        isaac->dibujar(ventana);
        for (auto& enemigo : enemigos) enemigo->dibujar(ventana);
        for (auto& proy : proyectilesJugador) proy.dibujar(ventana);
        for (auto& lagrima : lagrimasEnemigas) lagrima.dibujar(ventana);
        for (auto& proy : proyectilesRaptor) proy->dibujar(ventana);
        for (auto& proy : proyectilesSpreadshot) proy->dibujar(ventana);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H)) {
            isaac->dibujarHitbox(ventana);

            for (auto& enemigo : enemigos) {
                dibujarCirculoDebug(enemigo->getPosicion(), enemigo->getRadio(), sf::Color::Yellow);
            }

            for (auto& lagrima : lagrimasEnemigas) {
                dibujarCirculoDebug(lagrima.getPosicion(), lagrima.getRadio(), sf::Color::Cyan);
            }

            for (auto& proy : proyectilesRaptor) {
                dibujarCirculoDebug(proy->getPosicion(), proy->getRadio(), sf::Color::Magenta);
            }

            for (auto& proy : proyectilesSpreadshot) {
                dibujarCirculoDebug(proy->getPosicion(), proy->getRadio(), sf::Color::Green);
            }
        }

        dibujarHUD();
        dibujarMiniMapa();
        ventana.display();
    }

public:
    Juego()
        : ventana(sf::VideoMode({800, 600}), "Aventuras en el Edificio de Software"),
          hayTexturaCorazon(false),
          salaActual(6),
          bloqueoCambioSala(false) {
        ventana.setFramerateLimit(60);
        std::srand(static_cast<unsigned>(std::time(nullptr)));

        hayTexturaCorazon = texturaCorazon.loadFromFile(
            "assets/images/heart_pixel_art_32x32.png"
        );

        configurarSalas();

        tiempoEntreDisparos = 0.3f;

        personajeElegido = seleccionarPersonaje();
        if (personajeElegido < 0) return;

        const ConfigPersonaje& cfg = PERSONAJES[personajeElegido];
        isaac = std::make_unique<Jugador>(400.0f, 300.0f,
                                          cfg.sheet1, cfg.sheet2, cfg.proyectil);

        salas[salaActual].visitada = true;
        cargarEnemigosSala();
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
