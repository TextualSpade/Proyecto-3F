#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>

class Lagrima {
private:
    sf::Vector2f posicion;
    sf::Vector2f direccion;
    float velocidad;
    float radio;
    sf::CircleShape forma;
    bool destruida;

public:
    Lagrima(sf::Vector2f posInicial, sf::Vector2f dir) {
        posicion = posInicial;
        direccion = dir;
        velocidad = 500.0f;
        radio = 6.0f;
        destruida = false;
        forma.setRadius(radio);
        forma.setFillColor(sf::Color::Cyan);
        forma.setOrigin({radio, radio});
        forma.setPosition(posicion);
    }

    void actualizar(float dt) {
        posicion += direccion * velocidad * dt;
        forma.setPosition(posicion);
        if (posicion.x < 0 || posicion.x > 800 || posicion.y < 0 || posicion.y > 600) {
            destruida = true;
        }
    }

    void dibujar(sf::RenderWindow& ventana) { ventana.draw(forma); }
    bool estaDestruida() const { return destruida; }
};

class Jugador {
private:
    sf::Vector2f posicion;
    float velocidad;
    float radio;
    sf::CircleShape forma;

public:
    Jugador(float x, float y) {
        posicion = {x, y};
        velocidad = 300.0f;
        radio = 20.0f;
        forma.setRadius(radio);
        forma.setFillColor(sf::Color::Red);
        forma.setOrigin({radio, radio});
    }

    void actualizar(float dt) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) posicion.y -= velocidad * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) posicion.y += velocidad * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) posicion.x -= velocidad * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) posicion.x += velocidad * dt;

        if (posicion.x < 40.0f)  posicion.x = 40.0f;
        if (posicion.x > 760.0f) posicion.x = 760.0f;
        if (posicion.y < 40.0f)  posicion.y = 40.0f;
        if (posicion.y > 560.0f) posicion.y = 560.0f;

        forma.setPosition(posicion);
    }

    void dibujar(sf::RenderWindow& ventana) { ventana.draw(forma); }
    sf::Vector2f getPosicion() const { return posicion; }
};

class Juego {
private:
    sf::RenderWindow ventana;
    std::unique_ptr<Jugador> isaac;
    std::vector<Lagrima> lagrimas;
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

    void actualizar(float dt) {
        isaac->actualizar(dt);
        manejarDisparos();
        for (size_t i = 0; i < lagrimas.size(); ) {
            lagrimas[i].actualizar(dt);
            if (lagrimas[i].estaDestruida()) lagrimas.erase(lagrimas.begin() + i);
            else i++;
        }
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
        for (auto& lagrima : lagrimas) lagrima.dibujar(ventana);
        ventana.display();
    }

public:
    Juego() : ventana(sf::VideoMode({800, 600}), "The Binding of Isaac - Alpha SFML 3") {
        ventana.setFramerateLimit(60);
        isaac = std::make_unique<Jugador>(400.0f, 300.0f);
        tiempoEntreDisparos = 0.3f;
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