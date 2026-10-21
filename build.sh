#!/usr/bin/env bash
#
# build.sh - Compila Conquer.io.
#
# Uso:
#   ./build.sh          # compila
#   ./build.sh run      # compila y ejecuta el juego
#   ./build.sh clean    # borra el binario generado

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

VENDOR_INC="$ROOT/vendor/include"
VENDOR_LIB="$ROOT/vendor/lib"
RAYLIB_LIB="$VENDOR_LIB/libraylib.a"
GAME_BIN="$ROOT/game"

LINK_FLAGS=(-lraylib -lGL -lm -lpthread -lrt -lX11)

IMGUI_SOURCES=(
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/rlImGui/rlImGui.cpp
)
IMGUI_FLAGS=(-I vendor/imgui -I vendor/rlImGui -DNO_FONT_AWESOME)

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
    g++ -o "$GAME_BIN" src/*.cpp src/system/*.cpp "${IMGUI_SOURCES[@]}" \
        -I src/ -I vendor/include/ "${IMGUI_FLAGS[@]}" \
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
