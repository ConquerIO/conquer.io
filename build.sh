#!/usr/bin/env bash
#
# build.sh - Compila Conquer.io.
#
# Uso:
#   ./build.sh          # compila
#   ./build.sh run      # compila y ejecuta el juego
#   ./build.sh clean    # borra el binario generado
#
# Raylib (libreria estatica y cabeceras) esta versionada en vendor/,
# asi que no hace falta instalarla ni construirla para compilar.
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

VENDOR_INC="$ROOT/vendor/include"
VENDOR_LIB="$ROOT/vendor/lib"
RAYLIB_LIB="$VENDOR_LIB/libraylib.a"
GAME_BIN="$ROOT/game"

LINK_FLAGS=(-lraylib -lGL -lm -lpthread -lrt -lX11)

# ---------------------------------------------------------------------------
# Utilidades
# ---------------------------------------------------------------------------
info()  { printf '\033[1;34m[build]\033[0m %s\n' "$*"; }
warn()  { printf '\033[1;33m[aviso]\033[0m %s\n' "$*"; }
error() { printf '\033[1;31m[error]\033[0m %s\n' "$*" >&2; }

# ---------------------------------------------------------------------------
# Acciones
# ---------------------------------------------------------------------------
do_compile() {
    if [[ ! -f "$RAYLIB_LIB" ]]; then
        error "No se encontro $RAYLIB_LIB"
        warn "La libreria esta versionada en el repo; recuperala con:"
        warn "  git checkout -- vendor/lib/libraylib.a"
        exit 1
    fi

    info "Compilando el juego..."
    g++ -o "$GAME_BIN" src/*.cpp \
        -I src/ -I vendor/include/ \
        -L vendor/lib "${LINK_FLAGS[@]}"
    info "Listo: $GAME_BIN"
}

do_run() {
    do_compile
    info "Ejecutando el juego..."
    exec "$GAME_BIN"
}

do_clean() {
    info "Borrando el binario (la libreria vendorizada se conserva)..."
    rm -f "$GAME_BIN"
}

case "${1:-}" in
    ""|build) do_compile ;;
    run)      do_run ;;
    clean)    do_clean ;;
    -h|--help|help)
        awk 'NR==1{next} /^#/{sub(/^# ?/,""); print; next} {exit}' "$0"
        ;;
    *)
        error "Opcion desconocida: $1"
        echo "Usa: ./build.sh [build|run|clean]"
        exit 1
        ;;
esac
