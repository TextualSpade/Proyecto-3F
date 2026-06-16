# Proyecto 3F - Aventuras en el Edificio de Software

Juego 2D desarrollado en C++ con SFML para la materia Proyecto 252. El juego se inspira en una aventura tipo mazmorras dentro del contexto escolar del CETI Colomos: el jugador controla a un Ingeniero en Sistemas que debe recorrer salas, derrotar monstruos que representan estudiantes de software corrompidos y encontrar una llave secreta para acceder a la sala del jefe.

## Objetivo del juego

Explorar las salas del edificio, derrotar enemigos, recolectar objetos temporales y encontrar la llave oculta para entrar a la sala del jefe. La sala secreta se desbloquea al limpiar la sala verde y contiene la llave necesaria para avanzar al combate final.

## Controles

- W/A/S/D: mover al jugador.
- I/J/K/L: disparar arriba, izquierda, abajo y derecha.
- Flechas o W/S: navegar en el menú.
- Enter o Espacio: seleccionar opción del menú.
- H: mostrar hitboxes de depuración.
- Cualquier tecla: continuar desde portada o pantalla de Game Over.

## Mecánicas principales

- Sistema de salas conectadas con minimapa.
- Salas no visitadas ocultas en negro.
- Puerta secreta invisible desbloqueada al limpiar la sala verde.
- Llave necesaria para entrar a la sala del jefe.
- Drops de objetos por enemigo.
- Corazón: cura vida.
- Rayo: aumenta temporalmente la velocidad.
- Estrella: activa disparo triple temporal.
- Game Over con oscurecimiento progresivo y regreso al menú.

## Enemigos

- Blob: enemigo lento de contacto.
- Raptor: enemigo que dispara a distancia.
- Rusher: enemigo que embiste al jugador.
- Spreadshot: enemigo que dispara en varias direcciones.

## Características

- Portada inicial con música.
- Menú principal con opciones de jugar, Wikipedia, controles y salir.
- Wikipedia interna con historia de personajes, enemigos e ítems.
- Música diferente para menú, juego normal y sala de jefe.
- Efectos de sonido para disparos, daño, puertas, objetos y eventos.
- Sistema de buffs stackeables.

## Tecnologías

- Lenguaje: C++20.
- Librería gráfica/audio: SFML.
- Compilador: MinGW64 / MSYS2.
- Editor recomendado: Visual Studio Code.
- Build: makefile.

## Cómo compilar

Desde la raíz del proyecto:

```powershell
make
```

## Cómo ejecutar

```powershell
.\bin\main.exe
```

## Estructura solicitada por Proyecto 252

El proyecto debe conservar las carpetas requeridas por CETUS:

```text
.github/workflows/publish.yml
video/demo.mp4
gallery/cover.png
screenshots/screenshot1.png
screenshots/screenshot2.png
screenshots/screenshot3.png
bin/JuegoProyecto.exe o bin/main.exe
assets/
README.md
.gitignore
```

## Equipo

- Líder: Nicolás Pelayo Zárate 24310295.
- Coautoría de commits: Sofia Alejandra Cervantes Cedano 24310303.

## Créditos

Proyecto escolar desarrollado para CETI Colomos. Los recursos visuales y de audio incluidos se usan dentro del contexto académico del proyecto.
