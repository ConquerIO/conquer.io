# Conquer.io

Conquer.io es un juego de estrategia en tiempo real inspirado en los juegos de conquista territorial. El jugador compite contra 10 bots en un mapa generado al azar: cada uno elige dónde empezar, gasta tropas para expandirse sobre tierra libre y ataca el territorio de los demás. El objetivo es controlar la mayor parte del mapa.

## Arquitectura general

El proyecto sigue una arquitectura sencilla basada en:

- Estados del juego (menú y partida) gestionados por una máquina de estados.
- Lógica de mapa, tropas y expansión territorial centralizada en la clase `Game`.
- Entidades (`Entity`, con sus variantes `Player` y `Bot`) que tienen tropas, objetivo y frontera.
- Un mapa de casillas (`Map` y `TerritoryCell`) generado con ruido Perlin.
- Raylib como motor gráfico y de entrada para la ventana, el dibujo y los eventos.

La estructura principal es:

```text
conquer.io/
├── src/               # Código fuente del juego
├── vendor/            # Cabeceras y librería estática de Raylib (x86_64 y ARM64)
├── maps/              # Imágenes de mapas (actualmente no se usan)
├── build.sh           # Script de compilación (x86_64)
├── buildArm64.sh      # Script de compilación (ARM64)
├── game               # Binario generado tras compilar
└── README.md          # Documentación del proyecto
```

## Qué hace cada parte

### `src/main.cpp`
Es el punto de entrada del programa:

- inicializa la semilla aleatoria,
- crea la ventana con `InitWindow(...)` (diseño de 2560x1440, reducida si el monitor es más pequeño),
- desactiva ESC como tecla de salida de Raylib (`SetExitKey(KEY_NULL)`), porque el juego la usa para otras cosas,
- arranca la máquina de estados con el menú y ejecuta el bucle principal: aplica los cambios de estado pendientes, procesa la entrada, actualiza y dibuja el frame.

### `src/StateMachine.hpp` / `src/StateMachine.cpp`
Implementa la máquina de estados del juego. Permite añadir un estado, reemplazar el actual, retirarlo y saber si la aplicación debe cerrarse. Los cambios se aplican al principio del frame siguiente, así que un estado puede pedir el cambio desde su propio `handleInput()`.

### `src/GameState.hpp`
Define la interfaz base de un estado del juego. Todos los estados implementan `init()`, `handleInput()`, `update(float deltaTime)`, `render()`, `pause()` y `resume()`.

### `src/MenuState.hpp` / `src/MenuState.cpp`
Pantalla del menú principal:

- permite escribir el nombre del jugador (máximo 15 caracteres), con borrado continuo al mantener retroceso,
- selecciona el color del territorio con las flechas izquierda/derecha,
- con Enter y un nombre válido, empieza la partida,
- con ESC, cierra el juego.

### `src/MainGameState.hpp` / `src/MainGameState.cpp`
Pantalla de la partida. Traduce la entrada del usuario en órdenes para `Game` y lo dibuja todo:

- durante la fase de colocación, el clic izquierdo elige la casilla donde empezará el jugador,
- durante la partida, el clic izquierdo ataca: sobre tierra libre se expande, sobre territorio de otra entidad la ataca,
- el deslizador inferior fija el porcentaje de tropas que se envía en cada ataque,
- clic derecho o ESC cancela el ataque en curso,
- dibuja el mapa, los nombres y tropas sobre cada territorio, las estadísticas y el deslizador,
- al perder muestra la pantalla de Game Over con el tiempo sobrevivido; tras un segundo, cualquier tecla o clic vuelve al menú.

### `src/Game.hpp` / `src/Game.cpp`
Es el núcleo de la lógica del juego. Contiene el mapa, las entidades (el jugador en la posición 0 y los bots después), el tiempo de partida y el estado de fin de partida.

La partida tiene dos fases:

- **Colocación (`SPAWN`)**: los bots se reparten por el mapa con un muestreo de Poisson por rechazo (las bases se separan entre sí). El jugador tiene 13 segundos para elegir su casilla; si no elige, se le coloca en una casilla libre al azar.
- **Juego (`PLAYING`)**: cada segundo crecen las tropas de todas las entidades; el jugador y los bots avanzan sus ataques cada 0,12 segundos.

