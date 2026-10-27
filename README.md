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

Para arquitecturas x86 utilzar:
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

## Desarrollo y contribuciones

El proyecto organiza el trabajo mediante ramas y Pull Requests para revisar los cambios antes de integrarlos en las ramas principales. Los mensajes de commit siguen la convención **Conventional Commits**.

Prefijos habituales:

- `feat:` nueva funcionalidad.
- `fix:` corrección de errores.
- `refactor:` mejora interna sin cambiar el comportamiento esperado.
- `docs:` cambios en documentación.

---

