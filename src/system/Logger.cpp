#include "Logger.hpp"

void Logger::info(const char* message){
    spdlog::info(message);
}

void Logger::warn(const char* message){
    spdlog::warn(message);
}

void Logger::debug(const char* message){
    spdlog::debug(message);
}

void Logger::error(const char* message){
    spdlog::error(message);
}

void Logger::setPattern(const char* pattern){
    spdlog::set_pattern(pattern);
}