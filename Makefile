# Makefile - Compila Conquer.io para x86-64 o ARM64.
#
# Uso:
#   make                # all: compila (por defecto)
#   make run            # compila y ejecuta
#   make clean          # borra objetos y binario
#   make ARCH=arm64     # fuerza la arquitectura (x86 | arm64)
#   make -j$(nproc)     # compila en paralelo
#   make web            # versión web con Emscripten (ver sección Web)
#   make web-serve      # compila la versión web y la sirve en localhost
#   make web-chrome     # igual, abriendo Chrome con la GPU habilitada
#   make ccache-stats | clean-ccache-stats | clean-ccache   # gestión de ccache
#
# La arquitectura se detecta con uname -m. 
# Genérico: todos los .cpp bajo src/ se detectan solos. 
# Para añadir una librería basta con ampliar LIBS.

# ---- Configuración editable -------------------------------------------------
TARGET   := game

# Comprobar si ccache está disponible en el sistema
CCACHE := $(shell which ccache 2>/dev/null)
# Si ccache existe, se usa como prefijo; si no, queda vacío
CXX      := $(CCACHE) g++
CXXFLAGS := -std=c++17 -O3 -Wall
DEFS     := -DNO_FONT_AWESOME
LIBS     := -lraylib -lGL -lm -lpthread -lrt -lX11

# ---- Arquitectura -----------------------------------------------------------
HOST_ARCH := $(shell uname -m)
ARCH ?= $(if $(filter aarch64 arm64,$(HOST_ARCH)),arm64,x86)

ifeq ($(ARCH),x86)
  LIBDIRS := vendor/lib
else ifeq ($(ARCH),arm64)
  LIBDIRS := vendor/lib/arm64
else
  $(error ARCH='$(ARCH)' no válida: usa x86 o arm64)
endif

# ---- Detección automática ---------------------------------------------------
SRC_DIR   := src
BUILD_DIR := build/$(ARCH)

# Fuentes del juego (cualquier subcarpeta de src/) y de terceros compilados.
SRCS  := $(shell find $(SRC_DIR) -name '*.cpp') \
         $(wildcard vendor/imgui/*.cpp) $(wildcard vendor/rlImGui/*.cpp)
OBJS  := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS  := $(OBJS:.o=.d)

# Includes: src/ y cada carpeta de vendor/.
INCS    := -I $(SRC_DIR) $(addprefix -I ,$(patsubst %/,%,$(wildcard vendor/*/)))
LDFLAGS := $(addprefix -L ,$(LIBDIRS))

# ---- Reglas -----------------------------------------------------------------
.PHONY: check-emsdk web web-serve web-chrome all clean run clean-ccache-stats clean-ccache ccache-stats
.DEFAULT_GOAL := all

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[link] $@ ($(ARCH))"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: %.cpp
	@echo "[cxx]  $<"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(DEFS) $(INCS) -MMD -MP -c $< -o $@

# ---- Web (Emscripten) -------------------------------------------------------
# Requisitos: emsdk instalado y activo (source /ruta/a/emsdk/emsdk_env.sh) y raylib compilada para web en
# vendor/lib/web/libraylib.a (en raylib/src: make PLATFORM=PLATFORM_WEB).
#   make web         # genera build-web/index.html
#   make web-serve   # lo sirve en http://localhost:8080
#   make web-chrome  # lo sirve y abre Chrome con GPU (ver abajo); al cerrarlo para el servidor
WEB_PORT   ?= 8080
CHROME     ?= $(shell command -v google-chrome || command -v chromium || command -v chromium-browser)
CHROME_PROFILE := build/chrome-profile
WEB_DIR    := build-web
WEB_BUILD  := build/web
WEB_OBJS   := $(SRCS:%.cpp=$(WEB_BUILD)/%.o)
WEB_CXXFLAGS := -std=c++17 -O2 -Wall -DPLATFORM_WEB
WEB_LDFLAGS  := vendor/lib/web/libraylib.a -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 \
                -sSTACK_SIZE=1048576 -sASYNCIFY \
                --preload-file assets/fonts@/assets/fonts \
                --preload-file assets/images@/assets/images \
                --preload-file assets/sounds@/assets/sounds \
                --preload-file maps@/maps \
                --shell-file web/shell.html

web: $(WEB_DIR)/index.html

# Comprueba que Emscripten está disponible antes de compilar la versión web
check-emsdk:
	@command -v em++ >/dev/null 2>&1 || { \
	    echo "No se encuentra em++: instala emsdk y actívalo con 'source /ruta/a/emsdk/emsdk_env.sh'"; \
	    echo "Instrucciones: https://emscripten.org/docs/getting_started/downloads.html"; \
	    exit 1; }

$(WEB_DIR)/index.html: check-emsdk $(WEB_OBJS) web/shell.html
	@echo "[link] $@ (web)"
	@mkdir -p $(WEB_DIR)
	@em++ -o $@ $(WEB_OBJS) $(WEB_LDFLAGS)

$(WEB_BUILD)/%.o: %.cpp
	@echo "[em++] $<"
	@mkdir -p $(dir $@)
	@em++ $(WEB_CXXFLAGS) $(DEFS) $(INCS) -MMD -MP -c $< -o $@

web-serve: web
	python3 -m http.server $(WEB_PORT) -d $(WEB_DIR)

# Chrome bloquea la GPU de algunas VM (WebGL blocklisted): --ignore-gpu-blocklist la
# habilita. El perfil propio hace que sea una instancia independiente de tu Chrome y
# respete las opciones aunque ya tengas Chrome abierto.
web-chrome: web
	@if [ -z "$(CHROME)" ]; then echo "No se encuentra Chrome/Chromium (usa CHROME=/ruta/al/navegador)"; exit 1; fi
	@if ss -ltn 2>/dev/null | grep -q ":$(WEB_PORT) "; then \
	    echo "El puerto $(WEB_PORT) está ocupado: usa make web-chrome WEB_PORT=8081"; exit 1; fi
	@mkdir -p $(CHROME_PROFILE)
	@python3 -m http.server $(WEB_PORT) -d $(WEB_DIR) >/dev/null 2>&1 & \
	SERVER_PID=$$!; trap 'kill $$SERVER_PID 2>/dev/null' EXIT INT TERM; sleep 1; \
	echo "[chrome] http://localhost:$(WEB_PORT)/ (cierra la ventana para parar el servidor)"; \
	$(CHROME) --user-data-dir=$(CHROME_PROFILE) --no-first-run --no-default-browser-check \
	    --ignore-gpu-blocklist http://localhost:$(WEB_PORT)/

run: all
	./$(TARGET)

clean:
	rm -rf build build-web $(TARGET)

# (-z) Borra las estadísticas de ccache; la caché se conserva
clean-ccache-stats:
	@$(if $(CCACHE),ccache --zero-stats,echo "ccache no está instalado")

# (-C) Borra la caché de ccache; las estadísticas se conservan
clean-ccache:
	@$(if $(CCACHE),ccache --clear,echo "ccache no está instalado")
	        
# Muestra las estadísticas de ccache
ccache-stats:
	@$(if $(CCACHE),ccache -s,echo "ccache no está instalado")
	

-include $(DEPS) $(WEB_OBJS:.o=.d)
