# Conquer.io

Conquer.io es un juego de estrategia en tiempo real inspirado en los juegos de conquista territorial. El jugador y el bot se expanden sobre casillas neutrales en oleadas; el objetivo es controlar la mayor parte del mapa.

## Arquitectura general

El proyecto sigue una arquitectura muy sencilla basada en:

- Estado del juego (menú y partida) gestionado por una máquina de estados.
- Lógica de mapa y expansión territorial centralizada en una clase `Game`.
- Entidades `Player`, `Bot` y `TerritoryCell` que representan las tropas y el territorio.
- Raylib como motor gráfico y de entrada para la ventana, dibujo y eventos.

La estructura principal es:

```text
conquer.io/
├── src/               # Código fuente del juego
├── vendor/            # Librerías y cabeceras de Raylib
├── game               # Binario generado tras compilar
├── README.md          # Documentación del proyecto
└── .gitignore
```

## Qué hace cada parte

### `src/main.cpp`
Es el punto de entrada del programa. Aquí se crea la ventana con Raylib, se inicializa la máquina de estados y se ejecuta el bucle principal del juego:

- `InitWindow(...)` crea la ventana.
- `StateMachine` decide qué estado está activo.
- El bucle mientras la aplicación no termine:
  - calcula el `deltaTime`,
  - actualiza el estado actual,
  - procesa la entrada,
  - renderiza el frame.

### `src/StateMachine.hpp` / `src/StateMachine.cpp`
Implementa la máquina de estados del juego. Permite:

- añadir un estado nuevo,
- reemplazar el estado actual,
- retirar estados,
- cambiar entre menús y partidas,
- saber si la aplicación debe cerrarse.

Esto es importante porque el juego se compone de varios "escenarios": menú principal y partida principal.

### `src/GameState.hpp`
Define la interfaz base de un estado del juego. Todos los estados deben implementar:

- `init()`
- `handleInput()`
- `update(float deltaTime)`
- `render()`
- `pause()`
- `resume()`

Es la base común para que cualquier pantalla del juego pueda comportarse igual.

### `src/MenuState.hpp` / `src/MenuState.cpp`
Representa la pantalla principal del menú.

Hace lo siguiente:

- permite escribir el nombre del jugador,
- valida el largo máximo del texto,
- borra texto con retroceso continuo,
- selecciona un color para el territorio,
- al pulsar Enter con un nombre válido, cambia al estado de juego principal.

La interfaz dibuja el título, el campo de nombre, la paleta de colores y mensajes de ayuda.

### `src/MainGameState.hpp` / `src/MainGameState.cpp`
Es la pantalla de juego real. Aquí se construye y se controla la partida:

- obtiene el puntero del mouse para seleccionar una celda,
- procesa clic izquierdo para iniciar una expansión en oleadas desde el territorio del jugador,
- procesa clic derecho o ESC para cancelar la orden,
- detecta victoria/derrota y reinicio con `R`,
- dibuja el mapa, la UI, estadísticas del territorio, contador y mensajes finales.

### `src/Game.hpp` / `src/Game.cpp`
Es el núcleo de la lógica del juego. Esta clase contiene:

- el mapa de casillas (`std::vector<TerritoryCell>`),
- el jugador humano (`Player`),
- el bot enemigo (`Bot`),
- el tiempo de partida,
- estado de fin de partida (`winner`, `game_over`).

Su ciclo de actualización:

- hace crecer tropas de casillas controladas con el tiempo,
- expande en cada intervalo a todas las celdas neutrales que tocan el territorio del jugador o del bot por sus cuatro lados,
- inicia automáticamente las oleadas del bot,
- detiene cada oleada al encontrar agua o territorio del otro jugador.

### `src/Player.hpp` / `src/Player.cpp`
Representa al jugador humano.

Tiene:

- nombre,
- color del territorio,
- objetivo de expansión actual,
- temporizador de expansión,
- temporizador para marcar el intervalo entre oleadas de expansión.

Cuando el usuario hace clic en una casilla neutral, el jugador inicia oleadas que ocupan todas las casillas neutrales que tocan su territorio por los cuatro lados. Cada rama se detiene ante el agua o el territorio de otro dueño. La expansión no consume tropas y continúa hasta que ya no quedan casillas neutrales adyacentes.

### `src/Bot.hpp` / `src/Bot.cpp`
Representa la IA enemiga del bot.

Su comportamiento:

- se expande automáticamente en intervalos temporales,
- ocupa todas las celdas neutrales que tocan su territorio por sus cuatro lados,
- detiene cada rama al encontrar agua o territorio del jugador.

### `src/TerritoryCell.hpp`
Define la estructura de cada celda del mapa.

Cada casilla guarda:

- `owner`: quién la controla (`Neutral`, `Player`, `Bot`),
- `troops`: número de tropas que tiene,
- `capture_protection`: protección temporal tras ser capturada,
- `combat_timer`: tiempo de conflicto tras un ataque,
- `protected_from`: quien está protegido para evitar retomas inmediatas,
- `is_base`: si esa casilla es la base de una facción.

La lógica del mapa se basa en estas casillas para calcular expansión, defensa y captura.

### `src/vendor/`
Contiene las bibliotecas y cabeceras de Raylib que usa el proyecto para renderizar gráficos, entrada de teclado/mouse y audio. Gracias a esto, no hace falta instalar Raylib en el sistema para compilar.

### `game`
Es el ejecutable compilado del juego. Se genera al compilar el proyecto con `g++` y se usa para lanzarlo desde la terminal.

## Compilación y ejecución (Linux)

Este proyecto utiliza Raylib. Las librerías estáticas y cabeceras ya están incluidas en `vendor/`, así que no es necesario instalar Raylib en el sistema para compilar.

### 1. Compilar

```bash
./build.sh        # compila
./build.sh run    # compila y ejecuta
./build.sh clean  # borra el binario generado
```

Equivale a este comando manual:

```bash
g++ -o game src/*.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
```

### 2. Ejecutar

```bash
./game
```

## Cómo se juega

- Introduce un nombre y elige un color en el menú.
- Haz clic en una casilla neutral para expandirte en oleadas por los cuatro costados.
- Haz clic derecho o pulsa `ESC` para cancelar la orden.
- Las tropas crecen con el tiempo.
- La expansión del jugador se detiene al tocar territorio enemigo; no captura la base del bot.
- Pulsa `R` al terminar una partida para empezar otra.

## Resumen de la arquitectura

El proyecto está organizado como un juego pequeño de escritorio con un patrón clásico de máquina de estados:

- `MenuState` gestiona la introducción del nombre y la elección de color.
- `MainGameState` ejecuta la partida.
- `Game` resuelve la lógica de mapa y expansión territorial.
- `Player` y `Bot` representan a los actores del juego.
- `TerritoryCell` describe cada una de las piezas del mapa.
- `Raylib` se encarga de la ventana, renderizado y entrada del usuario.

Esta estructura hace que el juego sea fácil de extender: se puede añadir más pantallas, cambiar el comportamiento del bot o mejorar la lógica de expansión sin reescribir todo el proyecto.