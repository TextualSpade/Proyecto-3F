#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
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
#include <cstdint>
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


enum class EstadoDragon {
    Inactivo,
    Entrada,
    Combate,
    Invocando,
    Muriendo,
    Derrotado
};

struct DragonBoss {
    EstadoDragon estado = EstadoDragon::Inactivo;
    sf::Vector2f posicion{400.0f, -160.0f};
    sf::Vector2f destino{400.0f, 235.0f};
    int vida = 100;
    int vidaMaxima = 100;
    int fase = 1;
    bool invocacionFase1 = false;
    bool invocacionFase2 = false;
    bool invocacionFase3 = false;
    float tiempoEstado = 0.0f;
    float tiempoDisparo = 0.0f;
    float tiempoAnimacion = 0.0f;
    int frame = 0;
    int fila = 0;
    float direccionMovimiento = 1.0f;
    float tiempoPoderFase = 0.0f;
    int poderesCreadosFase = 0;
};

struct ProyectilDragon {
    sf::Vector2f posicion;
    sf::Vector2f velocidad;
    float radio = 15.0f;
    int rebotesRestantes = 0;
    bool destruido = false;
    float tiempoVida = 0.0f;
    int frame = 0;
    float tiempoAnimacion = 0.0f;
};

static constexpr float LIMITE_IZQ = 40.0f;
static constexpr float LIMITE_DER = 760.0f;
static constexpr float LIMITE_ARR = 40.0f;
static constexpr float LIMITE_ABA = 560.0f;
static constexpr float CENTRO_X = 400.0f;
static constexpr float CENTRO_Y = 300.0f;
static constexpr float ANCHO_PUERTA = 110.0f;

static constexpr float ANCHO_VIRTUAL = 800.0f;
static constexpr float ALTO_VIRTUAL = 600.0f;


struct RectPantalla {
    float x;
    float y;
    float w;
    float h;
};

static RectPantalla calcularRectContainPantalla(const sf::Vector2u& tamVentana,
                                               const sf::Vector2u& tamImagen,
                                               float margenX = 0.015f,
                                               float margenY = 0.015f) {
    if (tamImagen.x == 0 || tamImagen.y == 0 || tamVentana.x == 0 || tamVentana.y == 0) {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }

    float anchoDisponible = static_cast<float>(tamVentana.x) * (1.0f - 2.0f * margenX);
    float altoDisponible  = static_cast<float>(tamVentana.y) * (1.0f - 2.0f * margenY);

    float escalaX = anchoDisponible / static_cast<float>(tamImagen.x);
    float escalaY = altoDisponible  / static_cast<float>(tamImagen.y);
    float escala = std::min(escalaX, escalaY);

    float anchoFinal = static_cast<float>(tamImagen.x) * escala;
    float altoFinal  = static_cast<float>(tamImagen.y) * escala;

    float x = (static_cast<float>(tamVentana.x) - anchoFinal) / 2.0f;
    float y = (static_cast<float>(tamVentana.y) - altoFinal) / 2.0f;

    return {x, y, anchoFinal, altoFinal};
}

static void aplicarRectASprite(sf::Sprite& sprite,
                               const RectPantalla& rect,
                               const sf::Vector2u& tamImagen) {
    if (tamImagen.x == 0 || tamImagen.y == 0) return;

    sprite.setPosition({rect.x, rect.y});
    sprite.setScale({
        rect.w / static_cast<float>(tamImagen.x),
        rect.h / static_cast<float>(tamImagen.y)
    });
}

static sf::View crearVistaLetterbox(const sf::RenderWindow& ventana) {
    sf::View vista(sf::FloatRect({0.0f, 0.0f}, {ANCHO_VIRTUAL, ALTO_VIRTUAL}));

    float anchoVentana = static_cast<float>(ventana.getSize().x);
    float altoVentana = static_cast<float>(ventana.getSize().y);

    float proporcionVentana = anchoVentana / altoVentana;
    float proporcionJuego = ANCHO_VIRTUAL / ALTO_VIRTUAL;

    float viewportX = 0.0f;
    float viewportY = 0.0f;
    float viewportW = 1.0f;
    float viewportH = 1.0f;

    if (proporcionVentana > proporcionJuego) {
        viewportW = proporcionJuego / proporcionVentana;
        viewportX = (1.0f - viewportW) / 2.0f;
    } else if (proporcionVentana < proporcionJuego) {
        viewportH = proporcionVentana / proporcionJuego;
        viewportY = (1.0f - viewportH) / 2.0f;
    }

    vista.setViewport(sf::FloatRect({viewportX, viewportY}, {viewportW, viewportH}));
    return vista;
}


class Juego {
private:
    sf::RenderWindow ventana;
    sf::Texture texturaPortada;
    sf::Texture texturaMenuPrincipal;
    bool hayTexturaPortada;
    bool hayTexturaMenuPrincipal;
    sf::Music musicaPortada;
    sf::Music musicaMenu;
    sf::Music musicaFondo;
    std::string pistaFondoActual;

    sf::SoundBuffer bufferMenuSelect;
    sf::SoundBuffer bufferPlayerShoot;
    sf::SoundBuffer bufferPlayerHurt;
    sf::SoundBuffer bufferPlayerDeath;
    sf::SoundBuffer bufferDoorEnter;
    sf::SoundBuffer bufferLockedDoor;
    sf::SoundBuffer bufferSecretFound;
    sf::SoundBuffer bufferPowerHeart;
    sf::SoundBuffer bufferPowerLightning;
    sf::SoundBuffer bufferPowerStar;
    sf::SoundBuffer bufferPowerKey;
    sf::SoundBuffer bufferBlobHurt;
    sf::SoundBuffer bufferBlobDeath;
    sf::SoundBuffer bufferRaptorShoot;
    sf::SoundBuffer bufferRaptorHurt;
    sf::SoundBuffer bufferRusherCharge;
    sf::SoundBuffer bufferSpreadshotShoot;

    bool hayMenuSelect;
    bool hayPlayerShoot;
    bool hayPlayerHurt;
    bool hayPlayerDeath;
    bool hayDoorEnter;
    bool hayLockedDoor;
    bool haySecretFound;
    bool hayPowerHeart;
    bool hayPowerLightning;
    bool hayPowerStar;
    bool hayPowerKey;
    bool hayBlobHurt;
    bool hayBlobDeath;
    bool hayRaptorShoot;
    bool hayRaptorHurt;
    bool hayRusherCharge;
    bool haySpreadshotShoot;

    std::vector<sf::Sound> sonidosActivos;
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
    sf::Texture texturaDragonBoss;
    sf::Texture texturaFuegoDragon;
    bool hayTexturaDragonBoss;
    bool hayTexturaFuegoDragon;
    DragonBoss dragonBoss;
    std::vector<ProyectilDragon> proyectilesDragon;
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

    void dibujarImagenPantallaCompleta(const sf::Texture& textura) {
        sf::Sprite sprite(textura);
        sf::Vector2u tam = textura.getSize();
        if (tam.x == 0 || tam.y == 0) return;

        // Ajuste tipo "contain": muestra la imagen completa sin recortarla.
        // Si sobra espacio por los lados o arriba/abajo, queda negro.
        float escalaX = 800.0f / static_cast<float>(tam.x);
        float escalaY = 600.0f / static_cast<float>(tam.y);
        float escala = std::min(escalaX, escalaY);

        sprite.setScale({escala, escala});
        sprite.setPosition({
            (800.0f - static_cast<float>(tam.x) * escala) / 2.0f,
            (600.0f - static_cast<float>(tam.y) * escala) / 2.0f
        });

        ventana.draw(sprite);
    }


