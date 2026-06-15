#include <SFML/Graphics.hpp>
#include <vector>
#include <deque>
#include <memory>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <algorithm>
#include <array>
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
    bool visibleEnMapa = true;

    int mapaColumna = -1;
    int mapaFila = -1;
    sf::Color colorMapa = sf::Color(70, 70, 70);
};

enum class TipoObjeto {
    Corazon,
    Estrella,
    Rayo
};

struct ObjetoSuelo {
    TipoObjeto tipo;
    sf::Vector2f posicion;
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
    sf::Texture texturaLlave;
    sf::Texture texturaObjetoCorazon;
    sf::Texture texturaObjetoEstrella;
    sf::Texture texturaObjetoRayo;
    bool hayTexturaCorazon;
    bool hayTexturaLlave;
    bool hayTexturaObjetoCorazon;
    bool hayTexturaObjetoEstrella;
    bool hayTexturaObjetoRayo;
    std::unique_ptr<Jugador> isaac;
    std::deque<ProyectilJugador> proyectilesJugador;
    std::vector<Lagrima> lagrimasEnemigas;
    std::vector<std::unique_ptr<ProyectilRaptor>> proyectilesRaptor;
    std::vector<std::unique_ptr<ProyectilSpreadshot>> proyectilesSpreadshot;
    std::vector<std::unique_ptr<Enemigo>> enemigos;
    std::vector<ObjetoSuelo> objetosSuelo;
    sf::Clock reloj;
    sf::Clock relojDisparo;
    float tiempoEntreDisparos;
    int stacksVelocidad;
    int stacksEstrella;
    float tiempoVelocidad;
    float tiempoEstrella;
    int personajeElegido;
    std::vector<DatosSala> salas;
    int salaActual;
    bool bloqueoCambioSala;
    bool puertaSecretaDesbloqueada;
    bool llaveDisponible;
    bool llaveObtenida;
    sf::Vector2f posicionLlave;
    std::string mensajePantalla;
    float tiempoMensaje;
    sf::Font fuenteHUD;
    bool hayFuenteHUD;

    static constexpr int SALA_SECRETA = 0;
    static constexpr int SALA_E = 1; // room_green
    static constexpr int SALA_G = 2; // room_teal
    static constexpr int SALA_B = 3; // room_pink
    static constexpr int SALA_C = 4; // room_orange
    static constexpr int SALA_F = 5; // room_blue
    static constexpr int SALA_A = 6; // room_white
    static constexpr int SALA_D = 7; // room_red
    static constexpr int SALA_H = 8; // room_checker
    static constexpr int SALA_I = 9; // room_boss

    void mostrarMensaje(const std::string& texto, float segundos = 3.0f) {
        mensajePantalla = texto;
        tiempoMensaje = segundos;
    }

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

