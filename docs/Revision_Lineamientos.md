# Revisión de lineamientos Proyecto 252

## Resultado general

El código fuente, assets principales, galería y workflow existen, pero el ZIP revisado todavía no cumple al 100% con la estructura requerida por CETUS porque faltan archivos de entrega obligatorios.

## Estado de carpetas y archivos

| Requisito | Estado en el ZIP revisado | Observación |
|---|---:|---|
| `.github/workflows/publish.yml` | OK | GitHub Action presente. |
| `gallery/cover.png` | OK | Portada presente. |
| `README.md` | OK | Se reemplaza por una versión editada para el juego. |
| `assets/` | OK | Contiene imágenes, fuentes y música. |
| `assets/sounds/` | FALTA | En el ZIP revisado no apareció esta carpeta; si ya la tienes localmente, agrégala al commit. |
| `video/demo.mp4` | FALTA | Falta crear/cargar video de gameplay máximo 2 minutos. |
| `screenshots/*.png` | FALTA | Se requieren mínimo 3 capturas PNG. Encontradas: 0. |
| `bin/*.exe` | FALTA | En el ZIP revisado no hay ejecutable. Debes compilar y subir el `.exe` si CETUS lo pide. |

## Revisión del diagrama UML

El archivo anterior `docs/Juego.puml` no representaba el código actual. Tenía clases que no existen actualmente en el proyecto, como `Oso`, `Gato`, `Perro`, `Conejo`, `Fantasma`, `Iguana` y `Hongo`.

Se corrigió el diagrama para reflejar las clases reales:

- `Juego`
- `Jugador`
- `Enemigo`
- `Blob`
- `Raptor`
- `Rusher`
- `Spreadshot`
- `Animacion`
- `ProyectilJugador`
- `Lagrima`
- `ProyectilRaptor`
- `ProyectilSpreadshot`
- `DatosSala`
- `ObjetoSuelo`
- `TipoObjeto`

También se corrigieron relaciones:

- Herencia: `Blob`, `Raptor`, `Rusher` y `Spreadshot` son enemigos.
- Composición: `Jugador` y enemigos contienen animaciones.
- Agregación: `Juego` administra enemigos, proyectiles, salas y objetos.
- Uso: `Juego` usa `TipoObjeto` y `Animacion`.

## Pendientes antes de entrega final

1. Crear o copiar `video/demo.mp4`.
2. Crear o copiar mínimo 3 capturas:
   - `screenshots/screenshot1.png`
   - `screenshots/screenshot2.png`
   - `screenshots/screenshot3.png`
3. Compilar y verificar que exista el ejecutable en `bin/`.
4. Confirmar que `assets/sounds/` esté en el repositorio si los sonidos se usan en código.
5. Hacer push a `main` y revisar el log del GitHub Action.