    void cargarSonidos() {
        hayMenuSelect = bufferMenuSelect.loadFromFile("assets/sounds/menu_select.wav");
        hayPlayerShoot = bufferPlayerShoot.loadFromFile("assets/sounds/player_shoot.wav");
        hayPlayerHurt = bufferPlayerHurt.loadFromFile("assets/sounds/player_hurt.wav");
        hayPlayerDeath = bufferPlayerDeath.loadFromFile("assets/sounds/player_death.wav");
        hayDoorEnter = bufferDoorEnter.loadFromFile("assets/sounds/door_enter.wav");
        hayLockedDoor = bufferLockedDoor.loadFromFile("assets/sounds/locked_door.wav");
        haySecretFound = bufferSecretFound.loadFromFile("assets/sounds/secret_found.wav");
        hayPowerHeart = bufferPowerHeart.loadFromFile("assets/sounds/power_heart.wav");
        hayPowerLightning = bufferPowerLightning.loadFromFile("assets/sounds/power_lightning.wav");
        hayPowerStar = bufferPowerStar.loadFromFile("assets/sounds/power_star.wav");
        hayPowerKey = bufferPowerKey.loadFromFile("assets/sounds/power_key.wav");
        hayBlobHurt = bufferBlobHurt.loadFromFile("assets/sounds/blob_hurt.wav");
        hayBlobDeath = bufferBlobDeath.loadFromFile("assets/sounds/blob_death.wav");
        hayRaptorShoot = bufferRaptorShoot.loadFromFile("assets/sounds/raptor_shoot.wav");
        hayRaptorHurt = bufferRaptorHurt.loadFromFile("assets/sounds/raptor_hurt.wav");
        hayRusherCharge = bufferRusherCharge.loadFromFile("assets/sounds/rusher_charge.wav");
        haySpreadshotShoot = bufferSpreadshotShoot.loadFromFile("assets/sounds/spreadshot_shoot.wav");
    }

    void reproducirSonido(const sf::SoundBuffer& buffer, bool disponible, float volumen = 60.0f) {
        if (!disponible) return;

        if (sonidosActivos.size() > 28) {
            sonidosActivos.erase(sonidosActivos.begin());
        }

        sonidosActivos.emplace_back(buffer);
        sonidosActivos.back().setVolume(volumen);
        sonidosActivos.back().play();
    }

    void actualizarMusicaFondoSala() {
        std::string nuevaPista = (salaActual == SALA_I)
            ? "assets/music/boss_theme.wav"
            : "assets/music/game_theme.wav";

        if (pistaFondoActual == nuevaPista) {
            return;
        }

        musicaFondo.stop();
        pistaFondoActual.clear();

        if (musicaFondo.openFromFile(nuevaPista)) {
            musicaFondo.setLooping(true);
            musicaFondo.setVolume(salaActual == SALA_I ? 52.0f : 38.0f);
            musicaFondo.play();
            pistaFondoActual = nuevaPista;
        }
    }

    void detenerMusicaFondo() {
        musicaFondo.stop();
        pistaFondoActual.clear();
    }

    void reproducirSonidoDanioEnemigo(Enemigo* enemigo) {
        if (dynamic_cast<Blob*>(enemigo)) {
            reproducirSonido(bufferBlobHurt, hayBlobHurt, 55.0f);
        } else if (dynamic_cast<Raptor*>(enemigo)) {
            reproducirSonido(bufferRaptorHurt, hayRaptorHurt, 55.0f);
        } else if (dynamic_cast<Rusher*>(enemigo)) {
            reproducirSonido(bufferRusherCharge, hayRusherCharge, 45.0f);
        } else if (dynamic_cast<Spreadshot*>(enemigo)) {
            reproducirSonido(bufferSpreadshotShoot, haySpreadshotShoot, 38.0f);
        }
    }

    void reproducirSonidoMuerteEnemigo(Enemigo* enemigo) {
        if (dynamic_cast<Blob*>(enemigo)) {
            reproducirSonido(bufferBlobDeath, hayBlobDeath, 55.0f);
        } else if (dynamic_cast<Raptor*>(enemigo)) {
            reproducirSonido(bufferRaptorHurt, hayRaptorHurt, 50.0f);
        } else if (dynamic_cast<Rusher*>(enemigo)) {
            reproducirSonido(bufferRusherCharge, hayRusherCharge, 45.0f);
        } else if (dynamic_cast<Spreadshot*>(enemigo)) {
            reproducirSonido(bufferSpreadshotShoot, haySpreadshotShoot, 40.0f);
        }
    }

