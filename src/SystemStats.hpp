#pragma once

#include <chrono>
#include <cstddef>

//de momento solo sirve para Linux, ya luego se estandariza
class SystemStats
{
public:
    void update(float deltaTime);
    //si hay metricas disponible a evaluar 
    bool available() const { 
        return supported; 
    }

    //en ubuntu los datos estan en /proc , estos metodos leen esa carpeta y sacan las metricas
    float processCpuPercent() const { return process_cpu; }
    float systemCpuPercent() const { return system_cpu; }
    std::size_t processRssBytes() const { return process_rss; }
    std::size_t processPeakRssBytes() const { return process_peak_rss; }
    std::size_t systemTotalBytes() const { return system_total; }
    std::size_t systemAvailableBytes() const { return system_available; }
    int threadCount() const { return threads; }
    unsigned long minorFaults() const { return minor_faults; }
    unsigned long majorFaults() const { return major_faults; }

    static int cpuCoreCount();

private:
    static constexpr float SAMPLE_INTERVAL = 0.5f;
    // Peso de la muestra nueva en la media exponencial.
    static constexpr float SMOOTHING = 0.30f;

    void sample(double elapsedSeconds);
    void readProcessStatus();
    void readSystemMemory();

#ifdef __linux__
    bool supported = true;
#else
    bool supported = false;
#endif

    // Hay una muestra anterior valida para calcular incrementos de CPU.
    bool has_baselines = false;

    float timer = 0.0f;
    std::chrono::steady_clock::time_point last_sample{};

    // Baselines de la muestra anterior.
    std::size_t prev_utime = 0;
    std::size_t prev_stime = 0;
    unsigned long long prev_busy = 0;
    unsigned long long prev_total = 0;

    bool has_process_cpu = false;
    bool has_system_cpu = false;

    float process_cpu = 0.0f;
    float system_cpu = 0.0f;

    std::size_t process_rss = 0;
    std::size_t process_peak_rss = 0;
    std::size_t system_total = 0;
    std::size_t system_available = 0;

    int threads = 0;
    unsigned long minor_faults = 0;
    unsigned long major_faults = 0;
};
