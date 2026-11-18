# Conquer.io

**Conquer.io** es un juego 2D de estrategia en tiempo real (RTS) en el que cada píxel de tierra puede convertirse en parte de tu territorio. Expándete, administra tus tropas y compite contra rivales controlados por inteligencia artificial hasta dominar el mapa.

---

## Sobre el juego

Conquer.io es un juego de estrategia en tiempo real inspirado en [Territorial.io](https://territorial.io). La partida se desarrolla sobre un mapa formado por píxeles: el agua no se puede conquistar, la tierra libre ofrece una expansión sencilla y los territorios enemigos requieren tropas para ser capturados.

El objetivo es conquistar el **100 % de la tierra ocupable** o ser el último jugador que permanezca en pie.

## Características

- **Mapas territoriales:** mapas representados mediante una cuadrícula de píxeles, con tierra, agua y territorios controlados por jugadores.
- **Conquista y combate:** expande tu territorio en las cuatro direcciones cardinales y ataca zonas rivales.
- **Gestión de tropas:** administra tus tropas y decide cuánto invertir en cada ataque.
- **Crecimiento de tropas:** sistema de crecimiento mediante interés decreciente y un límite relacionado con el territorio conquistado.
- **Rivales con IA:** bots con distintos niveles de dificultad y comportamientos estratégicos.
- **Interfaz minimalista:** el mapa y la información esencial son el centro de la experiencia.
- **Overlay de depuración:** panel de información técnica y de partida activable con `F1`.
- **Versión web:** el juego se compila a WebAssembly con Emscripten y se puede jugar desde el navegador.

## Cómo jugar

1. Inicia una partida y selecciona las opciones disponibles.
2. Elige dónde establecer tu territorio inicial durante la fase de colocación.
3. Cuando comience la partida, amplía tu dominio y acumula tropas.
4. Selecciona una zona y ataca cuando la acción esté disponible.
5. Equilibra la expansión con la reserva de tropas necesaria para enfrentarte a tus rivales.
6. Conquista toda la tierra ocupable para ganar.

### Territorio y ataques

- **Agua:** no se puede conquistar y no cuenta para el porcentaje total de territorio ocupable.
- **Tierra libre:** no ofrece resistencia, por lo que resulta más sencilla de capturar.
- **Territorio enemigo:** puede oponer resistencia y consumir tropas durante el ataque.
- **Fronteras:** la expansión se realiza arriba, abajo, a izquierda y derecha; las diagonales no cuentan como adyacencia.

### Tropas y crecimiento

Las tropas aumentan con el paso del tiempo mediante un sistema de interés que disminuye a medida que el jugador se aproxima a su límite de tropas. Ese límite depende de la cantidad de territorio conquistado. Atacar consume tropas, así que elegir cuándo expandirse forma parte de la estrategia.

## Inteligencia artificial

El diseño contempla varios niveles de dificultad para los bots. Los comportamientos previstos varían en agresividad, gestión de tropas, alianzas y coordinación de ataques.

La dificultad y las estrategias disponibles dependen de la implementación actual del juego.

## Overlay de depuración

Con `F1` se puede mostrar u ocultar una capa de depuración sobre la partida. Está pensada para consultar información de la simulación y del rendimiento sin abandonar el juego.

Entre los datos contemplados se encuentran:

- Entidades y dimensiones del mapa.
- Tropas, interés y porcentaje de territorio.
- Información de los bots.
- FPS y tiempo por frame.
- Datos técnicos del entorno gráfico y de ejecución.

## Tecnologías

| Tecnología | Uso |
|---|---|
| **C++** | Lógica y sistemas del juego |
| **raylib** | Ventana, gráficos, entrada y renderizado |
| **ImGui** | Interfaz del overlay de depuración |
| **rlImGui** | Integración de ImGui con raylib |
| **Emscripten** | Compilación a WebAssembly para la versión web |
| **spdlog** | Logging estructurado |
| **Git y GitHub** | Control de versiones y colaboración |
| **GitHub Issues y Pull Requests** | Seguimiento de tareas, incidencias y revisión de cambios |

## Instalación y ejecución

### Requisitos

Para compilar el proyecto desde el código fuente, necesitarás:

- Git.
- Un compilador compatible con C++.
- raylib y las dependencias necesarias para tu sistema operativo.
- Las dependencias adicionales incluidas o configuradas en el proyecto, como ImGui, rlImGui y spdlog.

### Descargar el repositorio

```bash
git clone https://github.com/ConquerIO/conquer.io.git
cd conquer.io
```

### Compilar y ejecutar

Para arquitecturas x86 utilizar:
```bash
./build.sh
```

Para arquitecturas ARM utilizar:
```bash
./buildArm64.sh
```

Y para ejecutar lanza el ejecutable:
```bash
./game
```

### Compilar con Makefile

Alternativa a los scripts, con compilación incremental (solo recompila lo que ha cambiado):

```bash
make            # compila (equivale a make all) y genera ./game
make run        # compila y ejecuta
make -j$(nproc) # compila en paralelo
make clean      # borra build/, build-web/ y el binario
```

| Comando | Qué hace |
|---|---|
| `make` / `make all` | Compila el juego en `./game`. |
| `make run` | Compila y ejecuta `./game`. |
| `make clean` | Borra `build/`, `build-web/` y el binario. |
| `make ARCH=arm64` | Fuerza la arquitectura (`x86` o `arm64`). Por defecto se detecta con `uname -m`. |
| `make web`, `make web-serve`, `make web-chrome` | Versión web (ver [más abajo](#versión-web-emscripten)). |
| `make ccache-stats` | Muestra las estadísticas de ccache. |
| `make clean-ccache-stats` | Pone a cero las estadísticas de ccache (conserva la caché). |
| `make clean-ccache` | Vacía la caché de ccache (conserva las estadísticas). |

Notas:

- Todos los `.cpp` bajo `src/` se detectan automáticamente; no hay que registrar archivos nuevos.
- Si [ccache](https://ccache.dev/) está instalado se usa solo, y las recompilaciones son mucho más rápidas.
- Los objetos se guardan en `build/<arquitectura>/`, así que x86 y ARM64 no se pisan.
- Para añadir una librería, amplía la variable `LIBS` del `Makefile`.

### Versión web (Emscripten)

El juego también se puede compilar a WebAssembly y ejecutar en el navegador.

#### Prueba rápida (desde cero)

```bash
# 1. Instalar emsdk (una sola vez)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest

# 2. Activarlo en esta terminal (hay que repetirlo en cada terminal nueva)
source ./emsdk_env.sh
em++ --version   # comprueba que está activo

# 3. Clonar, compilar y servir el juego
cd ..
git clone https://github.com/ConquerIO/conquer.io.git
cd conquer.io
make web-serve
```

Abre http://localhost:8080: debería cargar el menú del juego. Si no carga, abre la consola del navegador (F12) y comprueba que tiene WebGL. Si estás en una máquina virtual y no hay WebGL, usa `make web-chrome` (ver tabla de comandos).

#### Requisitos y detalles

Requisitos adicionales:

- [emsdk](https://emscripten.org/docs/getting_started/downloads.html) instalado y activo (pasos en la prueba rápida de arriba). No viene en el repositorio.
- Python 3, solo para servir los archivos en local.
- `vendor/lib/web/libraylib.a`: raylib compilada para web, que ya viene en el repositorio. Para regenerarla, desde el código fuente de raylib (`src/`): `make PLATFORM=PLATFORM_WEB`.

Comandos:

| Comando | Qué hace |
|---|---|
| `make web` | Compila y genera `build-web/index.html` (junto con `.js`, `.wasm` y `.data`). |
| `make web-serve` | Compila y sirve el juego en http://localhost:8080. |
| `make web-chrome` | Igual que `web-serve`, pero abre Chrome con `--ignore-gpu-blocklist` (útil en máquinas virtuales donde Chrome bloquea WebGL) y para el servidor al cerrarlo. |

Opciones: `WEB_PORT=8081` cambia el puerto y `CHROME=/ruta/al/navegador` elige el navegador.

`index.html` no funciona abriéndolo con doble clic: hay que servirlo por HTTP (por ejemplo con `make web-serve`).

Limitaciones de la versión web:

- Requiere un navegador con WebGL.
- El botón SALIR del menú está oculto.
- El overlay `F1` no muestra estadísticas del sistema (no existe `/proc` en el navegador).
- Solo hay controles de ratón y teclado.

## Desarrollo y contribuciones

El proyecto organiza el trabajo mediante ramas y Pull Requests para revisar los cambios antes de integrarlos en las ramas principales. Los mensajes de commit siguen la convención **Conventional Commits**.

Prefijos habituales:

- `feat:` nueva funcionalidad.
- `fix:` corrección de errores.
- `refactor:` mejora interna sin cambiar el comportamiento esperado.
- `docs:` cambios en documentación.

---