    bool mostrarPortadaInicial() {
        if (musicaPortada.openFromFile("assets/music/title_theme.wav")) {
            musicaPortada.setLooping(true);
            musicaPortada.setVolume(45.0f);
            musicaPortada.play();
        }

        sf::Clock relojPortada;

        while (ventana.isOpen()) {
            while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
                if (evento->is<sf::Event::Closed>()) {
                    ventana.close();
                    return false;
                }

                if (evento->is<sf::Event::KeyPressed>() ||
                    evento->is<sf::Event::MouseButtonPressed>()) {
                    musicaPortada.stop();
                    return true;
                }
            }

            float tiempo = relojPortada.getElapsedTime().asSeconds();
            float progreso = std::min(tiempo / 2.5f, 1.0f);
            int alphaNegro = static_cast<int>(255.0f * (1.0f - progreso));

            ventana.clear(sf::Color::Black);

            if (hayTexturaPortada) {
                dibujarImagenPantallaCompleta(texturaPortada);
            } else {
                sf::RectangleShape fondo({800.0f, 600.0f});
                fondo.setFillColor(sf::Color(10, 10, 20));
                ventana.draw(fondo);
            }

            if (hayFuenteHUD && tiempo > 1.6f) {
                sf::Text texto(fuenteHUD, "Presione cualquier tecla para continuar", 26);
                float parpadeo = (std::sin(tiempo * 4.0f) + 1.0f) / 2.0f;
                std::uint8_t alpha = static_cast<std::uint8_t>(150 + 105 * parpadeo);
                texto.setFillColor(sf::Color(255, 235, 150, alpha));
                texto.setOutlineColor(sf::Color(30, 15, 5, alpha));
                texto.setOutlineThickness(2.0f);
                auto bounds = texto.getLocalBounds();
                texto.setOrigin({bounds.size.x / 2.0f, bounds.size.y / 2.0f});
                texto.setPosition({400.0f, 545.0f});
                ventana.draw(texto);
            }

            if (alphaNegro > 0) {
                sf::RectangleShape capaNegra({800.0f, 600.0f});
                capaNegra.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(alphaNegro)));
                ventana.draw(capaNegra);
            }

            ventana.display();
        }

        return false;
    }

    void dibujarLineaWikipedia(const sf::Font& fuente,
                               const std::string& texto,
                               float x,
                               float& y,
                               unsigned int tam,
                               sf::Color color,
                               bool titulo = false) {
        sf::Text linea(fuente, texto, tam);
        linea.setFillColor(color);
        linea.setOutlineColor(sf::Color(20, 10, 5));
        linea.setOutlineThickness(titulo ? 2.0f : 1.0f);
        linea.setPosition({x, y});
        ventana.draw(linea);
        y += static_cast<float>(tam) + (titulo ? 14.0f : 8.0f);
    }

    void mostrarWikipedia() {
        sf::Font fuente;
        bool hayFuente = fuente.openFromFile("assets/fonts/MedievalSharp.ttf");
        if (!hayFuente) hayFuente = fuente.openFromFile("assets/fonts/arial.ttf");

        if (!hayFuente) {
            mostrarMensaje("No se pudo cargar la fuente", 2.0f);
            return;
        }

        std::vector<std::vector<std::string>> paginas = {
            {
                "WIKIPEDIA - PERSONAJES",
                "Ingeniero en Sistemas: Un estudiante elegido por el edificio. Tras quedarse",
                "programando hasta tarde, encontro una falla extrana en los laboratorios y",
                "desperto con una espada de energia hecha de codigo compilado.",
                "",
                "Blob: Antes era un alumno que nunca cerraba sus ciclos while. Con el tiempo,",
                "sus errores se acumularon hasta convertirlo en una masa lenta de tareas",
                "pendientes, cafes frios y codigo sin identar.",
                "",
                "Raptor: Fue un estudiante brillante de estructuras de datos, pero intento",
                "optimizar tanto sus proyectos que termino fusionandose con sus propios",
                "algoritmos. Ahora ataca desde lejos y calcula trayectorias como si fueran",
                "examenes sorpresa."
            },
            {
                "WIKIPEDIA - ENEMIGOS",
                "Rusher: Era el estudiante que dejaba todo al ultimo minuto. Corrio tanto",
                "para entregar practicas, reportes y proyectos finales que su ansiedad se",
                "volvio su unica forma de moverse. Si te ve, se lanza sin pensarlo.",
                "",
                "Spreadshot: Fue un alumno de software que abria veinte pestañas, tres IDEs",
                "y cinco chats al mismo tiempo. Su mente se fragmento y ahora dispara en",
                "varias direcciones, como si cada proyectil fuera una tarea olvidada.",
                "",
                "Sala Secreta: Un pasillo que no aparece en el mapa oficial del CETI. Dicen",
                "que ahi se guardan los proyectos que nunca se pudieron compilar."
            },
            {
                "WIKIPEDIA - ITEMS",
                "Corazon: Recupera un corazon de vida. Se dice que aparece cuando el edificio",
                "reconoce que el estudiante aun tiene oportunidad de pasar el semestre.",
                "",
                "Rayo: Aumenta la velocidad 50% durante 30 segundos. Si tomas otro, se",
                "acumula el efecto y tambien se suma el tiempo. Es energia pura de cafe,",
                "estres y motivacion de ultima hora.",
                "",
                "Estrella: Activa el disparo triple durante 15 segundos. Si tomas otra, el",
                "efecto se acumula y dura mas. Representa ese momento raro en el que todo",
                "el codigo funciona a la primera.",
                "",
                "Llave: Abre la sala del jefe. Solo aparece en la sala secreta despues de",
                "derrotar a todos los enemigos que custodian el camino oculto."
            }
        };

        int pagina = 0;
        while (ventana.isOpen()) {
            while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
                if (evento->is<sf::Event::Closed>()) {
                    ventana.close();
                    return;
                }

                if (const auto* kp = evento->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::Escape ||
                        kp->code == sf::Keyboard::Key::Enter ||
                        kp->code == sf::Keyboard::Key::Backspace) {
                        return;
                    }

                    if (kp->code == sf::Keyboard::Key::Right ||
                        kp->code == sf::Keyboard::Key::D ||
                        kp->code == sf::Keyboard::Key::Space) {
                        pagina = (pagina + 1) % static_cast<int>(paginas.size());
                        reproducirSonido(bufferMenuSelect, hayMenuSelect, 50.0f);
                    }

                    if (kp->code == sf::Keyboard::Key::Left ||
                        kp->code == sf::Keyboard::Key::A) {
                        pagina = (pagina + static_cast<int>(paginas.size()) - 1) %
                                 static_cast<int>(paginas.size());
                        reproducirSonido(bufferMenuSelect, hayMenuSelect, 50.0f);
                    }
                }
            }

            ventana.clear(sf::Color(8, 8, 12));

            sf::RectangleShape panel({720.0f, 500.0f});
            panel.setPosition({40.0f, 50.0f});
            panel.setFillColor(sf::Color(45, 28, 18, 235));
            panel.setOutlineColor(sf::Color(225, 180, 65));
            panel.setOutlineThickness(4.0f);
            ventana.draw(panel);

            float y = 78.0f;
            for (size_t i = 0; i < paginas[pagina].size(); i++) {
                bool esTitulo = i == 0;
                dibujarLineaWikipedia(
                    fuente,
                    paginas[pagina][i],
                    70.0f,
                    y,
                    esTitulo ? 30u : 18u,
                    esTitulo ? sf::Color(255, 225, 120) : sf::Color(235, 220, 185),
                    esTitulo
                );
            }

            sf::Text ayuda(fuente, "A/D o Flechas: cambiar pagina   |   Enter/Esc: regresar", 16);
            ayuda.setFillColor(sf::Color(190, 170, 120));
            ayuda.setPosition({75.0f, 525.0f});
            ventana.draw(ayuda);

            sf::Text contador(fuente,
                "Pagina " + std::to_string(pagina + 1) + " / " + std::to_string(paginas.size()),
                16);
            contador.setFillColor(sf::Color(255, 220, 100));
            contador.setPosition({610.0f, 525.0f});
            ventana.draw(contador);

            ventana.display();
        }
    }

    bool mostrarMenuPrincipal() {
        musicaPortada.stop();
        ventana.setView(ventana.getDefaultView());

        if (musicaMenu.openFromFile("assets/music/menu_theme.wav")) {
            musicaMenu.setLooping(true);
            musicaMenu.setVolume(45.0f);
            musicaMenu.play();
        }

        int opcion = 0;
        const int totalOpciones = 4;

        // Coordenadas tomadas sobre la imagen original del menu_principal.png.
        // Así el selector queda centrado aunque la imagen se escale en pantalla completa.
        const float selectorCentroXImagen = 683.0f;
        const std::array<float, 4> selectorCentroYImagen = {204.0f, 337.0f, 469.0f, 624.0f};
        const float selectorAnchoImagen = 390.0f;
        const float selectorAltoImagen = 82.0f;

        while (ventana.isOpen()) {
            while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
                if (evento->is<sf::Event::Closed>()) {
                    ventana.close();
                    return false;
                }

                if (const auto* kp = evento->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::Up ||
                        kp->code == sf::Keyboard::Key::W) {
                        opcion = (opcion + totalOpciones - 1) % totalOpciones;
                        reproducirSonido(bufferMenuSelect, hayMenuSelect, 55.0f);
                    }

                    if (kp->code == sf::Keyboard::Key::Down ||
                        kp->code == sf::Keyboard::Key::S) {
                        opcion = (opcion + 1) % totalOpciones;
                        reproducirSonido(bufferMenuSelect, hayMenuSelect, 55.0f);
                    }

                    if (kp->code == sf::Keyboard::Key::Enter ||
                        kp->code == sf::Keyboard::Key::Space) {
                        reproducirSonido(bufferMenuSelect, hayMenuSelect, 65.0f);

                        if (opcion == 0) {
                            musicaMenu.stop();
                            ventana.setView(crearVistaLetterbox(ventana));
                            return true;
                        }

                        if (opcion == 1) {
                            mostrarWikipedia();
                            ventana.setView(ventana.getDefaultView());
                        } else if (opcion == 2) {
                            mostrarMensaje("Controles: WASD mover, IJKL disparar", 2.5f);
                        } else if (opcion == 3) {
                            ventana.close();
                            return false;
                        }
                    }

                    if (kp->code == sf::Keyboard::Key::Escape) {
                        ventana.close();
                        return false;
                    }
                }
            }

            if (tiempoMensaje > 0.0f) {
                tiempoMensaje -= reloj.restart().asSeconds();
            } else {
                reloj.restart();
            }

            ventana.setView(ventana.getDefaultView());
            ventana.clear(sf::Color::Black);

            RectPantalla rectMenu{0.0f, 0.0f, static_cast<float>(ventana.getSize().x), static_cast<float>(ventana.getSize().y)};

            if (hayTexturaMenuPrincipal) {
                sf::Sprite fondoMenu(texturaMenuPrincipal);
                rectMenu = calcularRectContainPantalla(
                    ventana.getSize(),
                    texturaMenuPrincipal.getSize(),
                    0.008f,
                    0.008f
                );
                aplicarRectASprite(fondoMenu, rectMenu, texturaMenuPrincipal.getSize());
                ventana.draw(fondoMenu);
            } else {
                sf::RectangleShape fondo({static_cast<float>(ventana.getSize().x), static_cast<float>(ventana.getSize().y)});
                fondo.setFillColor(sf::Color(25, 15, 10));
                ventana.draw(fondo);
            }

            if (hayTexturaMenuPrincipal && texturaMenuPrincipal.getSize().x > 0 && texturaMenuPrincipal.getSize().y > 0) {
                float escalaX = rectMenu.w / static_cast<float>(texturaMenuPrincipal.getSize().x);
                float escalaY = rectMenu.h / static_cast<float>(texturaMenuPrincipal.getSize().y);

                float selectorW = selectorAnchoImagen * escalaX;
                float selectorH = selectorAltoImagen * escalaY;
                float selectorX = rectMenu.x + selectorCentroXImagen * escalaX;
                float selectorY = rectMenu.y + selectorCentroYImagen[opcion] * escalaY;

                sf::RectangleShape selector({selectorW, selectorH});
                selector.setOrigin({selectorW / 2.0f, selectorH / 2.0f});
                selector.setPosition({selectorX, selectorY});
                selector.setFillColor(sf::Color(255, 220, 80, 38));
                selector.setOutlineColor(sf::Color(255, 225, 90));
                selector.setOutlineThickness(std::max(3.0f, selectorH * 0.045f));
                ventana.draw(selector);
            }

            dibujarMensajePantalla();

            ventana.display();
        }

        return false;
    }

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
                        reproducirSonido(bufferMenuSelect, hayMenuSelect, 65.0f);
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
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                if (tecla->code == sf::Keyboard::Key::Escape) {
                    ventana.close();
                }
            }
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

            reproducirSonido(bufferPlayerShoot, hayPlayerShoot, 42.0f);
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
                    reproducirSonido(bufferRaptorShoot, hayRaptorShoot, 35.0f);
                } else if (dynamic_cast<Spreadshot*>(enemigo.get())) {
                    proyectilesSpreadshot.push_back(
                        std::make_unique<ProyectilSpreadshot>(enemigo->getPosicion(), dirSalida));
                    reproducirSonido(bufferSpreadshotShoot, haySpreadshotShoot, 35.0f);
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

    sf::Vector2f centroHitboxEnemigo(Enemigo* enemigo) const {
        if (!enemigo) return {0.0f, 0.0f};

        sf::Vector2f centro = enemigo->getPosicion();

        // Los sprites de enemigos tienen espacio transparente alrededor.
        // Este pequeño ajuste centra mejor la zona real de contacto.
        if (dynamic_cast<Blob*>(enemigo)) {
            centro += sf::Vector2f(0.0f, 4.0f);
        } else if (dynamic_cast<Raptor*>(enemigo)) {
            centro += sf::Vector2f(0.0f, 6.0f);
        } else if (dynamic_cast<Rusher*>(enemigo)) {
            centro += sf::Vector2f(0.0f, 5.0f);
        } else if (dynamic_cast<Spreadshot*>(enemigo)) {
            centro += sf::Vector2f(0.0f, 5.0f);
        }

        return centro;
    }

    float radioHitboxEnemigo(Enemigo* enemigo) const {
        if (!enemigo) return 0.0f;

        // Un poco más grandes que antes, pero sin rebasar visualmente el sprite.
        if (dynamic_cast<Blob*>(enemigo)) return 23.0f;
        if (dynamic_cast<Raptor*>(enemigo)) return 27.0f;
        if (dynamic_cast<Rusher*>(enemigo)) return 25.0f;
        if (dynamic_cast<Spreadshot*>(enemigo)) return 26.0f;

        return enemigo->getRadio();
    }

    float probabilidadDropEnemigo(Enemigo* enemigo) const {
        // Drop rates ajustados: un poco más altos que antes,
        // pero sin llenar toda la sala de objetos.
        if (dynamic_cast<Blob*>(enemigo)) return 0.20f;
        if (dynamic_cast<Raptor*>(enemigo)) return 0.25f;
        if (dynamic_cast<Rusher*>(enemigo)) return 0.30f;
        if (dynamic_cast<Spreadshot*>(enemigo)) return 0.30f;
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
                reproducirSonido(bufferPowerHeart, hayPowerHeart, 65.0f);
                mostrarMensaje("Corazon +1", 1.8f);
                break;

            case TipoObjeto::Rayo:
                stacksVelocidad++;
                tiempoVelocidad += 30.0f;
                isaac->setMultiplicadorVelocidad(1.0f + 0.5f * static_cast<float>(stacksVelocidad));
                reproducirSonido(bufferPowerLightning, hayPowerLightning, 65.0f);
                mostrarMensaje("Velocidad aumentada", 2.2f);
                break;

            case TipoObjeto::Estrella:
                stacksEstrella++;
                tiempoEstrella += 15.0f;
                reproducirSonido(bufferPowerStar, hayPowerStar, 65.0f);
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
            recibirDanioDragon(proy);
        }

        for (auto& proy : proyectilesJugador) {
            for (auto& enemigo : enemigos) {
                if (!proy.estaDestruido() && !enemigo->estaMuerto() &&
                    colisionan(proy.getPosicion(), proy.getRadio(),
                               centroHitboxEnemigo(enemigo.get()),
                               radioHitboxEnemigo(enemigo.get()))) {
                    reproducirSonidoDanioEnemigo(enemigo.get());
                    enemigo->recibirDanio(1);
                    proy.destruir();
                }
            }
        }

        for (auto& enemigo : enemigos) {
            if (!enemigo->estaMuerto() &&
                colisionan(centroHitboxEnemigo(enemigo.get()),
                           radioHitboxEnemigo(enemigo.get()),
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                if (!isaac->esInvulnerable()) {
                    reproducirSonido(bufferPlayerHurt, hayPlayerHurt, 55.0f);
                }
                isaac->recibirDanio(1);
                if (auto* blob = dynamic_cast<Blob*>(enemigo.get())) blob->tocarJugador();
                if (auto* rusher = dynamic_cast<Rusher*>(enemigo.get())) rusher->golpearJugador();
            }
        }

        for (auto& lagrima : lagrimasEnemigas) {
            if (!lagrima.estaDestruida() &&
                colisionan(lagrima.getPosicion(), lagrima.getRadio(),
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                if (!isaac->esInvulnerable()) {
                    reproducirSonido(bufferPlayerHurt, hayPlayerHurt, 55.0f);
                }
                isaac->recibirDanio(1);
                lagrima.destruir();
            }
        }

        for (auto& proy : proyectilesRaptor) {
            if (!proy->estaDestruido() &&
                colisionan(proy->getPosicion(), proy->getRadio(),
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                if (!isaac->esInvulnerable()) {
                    reproducirSonido(bufferPlayerHurt, hayPlayerHurt, 55.0f);
                }
                isaac->recibirDanio(1);
                proy->golpear();
            }
        }

        for (auto& proy : proyectilesSpreadshot) {
            if (!proy->estaDestruido() &&
                colisionan(proy->getPosicion(), proy->getRadio(),
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                if (!isaac->esInvulnerable()) {
                    reproducirSonido(bufferPlayerHurt, hayPlayerHurt, 55.0f);
                }
                isaac->recibirDanio(1);
                proy->golpear();
            }
        }

        for (auto& proy : proyectilesDragon) {
            if (!proy.destruido &&
                colisionan(proy.posicion, proy.radio,
                           isaac->getCentroHitbox(), isaac->getRadio())) {
                if (!isaac->esInvulnerable()) {
                    reproducirSonido(bufferPlayerHurt, hayPlayerHurt, 55.0f);
                }
                isaac->recibirDanio(1);
                proy.destruido = true;
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
        proyectilesDragon.clear();
        enemigos.clear();
        objetosSuelo.clear();
        if (salaActual != SALA_I) {
            dragonBoss.estado = EstadoDragon::Inactivo;
        }
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
                // I = sala del jefe final. El dragón se maneja fuera del vector de enemigos.
                iniciarDragonBoss();
                break;

            default:
                break;
        }
    }


    void reiniciarDragonBoss() {
        dragonBoss = DragonBoss{};
        proyectilesDragon.clear();
    }

    void iniciarDragonBoss() {
        reiniciarDragonBoss();
        dragonBoss.estado = EstadoDragon::Entrada;
        dragonBoss.posicion = {400.0f, -170.0f};
        dragonBoss.destino = {400.0f, 235.0f};
        dragonBoss.vida = 100;
        dragonBoss.vidaMaxima = 100;
        dragonBoss.fase = 1;
        dragonBoss.fila = 2;      // attack: se usa para la entrada desde el cielo
        dragonBoss.frame = 0;
        dragonBoss.tiempoEstado = 0.0f;
        dragonBoss.tiempoDisparo = 0.0f;
        dragonBoss.tiempoAnimacion = 0.0f;
        dragonBoss.poderesCreadosFase = 0;
        dragonBoss.tiempoPoderFase = 2.5f;
        mostrarMensaje("El dragon ha despertado", 2.5f);
    }

    bool dragonActivo() const {
        return salaActual == SALA_I &&
               dragonBoss.estado != EstadoDragon::Inactivo &&
               dragonBoss.estado != EstadoDragon::Derrotado;
    }

    bool dragonPuedeRecibirDanio() const {
        return dragonBoss.estado == EstadoDragon::Combate;
    }

    int calcularFaseDragon() const {
        if (dragonBoss.vida > 66) return 1;
        if (dragonBoss.vida > 33) return 2;
        return 3;
    }

    void actualizarAnimacionDragon(float dt) {
        dragonBoss.tiempoAnimacion += dt;
        float velocidad = 0.12f;
        int maxFrames = 3;

        switch (dragonBoss.estado) {
            case EstadoDragon::Entrada:
                dragonBoss.fila = 2; // attack
                maxFrames = 4;
                velocidad = 0.10f;
                break;
            case EstadoDragon::Combate:
                dragonBoss.fila = 1; // walk: movimiento leve en el centro
                maxFrames = 5;
                velocidad = 0.14f;
                break;
            case EstadoDragon::Invocando:
                dragonBoss.fila = 4; // hurt / quieto
                maxFrames = 2;
                velocidad = 0.20f;
                break;
            case EstadoDragon::Muriendo:
                dragonBoss.fila = 5; // death
                maxFrames = 5;
                velocidad = 0.18f;
                break;
            default:
                dragonBoss.fila = 0;
                maxFrames = 3;
                velocidad = 0.16f;
                break;
        }

        if (dragonBoss.tiempoAnimacion >= velocidad) {
            dragonBoss.tiempoAnimacion = 0.0f;
            if (dragonBoss.estado == EstadoDragon::Muriendo) {
                if (dragonBoss.frame < maxFrames - 1) dragonBoss.frame++;
            } else {
                dragonBoss.frame = (dragonBoss.frame + 1) % maxFrames;
            }
        }
    }

    void soltarPoderBossAleatorio() {
        TipoObjeto tipo = TipoObjeto::Corazon;
        int r = std::rand() % 100;
        if (r < 34) tipo = TipoObjeto::Corazon;
        else if (r < 67) tipo = TipoObjeto::Rayo;
        else tipo = TipoObjeto::Estrella;

        std::vector<sf::Vector2f> puntos = {
            {210.0f, 230.0f}, {590.0f, 230.0f}, {235.0f, 430.0f},
            {565.0f, 430.0f}, {400.0f, 420.0f}, {400.0f, 180.0f}
        };

        sf::Vector2f pos = puntos[std::rand() % puntos.size()];
        objetosSuelo.push_back({tipo, pos});
    }

    void intentarPoderesBoss(float dt) {
        if (dragonBoss.estado != EstadoDragon::Combate) return;
        if (dragonBoss.poderesCreadosFase >= 3) return;

        dragonBoss.tiempoPoderFase -= dt;
        if (dragonBoss.tiempoPoderFase <= 0.0f) {
            soltarPoderBossAleatorio();
            dragonBoss.poderesCreadosFase++;
            dragonBoss.tiempoPoderFase = 6.5f + static_cast<float>(std::rand() % 400) / 100.0f;
        }
    }

    void dispararConoDragon(int cantidad, bool rebota) {
        sf::Vector2f origen = dragonBoss.posicion + sf::Vector2f(0.0f, 45.0f);
        sf::Vector2f objetivo = isaac ? isaac->getCentroHitbox() : sf::Vector2f{400.0f, 500.0f};
        sf::Vector2f base = objetivo - origen;
        float len = std::sqrt(base.x * base.x + base.y * base.y);
        if (len <= 0.001f) base = {0.0f, 1.0f};
        else base /= len;

        float separacion = 0.13f;
        float centro = static_cast<float>(cantidad - 1) / 2.0f;

        for (int i = 0; i < cantidad; i++) {
            float offset = (static_cast<float>(i) - centro) * separacion;
            sf::Vector2f dir = direccionRotada(base, offset);
            ProyectilDragon p;
            p.posicion = origen;
            p.velocidad = dir * 245.0f;
            p.radio = 15.0f;
            p.rebotesRestantes = rebota ? 1 : 0;
            proyectilesDragon.push_back(p);
        }
    }

    void spawnearAddsDragon(int fase) {
        using T = TipoEnemigoSpawn;
        if (fase == 1) {
            spawnearGrupo({T::Blob, T::Raptor});
            mostrarMensaje("El dragon llama estudiantes corrompidos", 2.4f);
        } else if (fase == 2) {
            if (std::rand() % 2 == 0) spawnearGrupo({T::Spreadshot, T::Rusher});
            else spawnearGrupo({T::Spreadshot, T::Spreadshot});
            mostrarMensaje("El dragon se cubre con refuerzos", 2.4f);
        } else {
            spawnearGrupo({T::Rusher, T::Spreadshot});
            mostrarMensaje("Ultima defensa del dragon", 2.4f);
        }

        dragonBoss.estado = EstadoDragon::Invocando;
        dragonBoss.frame = 0;
        dragonBoss.tiempoEstado = 0.0f;
        proyectilesDragon.clear();
    }

    void revisarInvocacionDragon() {
        if (dragonBoss.estado != EstadoDragon::Combate) return;

        if (dragonBoss.fase == 1 && !dragonBoss.invocacionFase1 && dragonBoss.vida <= 83) {
            dragonBoss.invocacionFase1 = true;
            spawnearAddsDragon(1);
        } else if (dragonBoss.fase == 2 && !dragonBoss.invocacionFase2 && dragonBoss.vida <= 50) {
            dragonBoss.invocacionFase2 = true;
            spawnearAddsDragon(2);
        } else if (dragonBoss.fase == 3 && !dragonBoss.invocacionFase3 && dragonBoss.vida <= 16) {
            dragonBoss.invocacionFase3 = true;
            spawnearAddsDragon(3);
        }
    }

    void actualizarDragonBoss(float dt) {
        if (salaActual != SALA_I || dragonBoss.estado == EstadoDragon::Inactivo ||
            dragonBoss.estado == EstadoDragon::Derrotado) {
            return;
        }

        dragonBoss.tiempoEstado += dt;
        actualizarAnimacionDragon(dt);

        if (dragonBoss.estado == EstadoDragon::Entrada) {
            float progreso = std::min(dragonBoss.tiempoEstado / 2.7f, 1.0f);
            float suave = progreso * progreso * (3.0f - 2.0f * progreso);
            dragonBoss.posicion.y = -170.0f + (dragonBoss.destino.y + 170.0f) * suave;
            dragonBoss.posicion.x = 400.0f;

            if (progreso >= 1.0f) {
                dragonBoss.estado = EstadoDragon::Combate;
                dragonBoss.frame = 0;
                dragonBoss.tiempoEstado = 0.0f;
                dragonBoss.tiempoDisparo = 1.0f;
                mostrarMensaje("Derrota al Dragon del Edificio", 2.5f);
            }
            return;
        }

        if (dragonBoss.estado == EstadoDragon::Invocando) {
            if (enemigos.empty()) {
                dragonBoss.estado = EstadoDragon::Combate;
                dragonBoss.frame = 0;
                dragonBoss.tiempoEstado = 0.0f;
                dragonBoss.tiempoDisparo = 1.0f;
                mostrarMensaje("El dragon vuelve a atacar", 1.8f);
            }
            return;
        }

        if (dragonBoss.estado == EstadoDragon::Muriendo) {
            proyectilesDragon.clear();
            if (dragonBoss.tiempoEstado > 3.0f) {
                dragonBoss.estado = EstadoDragon::Derrotado;
                salas[SALA_I].limpiada = true;
                mostrarMensaje("Dragon derrotado", 4.0f);
            }
            return;
        }

        if (dragonBoss.estado == EstadoDragon::Combate) {
            int nuevaFase = calcularFaseDragon();
            if (nuevaFase != dragonBoss.fase) {
                dragonBoss.fase = nuevaFase;
                dragonBoss.poderesCreadosFase = 0;
                dragonBoss.tiempoPoderFase = 2.0f;
                mostrarMensaje("Fase " + std::to_string(dragonBoss.fase), 1.8f);
            }

            // Movimiento pequeño para que se sienta vivo, sin abandonar el centro de la sala.
            dragonBoss.posicion.x += dragonBoss.direccionMovimiento * 22.0f * dt;
            if (dragonBoss.posicion.x > 450.0f) dragonBoss.direccionMovimiento = -1.0f;
            if (dragonBoss.posicion.x < 350.0f) dragonBoss.direccionMovimiento = 1.0f;

            dragonBoss.tiempoDisparo -= dt;
            if (dragonBoss.tiempoDisparo <= 0.0f) {
                int cantidad = (dragonBoss.fase == 1) ? 3 : (dragonBoss.fase == 2 ? 5 : 6);
                bool rebota = dragonBoss.fase == 3;
                dispararConoDragon(cantidad, rebota);
                dragonBoss.fila = 3;
                dragonBoss.frame = 0;
                dragonBoss.tiempoDisparo = (dragonBoss.fase == 1) ? 2.25f : (dragonBoss.fase == 2 ? 1.75f : 1.35f);
            }

            revisarInvocacionDragon();
            intentarPoderesBoss(dt);
        }
    }

    void actualizarProyectilesDragon(float dt) {
        for (auto& p : proyectilesDragon) {
            if (p.destruido) continue;

            p.posicion += p.velocidad * dt;
            p.tiempoVida += dt;
            p.tiempoAnimacion += dt;
            if (p.tiempoAnimacion >= 0.08f) {
                p.tiempoAnimacion = 0.0f;
                p.frame = (p.frame + 1) % 6;
            }

            if (p.rebotesRestantes > 0) {
                bool reboto = false;
                if (p.posicion.x <= LIMITE_IZQ + p.radio || p.posicion.x >= LIMITE_DER - p.radio) {
                    p.velocidad.x *= -1.0f;
                    p.rebotesRestantes--;
                    reboto = true;
                }
                if (!reboto && (p.posicion.y <= LIMITE_ARR + p.radio || p.posicion.y >= LIMITE_ABA - p.radio)) {
                    p.velocidad.y *= -1.0f;
                    p.rebotesRestantes--;
                }
            } else if (p.posicion.x < LIMITE_IZQ - 60.0f || p.posicion.x > LIMITE_DER + 60.0f ||
                       p.posicion.y < LIMITE_ARR - 60.0f || p.posicion.y > LIMITE_ABA + 60.0f) {
                p.destruido = true;
            }

            if (p.tiempoVida > 7.0f) p.destruido = true;
        }

        proyectilesDragon.erase(
            std::remove_if(proyectilesDragon.begin(), proyectilesDragon.end(),
                           [](const ProyectilDragon& p) { return p.destruido; }),
            proyectilesDragon.end());
    }

    void recibirDanioDragon(ProyectilJugador& proy) {
        if (!dragonPuedeRecibirDanio() || proy.estaDestruido()) return;

        sf::Vector2f centro = dragonBoss.posicion + sf::Vector2f(0.0f, 20.0f);
        if (colisionan(proy.getPosicion(), proy.getRadio(), centro, 92.0f)) {
            dragonBoss.vida--;
            proy.destruir();
            dragonBoss.fila = 4;
            dragonBoss.frame = 0;

            if (dragonBoss.vida <= 0) {
                dragonBoss.vida = 0;
                dragonBoss.estado = EstadoDragon::Muriendo;
                dragonBoss.frame = 0;
                dragonBoss.tiempoEstado = 0.0f;
                enemigos.clear();
                proyectilesDragon.clear();
                objetosSuelo.clear();
                mostrarMensaje("El dragon cae", 3.0f);
            }
        }
    }

    void dibujarDragonBoss() {
        if (salaActual != SALA_I || dragonBoss.estado == EstadoDragon::Inactivo ||
            dragonBoss.estado == EstadoDragon::Derrotado || !hayTexturaDragonBoss) {
            return;
        }

        sf::Sprite sprite(texturaDragonBoss);
        sprite.setTextureRect(sf::IntRect({dragonBoss.frame * 256, dragonBoss.fila * 256}, {256, 256}));
        sprite.setOrigin({128.0f, 128.0f});
        sprite.setPosition(dragonBoss.posicion);
        sprite.setScale({1.42f, 1.42f});
        ventana.draw(sprite);
    }

    void dibujarProyectilesDragon() {
        for (const auto& p : proyectilesDragon) {
            if (hayTexturaFuegoDragon) {
                sf::Sprite fuego(texturaFuegoDragon);
                fuego.setTextureRect(sf::IntRect({p.frame * 128, 0}, {128, 128}));
                fuego.setOrigin({64.0f, 64.0f});
                fuego.setPosition(p.posicion);
                float angulo = std::atan2(p.velocidad.y, p.velocidad.x) * 180.0f / 3.14159265f;
                fuego.setRotation(sf::degrees(angulo));
                fuego.setScale({0.42f, 0.42f});
                ventana.draw(fuego);
            } else {
                sf::CircleShape bola(p.radio);
                bola.setOrigin({p.radio, p.radio});
                bola.setPosition(p.posicion);
                bola.setFillColor(sf::Color(255, 95, 15));
                bola.setOutlineThickness(3.0f);
                bola.setOutlineColor(sf::Color(255, 220, 80));
                ventana.draw(bola);
            }
        }
    }

    void dibujarBarraVidaDragon() {
        if (salaActual != SALA_I || dragonBoss.estado == EstadoDragon::Inactivo ||
            dragonBoss.estado == EstadoDragon::Derrotado) {
            return;
        }

        float progreso = static_cast<float>(dragonBoss.vida) / static_cast<float>(dragonBoss.vidaMaxima);
        if (dragonBoss.estado == EstadoDragon::Entrada) {
            progreso = std::min(dragonBoss.tiempoEstado / 2.7f, 1.0f);
        }

        sf::RectangleShape fondo({560.0f, 22.0f});
        fondo.setPosition({120.0f, 18.0f});
        fondo.setFillColor(sf::Color(25, 10, 15, 230));
        fondo.setOutlineThickness(3.0f);
        fondo.setOutlineColor(sf::Color(210, 170, 55));
        ventana.draw(fondo);

        sf::RectangleShape vida({560.0f * progreso, 22.0f});
        vida.setPosition({120.0f, 18.0f});
        vida.setFillColor(sf::Color(170, 35, 70, 240));
        ventana.draw(vida);

        for (int i = 1; i < 3; i++) {
            sf::RectangleShape corte({4.0f, 28.0f});
            corte.setPosition({120.0f + 560.0f * static_cast<float>(i) / 3.0f - 2.0f, 15.0f});
            corte.setFillColor(sf::Color(245, 210, 100));
            ventana.draw(corte);
        }

        if (hayFuenteHUD) {
            sf::Text nombre(fuenteHUD, "DRAGON DEL EDIFICIO", 18);
            nombre.setFillColor(sf::Color(255, 235, 150));
            nombre.setOutlineColor(sf::Color::Black);
            nombre.setOutlineThickness(2.0f);
            auto b = nombre.getLocalBounds();
            nombre.setOrigin({b.size.x / 2.0f, 0.0f});
            nombre.setPosition({400.0f, 45.0f});
            ventana.draw(nombre);
        }
    }

    bool puedeEntrarSala(int nuevaSala) {
        if (nuevaSala == SALA_I && !llaveObtenida) {
            reproducirSonido(bufferLockedDoor, hayLockedDoor, 65.0f);
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
        proyectilesDragon.clear();
        enemigos.clear();
        reiniciarDragonBoss();

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
        actualizarMusicaFondoSala();
    }

    void actualizar(float dt) {
        if (tiempoMensaje > 0.0f) {
            tiempoMensaje -= dt;
        }

        actualizarBuffs(dt);

        bool jugadorBloqueadoPorBoss =
            (salaActual == SALA_I && dragonBoss.estado == EstadoDragon::Entrada);

        if (!jugadorBloqueadoPorBoss) {
            isaac->actualizar(dt);
            manejarDisparos();
        }

        for (auto& enemigo : enemigos) {
            enemigo->setObjetivo(isaac->getCentroHitbox());
            enemigo->actualizar(dt);
        }

        manejarDisparosEnemigos();
        actualizarDragonBoss(dt);

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

        actualizarProyectilesDragon(dt);
        manejarColisiones();

        for (size_t i = 0; i < enemigos.size(); ) {
            bool eliminar = false;
            if (auto* b  = dynamic_cast<Blob*>(enemigos[i].get()))       eliminar = b->listo_para_eliminar();
            else if (auto* r  = dynamic_cast<Raptor*>(enemigos[i].get())) eliminar = r->listo_para_eliminar();
            else if (auto* ru = dynamic_cast<Rusher*>(enemigos[i].get())) eliminar = ru->listo_para_eliminar();
            else if (auto* s  = dynamic_cast<Spreadshot*>(enemigos[i].get())) eliminar = s->listo_para_eliminar();
            else eliminar = enemigos[i]->estaMuerto();
            if (eliminar) {
                reproducirSonidoMuerteEnemigo(enemigos[i].get());
                intentarSoltarObjeto(enemigos[i].get());
                enemigos.erase(enemigos.begin() + i);
            } else {
                i++;
            }
        }

        if (salaActual != SALA_I && enemigos.empty() && !salas[salaActual].limpiada) {
            salas[salaActual].limpiada = true;

            if (salaActual == SALA_E && !puertaSecretaDesbloqueada) {
                puertaSecretaDesbloqueada = true;
                salas[SALA_E].izquierda = SALA_SECRETA;
                reproducirSonido(bufferSecretFound, haySecretFound, 70.0f);
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

        if (!jugadorBloqueadoPorBoss && dragonBoss.estado != EstadoDragon::Muriendo) {
            intentarCambioSala();
        }

        if (!isaac->estaVivo()) {
            reproducirSonido(bufferPlayerDeath, hayPlayerDeath, 65.0f);
            manejarGameOver();
            return;
        }
    }

    void dibujarEscenaSinDisplay() {
        ventana.clear(sf::Color(0, 0, 0));
        dibujarFondoSala();
        dibujarPuertasDisponibles();
        dibujarDragonBoss();

        if (isaac) isaac->dibujar(ventana);
        for (auto& enemigo : enemigos) enemigo->dibujar(ventana);
        for (auto& proy : proyectilesJugador) proy.dibujar(ventana);
        for (auto& lagrima : lagrimasEnemigas) lagrima.dibujar(ventana);
        for (auto& proy : proyectilesRaptor) proy->dibujar(ventana);
        for (auto& proy : proyectilesSpreadshot) proy->dibujar(ventana);
        dibujarProyectilesDragon();

        dibujarObjetosSuelo();
        dibujarLlave();
        dibujarHUD();
        dibujarIndicadorLlaveHUD();
        dibujarBarraVidaDragon();
        dibujarMiniMapa();
    }

    bool mostrarPantallaGameOver() {
        detenerMusicaFondo();

        sf::Clock relojOscurecer;
        sf::Clock relojAnimacion;
        bool puedeContinuar = false;

        while (ventana.isOpen()) {
            while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
                if (evento->is<sf::Event::Closed>()) {
                    ventana.close();
                    return false;
                }

                if (puedeContinuar &&
                    (evento->is<sf::Event::KeyPressed>() ||
                     evento->is<sf::Event::MouseButtonPressed>())) {
                    return true;
                }
            }

            float dt = relojAnimacion.restart().asSeconds();
            if (isaac) {
                isaac->actualizar(dt);
            }

            float tiempo = relojOscurecer.getElapsedTime().asSeconds();
            float progresoOscuro = std::min(tiempo / 2.4f, 1.0f);
            float parpadeo = (std::sin(tiempo * 4.0f) + 1.0f) * 0.5f;

            dibujarEscenaSinDisplay();

            sf::RectangleShape capaNegra({800.0f, 600.0f});
            std::uint8_t alphaNegro = static_cast<std::uint8_t>(220.0f * progresoOscuro);
            capaNegra.setFillColor(sf::Color(0, 0, 0, alphaNegro));
            ventana.draw(capaNegra);

            if (hayFuenteHUD && tiempo > 0.8f) {
                sf::Text gameOver(fuenteHUD, "GAME OVER", 72);
                gameOver.setFillColor(sf::Color(235, 185, 45));
                gameOver.setOutlineColor(sf::Color(55, 30, 5));
                gameOver.setOutlineThickness(5.0f);
                auto gb = gameOver.getLocalBounds();
                gameOver.setOrigin({gb.size.x / 2.0f, gb.size.y / 2.0f});
                gameOver.setPosition({400.0f, 250.0f});
                ventana.draw(gameOver);

                if (tiempo > 2.4f) {
                    puedeContinuar = true;
                    sf::Text continuar(fuenteHUD, "Presione cualquier tecla para continuar", 24);
                    std::uint8_t alphaTexto = static_cast<std::uint8_t>(150.0f + 105.0f * parpadeo);
                    continuar.setFillColor(sf::Color(255, 230, 140, alphaTexto));
                    continuar.setOutlineColor(sf::Color(30, 15, 0, alphaTexto));
                    continuar.setOutlineThickness(2.0f);
                    auto cb = continuar.getLocalBounds();
                    continuar.setOrigin({cb.size.x / 2.0f, cb.size.y / 2.0f});
                    continuar.setPosition({400.0f, 360.0f});
                    ventana.draw(continuar);
                }
            }

            ventana.display();
        }

        return false;
    }

    void manejarGameOver() {
        if (!mostrarPantallaGameOver()) {
            return;
        }

        if (!mostrarMenuPrincipal()) {
            ventana.close();
            return;
        }

        ventana.setView(crearVistaLetterbox(ventana));

        personajeElegido = seleccionarPersonaje();
        if (personajeElegido < 0) {
            ventana.close();
            return;
        }

        const ConfigPersonaje& cfg = PERSONAJES[personajeElegido];
        isaac = std::make_unique<Jugador>(400.0f, 300.0f,
                                          cfg.sheet1, cfg.sheet2, cfg.proyectil);

        reiniciarPartida();
        reloj.restart();
    }

    void renderizar() {
        ventana.setView(crearVistaLetterbox(ventana));
        ventana.clear(sf::Color(0, 0, 0));
        dibujarFondoSala();
        dibujarPuertasDisponibles();
        dibujarDragonBoss();

        isaac->dibujar(ventana);
        for (auto& enemigo : enemigos) enemigo->dibujar(ventana);
        for (auto& proy : proyectilesJugador) proy.dibujar(ventana);
        for (auto& lagrima : lagrimasEnemigas) lagrima.dibujar(ventana);
        for (auto& proy : proyectilesRaptor) proy->dibujar(ventana);
        for (auto& proy : proyectilesSpreadshot) proy->dibujar(ventana);
        dibujarProyectilesDragon();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H)) {
            isaac->dibujarHitbox(ventana);

            for (auto& enemigo : enemigos) {
                dibujarCirculoDebug(centroHitboxEnemigo(enemigo.get()), radioHitboxEnemigo(enemigo.get()), sf::Color::Yellow);
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

            if (dragonActivo()) {
                dibujarCirculoDebug(dragonBoss.posicion + sf::Vector2f(0.0f, 20.0f), 92.0f, sf::Color(255, 120, 0));
            }
            for (auto& proy : proyectilesDragon) {
                dibujarCirculoDebug(proy.posicion, proy.radio, sf::Color(255, 110, 20));
            }
        }

        dibujarObjetosSuelo();
        dibujarLlave();
        dibujarHUD();
        dibujarIndicadorLlaveHUD();
        dibujarBarraVidaDragon();
        dibujarMiniMapa();
        dibujarMensajePantalla();
        ventana.display();
    }

public:
    Juego()
        : ventana(sf::VideoMode::getDesktopMode(), "Aventuras en el Edificio de Software", sf::State::Fullscreen),
          hayTexturaPortada(false),
          hayTexturaMenuPrincipal(false),
          pistaFondoActual(""),
          hayMenuSelect(false),
          hayPlayerShoot(false),
          hayPlayerHurt(false),
          hayPlayerDeath(false),
          hayDoorEnter(false),
          hayLockedDoor(false),
          haySecretFound(false),
          hayPowerHeart(false),
          hayPowerLightning(false),
          hayPowerStar(false),
          hayPowerKey(false),
          hayBlobHurt(false),
          hayBlobDeath(false),
          hayRaptorShoot(false),
          hayRaptorHurt(false),
          hayRusherCharge(false),
          haySpreadshotShoot(false),
          hayTexturaCorazon(false),
          hayTexturaLlave(false),
          hayTexturaObjetoCorazon(false),
          hayTexturaObjetoEstrella(false),
          hayTexturaObjetoRayo(false),
          hayTexturaDragonBoss(false),
          hayTexturaFuegoDragon(false),
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
        ventana.setView(crearVistaLetterbox(ventana));
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        cargarSonidos();

        hayFuenteHUD = fuenteHUD.openFromFile("assets/fonts/MedievalSharp.ttf");
        if (!hayFuenteHUD) {
            hayFuenteHUD = fuenteHUD.openFromFile("assets/fonts/arial.ttf");
        }

        hayTexturaPortada = texturaPortada.loadFromFile(
            "assets/images/ui/portada.png"
        );
        hayTexturaMenuPrincipal = texturaMenuPrincipal.loadFromFile(
            "assets/images/ui/menu_principal.png"
        );

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

        hayTexturaDragonBoss = texturaDragonBoss.loadFromFile(
            "assets/images/boss/dragon_sheet.png"
        );
        hayTexturaFuegoDragon = texturaFuegoDragon.loadFromFile(
            "assets/images/boss/dragon_fire_sheet.png"
        );

        configurarSalas();

        tiempoEntreDisparos = 0.3f;

        if (!mostrarPortadaInicial()) {
            personajeElegido = -1;
            return;
        }

        if (!mostrarMenuPrincipal()) {
            personajeElegido = -1;
            return;
        }

        personajeElegido = seleccionarPersonaje();
        if (personajeElegido < 0) return;

        const ConfigPersonaje& cfg = PERSONAJES[personajeElegido];
        isaac = std::make_unique<Jugador>(400.0f, 300.0f,
                                          cfg.sheet1, cfg.sheet2, cfg.proyectil);

        salas[salaActual].visitada = true;
        cargarEnemigosSala();
        actualizarMusicaFondoSala();
    }

    void ejecutar() {
        if (personajeElegido < 0) return;
        reloj.restart();
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
