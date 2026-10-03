# conquer.io

## Compilación y Ejecución (Linux)

Este proyecto utiliza Raylib. Las librerías estáticas y cabeceras ya están incluidas en la carpeta `vendor/`, por lo que no es necesario instalar Raylib en el sistema para compilar.

**1. Compilar el juego:**
Abre una terminal en la raíz del proyecto y ejecuta el siguiente comando:

```bash
g++ -o game src/*.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
```

**2. Ejecutar el juego:**
Una vez compilado, puedes ejecutar el juego con el siguiente comando:

```bash
./game
```

## Partida local

Introduce un nombre y elige un color en el menú. En la partida, haz clic en
una casilla neutral o enemiga para ordenar el avance automático hasta ella.
Haz clic derecho o pulsa `ESC` para cancelar la orden. Las tropas crecen con
el tiempo; captura la base del bot para ganar. Pulsa `R` al terminar para
empezar otra partida.