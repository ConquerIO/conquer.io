#pragma once
#include "spdlog/spdlog.h"

// Singleton para los logs del juego
// ----------------------------------
// Importar Logger.hpp evita poner el include
// en main.cpp y tener include cíclicos
//
// Esta clase no se instancia, se le llama como:
// Logger::error("Ha ocurrido un error");
class Logger {
public:
    static void info(const char* message);
    static void warn(const char* message);
    static void error(const char* message);
    static void debug(const char* message);
    static void setPattern(const char* pattern);
};