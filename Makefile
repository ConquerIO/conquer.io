# Makefile - Compila Conquer.io para x86-64 o ARM64.
#
# Uso:
#   make                # all: compila (por defecto)
#   make run            # compila y ejecuta
#   make clean          # borra objetos y binario
#   make ARCH=arm64     # fuerza la arquitectura (x86 | arm64)
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
.PHONY: all clean run clean-ccache-stats clean-ccache ccache-stats
.DEFAULT_GOAL := all

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[link] $@ ($(ARCH))"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: %.cpp
	@echo "[cxx]  $<"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(DEFS) $(INCS) -MMD -MP -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf build $(TARGET)

# (-z) Borra las estadísticas de ccache; la caché se conserva
clean-ccache-stats:
	@$(if $(CCACHE),ccache --zero-stats,echo "ccache no está instalado")

# (-C) Borra la caché de ccache; las estadísticas se conservan
clean-ccache:
	@$(if $(CCACHE),ccache --clear,echo "ccache no está instalado")
	        
# Muestra las estadísticas de ccache
ccache-stats:
	@$(if $(CCACHE),ccache -s,echo "ccache no está instalado")
	

-include $(DEPS)