Otras reglas que implementa:

- **Crecimiento de tropas**: el interés empieza en un 10 % por tick y disminuye a medida que las tropas se acercan al límite de 200 por casilla controlada. Los decimales se acumulan entre ticks para no perderlos.
- **Expansión por frontera**: solo se recorren las casillas del borde del territorio, y cada casilla conquistada gasta tropas del ataque.
- **Resistencia**: conquistar tierra libre cuesta 1 tropa por casilla; conquistar territorio enemigo cuesta las tropas del enemigo divididas entre las del atacante (redondeado, mínimo 1).
- **Ataques mutuos**: si dos entidades se atacan entre sí, sus ataques chocan y se restan.
- **Bots**: usan una paleta de colores que evita los que se parecen al del jugador.
- **Fin de partida**: la partida termina cuando el jugador se queda sin casillas.

### `src/Entity.hpp` / `src/Entity.cpp`
Clase base del jugador y los bots. Guarda nombre, color, tropas, el objetivo actual (`Target`), las tropas comprometidas en el ataque (`attack_fuel`), la resistencia del ataque y la frontera del territorio. `beginAttack` aparta el porcentaje de tropas del ataque y `endAttack` devuelve las que no se hayan gastado.

### `src/Player.hpp` / `src/Player.cpp`
El jugador humano: una `Entity` con un temporizador que marca el ritmo al que avanza su ataque.

### `src/Bot.hpp` / `src/Bot.cpp`
La IA de los bots. Cada bot lanza un ataque cada 6-10 segundos con entre el 30 % y el 70 % de sus tropas. En el 30 % de los casos ataca a un vecino; el resto de las veces se expande sobre tierra libre.

### `src/Target.hpp`
Representa el objetivo de un ataque: ninguno, tierra libre o una entidad enemiga concreta.

### `src/Map.hpp` / `src/Map.cpp` y `src/MapGenerator.hpp` / `src/MapGenerator.cpp`
`Map` guarda la cuadrícula de casillas (200x200). `MapGenerator` genera una imagen de ruido Perlin y `Map::generateMap` la convierte en mapa: los valores por debajo del umbral son agua y el resto tierra. `Map` también puede cargar un mapa desde una imagen (los píxeles negros son agua).

### `src/TerritoryCell.hpp`
Cada casilla del mapa guarda:

- `owner`: la entidad que la controla (`nullptr` si es tierra libre),
- `isWater`: si es agua (no se puede conquistar).

### `vendor/`
Contiene las cabeceras de Raylib y su librería estática, compilada para x86_64 (`vendor/lib/`) y ARM64 (`vendor/lib/arm64/`). Gracias a esto no hace falta instalar Raylib en el sistema para compilar.

## Compilación y ejecución (Linux)

### 1. Compilar

```bash
./build.sh        # compila
./build.sh run    # compila y ejecuta
./build.sh clean  # borra el binario generado
```

En ARM64, usa `./buildArm64.sh` con las mismas opciones.

`./build.sh` equivale a este comando manual:

```bash
g++ -o game src/*.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
```

### 2. Ejecutar

```bash
./game
```

## Cómo se juega

- En el menú, escribe tu nombre, elige color con las flechas y pulsa Enter. ESC cierra el juego.
- Tienes 13 segundos para hacer clic en la casilla donde quieres empezar.
- Ajusta con el deslizador el porcentaje de tropas que envías en cada ataque.
- Haz clic en tierra libre para expandirte o en el territorio de otro jugador para atacarlo.
- Haz clic derecho o pulsa ESC para cancelar el ataque; las tropas no gastadas vuelven a tu territorio.
- Tus tropas crecen cada segundo según el tamaño de tu territorio.
- Si te quedas sin casillas, aparece la pantalla de Game Over; pulsa cualquier tecla para volver al menú.

## Integración continua

`.github/workflows/ci.yml` compila el juego en cada push o pull request a `develop` y `main`, y muestra los avisos del compilador con `-Wall -Wextra` de forma informativa. No ejecuta el juego, porque los runners no tienen pantalla.