    sf::Vector2f direccionRotada(sf::Vector2f direccion, float radianes) const {
        float c = std::cos(radianes);
        float sen = std::sin(radianes);
        sf::Vector2f resultado{
            direccion.x * c - direccion.y * sen,
            direccion.x * sen + direccion.y * c
        };

        float longitud = std::sqrt(resultado.x * resultado.x + resultado.y * resultado.y);
        if (longitud > 0.0f) {
            resultado /= longitud;
        }
        return resultado;
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
            int paresExtra = std::min(stacksEstrella, 3);

            if (paresExtra <= 0) {
                proyectilesJugador.emplace_back(
                    isaac->getCentroDisparo(), dirDisparo,
                    isaac->getRutaProyectil());
            } else {
                // Estrella: dispara en un cono cerrado.
                // 1 stack = 3 proyectiles, 2 stacks = 5, 3+ stacks = 7.
                const float separacion = 0.10f;
                for (int i = -paresExtra; i <= paresExtra; i++) {
                    sf::Vector2f dir = direccionRotada(dirDisparo, separacion * static_cast<float>(i));
                    proyectilesJugador.emplace_back(
                        isaac->getCentroDisparo(), dir,
                        isaac->getRutaProyectil());
                }
            }

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

    float probabilidadDropEnemigo(Enemigo* enemigo) const {
        if (dynamic_cast<Blob*>(enemigo)) return 0.25f;
        if (dynamic_cast<Raptor*>(enemigo)) return 0.35f;
        if (dynamic_cast<Rusher*>(enemigo)) return 0.50f;
        if (dynamic_cast<Spreadshot*>(enemigo)) return 0.50f;
        return 0.0f;
    }

    void intentarSoltarObjeto(Enemigo* enemigo) {
        if (!enemigo) return;

        float probabilidad = probabilidadDropEnemigo(enemigo);
        float tiro = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        if (tiro > probabilidad) {
            return;
        }

        int tipo = std::rand() % 100;
        TipoObjeto objeto = TipoObjeto::Corazon;

        if (tipo < 40) {
            objeto = TipoObjeto::Corazon;      // 40%
        } else if (tipo < 65) {
            objeto = TipoObjeto::Estrella;    // 25%
        } else if (tipo < 90) {
            objeto = TipoObjeto::Rayo;        // 25%
        } else {
            // 10% reservado para el cuarto objeto que definiremos después.
            return;
        }

        objetosSuelo.push_back({objeto, enemigo->getPosicion()});
    }

    sf::Texture* texturaParaObjeto(TipoObjeto tipo) {
        switch (tipo) {
            case TipoObjeto::Corazon:
                return hayTexturaObjetoCorazon ? &texturaObjetoCorazon : nullptr;
            case TipoObjeto::Estrella:
                return hayTexturaObjetoEstrella ? &texturaObjetoEstrella : nullptr;
            case TipoObjeto::Rayo:
                return hayTexturaObjetoRayo ? &texturaObjetoRayo : nullptr;
        }
        return nullptr;
    }

    void aplicarObjeto(TipoObjeto tipo) {
        switch (tipo) {
            case TipoObjeto::Corazon:
                isaac->sanar(1);
                mostrarMensaje("Corazon +1", 1.8f);
                break;

            case TipoObjeto::Rayo:
                stacksVelocidad++;
                tiempoVelocidad += 30.0f;
                isaac->setMultiplicadorVelocidad(1.0f + 0.5f * static_cast<float>(stacksVelocidad));
                mostrarMensaje("Velocidad aumentada", 2.2f);
                break;

            case TipoObjeto::Estrella:
                stacksEstrella++;
                tiempoEstrella += 15.0f;
                mostrarMensaje("Disparo triple activado", 2.2f);
                break;
        }
    }

    void actualizarBuffs(float dt) {
        if (tiempoVelocidad > 0.0f) {
            tiempoVelocidad -= dt;
            if (tiempoVelocidad <= 0.0f) {
                tiempoVelocidad = 0.0f;
                stacksVelocidad = 0;
                isaac->setMultiplicadorVelocidad(1.0f);
            }
        }

        if (tiempoEstrella > 0.0f) {
            tiempoEstrella -= dt;
            if (tiempoEstrella <= 0.0f) {
                tiempoEstrella = 0.0f;
                stacksEstrella = 0;
            }
        }
    }

    void actualizarObjetosSuelo() {
        for (size_t i = 0; i < objetosSuelo.size(); ) {
            if (colisionan(isaac->getCentroHitbox(), isaac->getRadio(),
                           objetosSuelo[i].posicion, 18.0f)) {
                aplicarObjeto(objetosSuelo[i].tipo);
                objetosSuelo.erase(objetosSuelo.begin() + i);
            } else {
                i++;
            }
        }
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

        // SALA SECRETA: no aparece en el minimapa.
        // Se desbloquea cuando limpias la sala E / verde.
        salas[SALA_SECRETA].nombre = "Sala secreta";
        salas[SALA_SECRETA].rutaFondo = "assets/images/rooms/room_left_purple.jpeg";
        salas[SALA_SECRETA].visibleEnMapa = false;
        salas[SALA_SECRETA].derecha = SALA_E;

        // E = room_green
        salas[SALA_E].nombre = "E - Sala verde";
        salas[SALA_E].rutaFondo = "assets/images/rooms/room_green.jpeg";
        salas[SALA_E].mapaColumna = 1;
        salas[SALA_E].mapaFila = 1;
        salas[SALA_E].colorMapa = sf::Color(70, 170, 80);
        salas[SALA_E].derecha = SALA_C;
        // La izquierda se activa después de limpiar esta sala.
        salas[SALA_E].izquierda = -1;

        // G = room_teal
        salas[SALA_G].nombre = "G - Sala turquesa";
        salas[SALA_G].rutaFondo = "assets/images/rooms/room_teal.jpeg";
        salas[SALA_G].mapaColumna = 1;
        salas[SALA_G].mapaFila = 2;
        salas[SALA_G].colorMapa = sf::Color(40, 180, 175);
        salas[SALA_G].derecha = SALA_F;

        // B = room_pink
        salas[SALA_B].nombre = "B - Sala rosa";
        salas[SALA_B].rutaFondo = "assets/images/rooms/room_pink.jpeg";
        salas[SALA_B].mapaColumna = 2;
        salas[SALA_B].mapaFila = 0;
        salas[SALA_B].colorMapa = sf::Color(210, 100, 170);
        salas[SALA_B].derecha = SALA_A;
        salas[SALA_B].abajo = SALA_C;

        // C = room_orange, conexión hacia todos los lados.
        salas[SALA_C].nombre = "C - Sala naranja";
        salas[SALA_C].rutaFondo = "assets/images/rooms/room_orange.jpeg";
        salas[SALA_C].mapaColumna = 2;
        salas[SALA_C].mapaFila = 1;
        salas[SALA_C].colorMapa = sf::Color(220, 130, 45);
        salas[SALA_C].arriba = SALA_B;
        salas[SALA_C].abajo = SALA_F;
        salas[SALA_C].izquierda = SALA_E;
        salas[SALA_C].derecha = SALA_D;

        // F = room_blue
        salas[SALA_F].nombre = "F - Sala azul";
        salas[SALA_F].rutaFondo = "assets/images/rooms/room_blue.jpeg";
        salas[SALA_F].mapaColumna = 2;
        salas[SALA_F].mapaFila = 2;
        salas[SALA_F].colorMapa = sf::Color(75, 130, 220);
        salas[SALA_F].arriba = SALA_C;
        salas[SALA_F].izquierda = SALA_G;
        salas[SALA_F].derecha = SALA_H;

        // A = room_white, sala inicial.
        salas[SALA_A].nombre = "A - Sala blanca";
        salas[SALA_A].rutaFondo = "assets/images/rooms/room_white.jpeg";
        salas[SALA_A].mapaColumna = 3;
        salas[SALA_A].mapaFila = 0;
        salas[SALA_A].colorMapa = sf::Color(230, 230, 210);
        salas[SALA_A].izquierda = SALA_B;

        // D = room_red
        salas[SALA_D].nombre = "D - Sala roja";
        salas[SALA_D].rutaFondo = "assets/images/rooms/room_red.jpeg";
        salas[SALA_D].mapaColumna = 3;
        salas[SALA_D].mapaFila = 1;
        salas[SALA_D].colorMapa = sf::Color(190, 55, 55);
        salas[SALA_D].izquierda = SALA_C;
        salas[SALA_D].abajo = SALA_H;

        // H = room_checker
        salas[SALA_H].nombre = "H - Sala tablero";
        salas[SALA_H].rutaFondo = "assets/images/rooms/room_checker.jpeg";
        salas[SALA_H].mapaColumna = 3;
        salas[SALA_H].mapaFila = 2;
        salas[SALA_H].colorMapa = sf::Color(170, 170, 170);
        salas[SALA_H].izquierda = SALA_F;
        salas[SALA_H].arriba = SALA_D;
        salas[SALA_H].abajo = SALA_I;

        // I = room_boss. Requiere llave.
        salas[SALA_I].nombre = "I - Sala boss";
        salas[SALA_I].rutaFondo = "assets/images/rooms/room_boss.jpeg";
        salas[SALA_I].mapaColumna = 3;
        salas[SALA_I].mapaFila = 3;
        salas[SALA_I].colorMapa = sf::Color(125, 70, 165);
        salas[SALA_I].arriba = SALA_H;

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
        objetosSuelo.clear();
    }

    std::vector<sf::Vector2f> posicionesSpawnSeguras(int cantidad) {
        std::vector<sf::Vector2f> candidatos = {
            {170.0f, 170.0f}, {400.0f, 165.0f}, {630.0f, 170.0f},
            {165.0f, 300.0f},                   {635.0f, 300.0f},
            {170.0f, 435.0f}, {400.0f, 435.0f}, {630.0f, 435.0f},
            {285.0f, 250.0f}, {515.0f, 250.0f},
            {285.0f, 370.0f}, {515.0f, 370.0f}
        };

        sf::Vector2f jugador = isaac ? isaac->getCentroHitbox() : sf::Vector2f{CENTRO_X, CENTRO_Y};
        std::sort(candidatos.begin(), candidatos.end(),
            [jugador](sf::Vector2f a, sf::Vector2f b) {
                float da = (a.x - jugador.x) * (a.x - jugador.x) +
                           (a.y - jugador.y) * (a.y - jugador.y);
                float db = (b.x - jugador.x) * (b.x - jugador.x) +
                           (b.y - jugador.y) * (b.y - jugador.y);
                return da > db;
            });

        if (cantidad < static_cast<int>(candidatos.size())) {
            candidatos.resize(cantidad);
        }

        return candidatos;
    }

    enum class TipoEnemigoSpawn {
        Blob,
        Raptor,
        Rusher,
        Spreadshot
    };

    void agregarEnemigo(TipoEnemigoSpawn tipo, sf::Vector2f pos) {
        switch (tipo) {
            case TipoEnemigoSpawn::Blob:
                enemigos.push_back(std::make_unique<Blob>(pos.x, pos.y));
                break;
            case TipoEnemigoSpawn::Raptor:
                enemigos.push_back(std::make_unique<Raptor>(pos.x, pos.y));
                break;
            case TipoEnemigoSpawn::Rusher:
                enemigos.push_back(std::make_unique<Rusher>(pos.x, pos.y));
                break;
            case TipoEnemigoSpawn::Spreadshot:
                enemigos.push_back(std::make_unique<Spreadshot>(pos.x, pos.y));
                break;
        }
    }

    void spawnearGrupo(const std::vector<TipoEnemigoSpawn>& tipos) {
        std::vector<sf::Vector2f> posiciones = posicionesSpawnSeguras(static_cast<int>(tipos.size()));

        for (size_t i = 0; i < tipos.size(); i++) {
            sf::Vector2f pos = posiciones[i % posiciones.size()];
            agregarEnemigo(tipos[i], pos);
        }
    }

    void cargarEnemigosSala() {
        limpiarEntidadesDeSala();

        if (salas[salaActual].limpiada) {
            return;
        }

        using T = TipoEnemigoSpawn;

        switch (salaActual) {
            case SALA_A:
                // A = room_white. Sala inicial vacía.
                break;

            case SALA_B:
                spawnearGrupo({T::Blob, T::Blob, T::Blob, T::Blob});
                break;

            case SALA_C:
                spawnearGrupo({T::Blob, T::Blob, T::Blob, T::Blob, T::Raptor, T::Raptor});
                break;

            case SALA_D:
                spawnearGrupo({T::Rusher, T::Rusher, T::Rusher, T::Rusher});
                break;

            case SALA_E:
                spawnearGrupo({T::Raptor, T::Raptor, T::Spreadshot, T::Blob, T::Blob});
                break;

            case SALA_F:
                spawnearGrupo({T::Spreadshot, T::Spreadshot, T::Raptor, T::Raptor});
                break;

            case SALA_G:
                spawnearGrupo({T::Spreadshot, T::Spreadshot, T::Spreadshot, T::Spreadshot});
                break;

            case SALA_H:
                spawnearGrupo({T::Rusher, T::Spreadshot, T::Spreadshot});
                break;

            case SALA_SECRETA:
                spawnearGrupo({T::Rusher, T::Rusher, T::Spreadshot, T::Raptor, T::Raptor, T::Blob});
                break;

            case SALA_I:
                // I = boss. Lo dejamos temporal por ahora hasta definir jefe/enemigos finales.
                spawnearGrupo({T::Spreadshot, T::Raptor, T::Rusher});
                break;

            default:
                break;
        }
    }

    bool puedeEntrarSala(int nuevaSala) {
        if (nuevaSala == SALA_I && !llaveObtenida) {
            mostrarMensaje("Necesitas una llave para entrar al jefe", 3.0f);
            return false;
        }

        return true;
    }

    void cambiarSala(int nuevaSala, sf::Vector2f nuevaPosicionJugador) {
        if (nuevaSala < 0 || nuevaSala >= static_cast<int>(salas.size())) {
            return;
        }

        if (!puedeEntrarSala(nuevaSala)) {
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
            // La puerta secreta de la sala verde existe, pero es invisible.
            if (!(salaActual == SALA_E && sala.izquierda == SALA_SECRETA)) {
                dibujarPuerta(LIMITE_IZQ - 5.0f, CENTRO_Y - 45.0f, 20.0f, 90.0f);
            }
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
            if (!salas[i].visibleEnMapa || salas[i].mapaColumna < 0 || salas[i].mapaFila < 0) {
                continue;
            }

            sf::Vector2f pos = posicionMiniSala(i, baseX, baseY, paso);
            sf::Vector2f centro{pos.x + tam / 2.0f, pos.y + tam / 2.0f};

            auto dibujarConexion = [&](int vecino) {
                if (vecino == -1 || vecino < 0 || vecino >= static_cast<int>(salas.size())) {
                    return;
                }
                if (!salas[vecino].visibleEnMapa || salas[vecino].mapaColumna < 0 || salas[vecino].mapaFila < 0) {
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
            if (!salas[i].visibleEnMapa || salas[i].mapaColumna < 0 || salas[i].mapaFila < 0) {
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

    void actualizarLlave() {
        if (!llaveDisponible || llaveObtenida || salaActual != SALA_SECRETA) {
            return;
        }

        if (colisionan(isaac->getCentroHitbox(), isaac->getRadio(), posicionLlave, 14.0f)) {
            llaveDisponible = false;
            llaveObtenida = true;
            mostrarMensaje("Llave obtenida. Ahora puedes abrir la sala I", 3.0f);
        }
    }

    void dibujarObjetoFallback(const ObjetoSuelo& objeto) {
        sf::CircleShape forma(14.0f);
        forma.setOrigin({14.0f, 14.0f});
        forma.setPosition(objeto.posicion);

        switch (objeto.tipo) {
            case TipoObjeto::Corazon:
                forma.setFillColor(sf::Color(245, 65, 100));
                break;
            case TipoObjeto::Estrella:
                forma.setFillColor(sf::Color(255, 220, 70));
                break;
            case TipoObjeto::Rayo:
                forma.setFillColor(sf::Color(255, 245, 70));
                break;
        }

        forma.setOutlineThickness(2.0f);
        forma.setOutlineColor(sf::Color::Black);
        ventana.draw(forma);
    }

    void dibujarObjetosSuelo() {
        for (const auto& objeto : objetosSuelo) {
            sf::Texture* textura = texturaParaObjeto(objeto.tipo);
            if (textura && textura->getSize().x > 0 && textura->getSize().y > 0) {
                sf::Sprite sprite(*textura);
                auto tam = textura->getSize();
                sprite.setOrigin({static_cast<float>(tam.x) / 2.0f,
                                  static_cast<float>(tam.y) / 2.0f});
                sprite.setPosition(objeto.posicion);

                float maximo = 34.0f;
                float escala = maximo / static_cast<float>(std::max(tam.x, tam.y));
                sprite.setScale({escala, escala});
                ventana.draw(sprite);
            } else {
                dibujarObjetoFallback(objeto);
            }
        }
    }

    void dibujarLlave() {
        if (!llaveDisponible || llaveObtenida || salaActual != SALA_SECRETA) {
            return;
        }

        if (hayTexturaLlave) {
            sf::Sprite llave(texturaLlave);
            auto tam = texturaLlave.getSize();
            llave.setOrigin({static_cast<float>(tam.x) / 2.0f,
                             static_cast<float>(tam.y) / 2.0f});
            llave.setPosition(posicionLlave);
            llave.setScale({0.18f, 0.18f});
            ventana.draw(llave);
            return;
        }

        sf::CircleShape cabeza(8.0f);
        cabeza.setOrigin({8.0f, 8.0f});
        cabeza.setPosition(posicionLlave);
        cabeza.setFillColor(sf::Color(255, 220, 60));
        cabeza.setOutlineThickness(2.0f);
        cabeza.setOutlineColor(sf::Color(80, 55, 10));
        ventana.draw(cabeza);

        sf::RectangleShape cuerpo({28.0f, 6.0f});
        cuerpo.setOrigin({0.0f, 3.0f});
        cuerpo.setPosition({posicionLlave.x + 6.0f, posicionLlave.y});
        cuerpo.setFillColor(sf::Color(255, 220, 60));
        cuerpo.setOutlineThickness(1.0f);
        cuerpo.setOutlineColor(sf::Color(80, 55, 10));
        ventana.draw(cuerpo);

        sf::RectangleShape diente({5.0f, 12.0f});
        diente.setPosition({posicionLlave.x + 27.0f, posicionLlave.y});
        diente.setFillColor(sf::Color(255, 220, 60));
        ventana.draw(diente);
    }

    void dibujarMensajePantalla() {
        if (!hayFuenteHUD || tiempoMensaje <= 0.0f || mensajePantalla.empty()) {
            return;
        }

        sf::Text texto(fuenteHUD, mensajePantalla, 24);
        texto.setFillColor(sf::Color(255, 240, 140));
        texto.setOutlineColor(sf::Color::Black);
        texto.setOutlineThickness(3.0f);
        auto bounds = texto.getLocalBounds();
        texto.setOrigin({bounds.size.x / 2.0f, 0.0f});
        texto.setPosition({400.0f, 82.0f});
        ventana.draw(texto);
    }

    void dibujarIndicadorLlaveHUD() {
        if (!llaveObtenida) {
            return;
        }

        if (hayTexturaLlave) {
            sf::Sprite llave(texturaLlave);
            auto tam = texturaLlave.getSize();
            llave.setOrigin({static_cast<float>(tam.x) / 2.0f,
                             static_cast<float>(tam.y) / 2.0f});
            llave.setPosition({55.0f, 56.0f});
            llave.setScale({0.07f, 0.07f});
            ventana.draw(llave);
            return;
        }

        sf::CircleShape cabeza(6.0f);
        cabeza.setOrigin({6.0f, 6.0f});
        cabeza.setPosition({52.0f, 52.0f});
        cabeza.setFillColor(sf::Color(255, 220, 60));
        cabeza.setOutlineThickness(2.0f);
        cabeza.setOutlineColor(sf::Color::Black);
        ventana.draw(cabeza);

        sf::RectangleShape cuerpo({22.0f, 5.0f});
        cuerpo.setOrigin({0.0f, 2.5f});
        cuerpo.setPosition({58.0f, 52.0f});
        cuerpo.setFillColor(sf::Color(255, 220, 60));
        cuerpo.setOutlineThickness(1.0f);
        cuerpo.setOutlineColor(sf::Color::Black);
        ventana.draw(cuerpo);
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

        salaActual = SALA_A;
        puertaSecretaDesbloqueada = false;
        llaveDisponible = false;
        llaveObtenida = false;
        stacksVelocidad = 0;
        stacksEstrella = 0;
        tiempoVelocidad = 0.0f;
        tiempoEstrella = 0.0f;
        if (isaac) {
            isaac->setMultiplicadorVelocidad(1.0f);
        }
        mensajePantalla.clear();
        tiempoMensaje = 0.0f;

        for (auto& sala : salas) {
            sala.visitada = false;
            sala.limpiada = false;
        }
        salas[SALA_E].izquierda = -1;
        salas[salaActual].visitada = true;

        isaac->reiniciar(400.0f, 300.0f);
        cargarEnemigosSala();
    }

    void actualizar(float dt) {
        if (tiempoMensaje > 0.0f) {
            tiempoMensaje -= dt;
        }

        actualizarBuffs(dt);
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
            if (eliminar) {
                intentarSoltarObjeto(enemigos[i].get());
                enemigos.erase(enemigos.begin() + i);
            } else {
                i++;
            }
        }

        if (enemigos.empty() && !salas[salaActual].limpiada) {
            salas[salaActual].limpiada = true;

            if (salaActual == SALA_E && !puertaSecretaDesbloqueada) {
                puertaSecretaDesbloqueada = true;
                salas[SALA_E].izquierda = SALA_SECRETA;
                mostrarMensaje("Busca una puerta invisible", 4.0f);
            }

            if (salaActual == SALA_SECRETA && !llaveObtenida) {
                llaveDisponible = true;
                posicionLlave = {400.0f, 300.0f};
                mostrarMensaje("Aparecio una llave", 3.0f);
            }
        }

        actualizarLlave();
        actualizarObjetosSuelo();
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

        dibujarObjetosSuelo();
        dibujarLlave();
        dibujarHUD();
        dibujarIndicadorLlaveHUD();
        dibujarMiniMapa();
        dibujarMensajePantalla();
        ventana.display();
    }

public:
    Juego()
        : ventana(sf::VideoMode({800, 600}), "Aventuras en el Edificio de Software"),
          hayTexturaCorazon(false),
          hayTexturaLlave(false),
          hayTexturaObjetoCorazon(false),
          hayTexturaObjetoEstrella(false),
          hayTexturaObjetoRayo(false),
          salaActual(SALA_A),
          bloqueoCambioSala(false),
          puertaSecretaDesbloqueada(false),
          llaveDisponible(false),
          llaveObtenida(false),
          posicionLlave(400.0f, 300.0f),
          stacksVelocidad(0),
          stacksEstrella(0),
          tiempoVelocidad(0.0f),
          tiempoEstrella(0.0f),
          tiempoMensaje(0.0f),
          hayFuenteHUD(false) {
        ventana.setFramerateLimit(60);
        std::srand(static_cast<unsigned>(std::time(nullptr)));

        hayFuenteHUD = fuenteHUD.openFromFile("assets/fonts/MedievalSharp.ttf");
        if (!hayFuenteHUD) {
            hayFuenteHUD = fuenteHUD.openFromFile("assets/fonts/arial.ttf");
        }

        hayTexturaCorazon = texturaCorazon.loadFromFile(
            "assets/images/heart_pixel_art_32x32.png"
        );

        hayTexturaLlave = texturaLlave.loadFromFile(
            "assets/images/key_pixel.png"
        );

        hayTexturaObjetoCorazon = texturaObjetoCorazon.loadFromFile(
            "assets/images/items/item_heart.png"
        );
        hayTexturaObjetoEstrella = texturaObjetoEstrella.loadFromFile(
            "assets/images/items/item_star.png"
        );
        hayTexturaObjetoRayo = texturaObjetoRayo.loadFromFile(
            "assets/images/items/item_lightning.png"
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
