#include <SystemStats.hpp>

#include <thread>

#ifdef __linux__
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <string>
#endif

namespace
{
#ifdef __linux__
constexpr std::size_t BYTES_PER_KB = 1024;

struct ProcStatSample
{
    std::size_t utime = 0;   
    std::size_t stime = 0;
    unsigned long minflt = 0;
    unsigned long majflt = 0;
};

bool readProcStat(ProcStatSample& out)
{
    std::ifstream file("/proc/self/stat");
    if (!file.is_open()){
        return false;
    }

    std::string line;
    if (!std::getline(file, line)){
        return false;
    }

    const std::size_t close = line.rfind(')');
    if (close == std::string::npos){
        return false;
    }

    std::istringstream fields(line.substr(close + 1));

    std::string state;
    long ppid = 0;
    long pgrp = 0;
    long session = 0;
    long tty = 0;
    long tpgid = 0;
    unsigned long flags = 0;
    unsigned long minflt = 0;
    unsigned long cminflt = 0;
    unsigned long majflt = 0;
    unsigned long cmajflt = 0;
    unsigned long utime = 0;
    unsigned long stime = 0;

    fields >> state >> ppid >> pgrp >> session >> tty >> tpgid >> flags
           >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime;

    if (!fields){
        return false;
    }

    out.utime = static_cast<std::size_t>(utime);
    out.stime = static_cast<std::size_t>(stime);
    out.minflt = minflt;
    out.majflt = majflt;
    return true;
}

//se encarga de leer en formato clave:valor
bool readKeyedValue(const char* path, const std::string& key, unsigned long& value)
{
    std::ifstream file(path);
    if (!file.is_open()){
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.compare(0, key.size(), key) != 0){
            continue;
        }

        std::istringstream fields(line.substr(key.size()));
        unsigned long parsed = 0;
        if (fields >> parsed)
        {
            value = parsed;
            return true;
        }
    }
    return false;
}

//lee los contadores globales
bool readSystemCpu(unsigned long long& busy, unsigned long long& total)
{
    std::ifstream file("/proc/stat");
    if (!file.is_open()){
        return false;
    }

    std::string label;
    unsigned long long user = 0;
    unsigned long long nice = 0;
    unsigned long long system = 0;
    unsigned long long idle = 0;
    unsigned long long iowait = 0;
    unsigned long long irq = 0;
    unsigned long long softirq = 0;
    unsigned long long steal = 0;

    file >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    if (label != "cpu" || !file){
        return false;
    }

    const unsigned long long idle_all = idle + iowait;
    total = user + nice + system + idle_all + irq + softirq + steal;
    busy = total - idle_all;
    return total > 0;
}
#endif
}

int SystemStats::cpuCoreCount()
{
    const unsigned int cores = std::thread::hardware_concurrency();
    return cores == 0 ? 1 : static_cast<int>(cores);
}

void SystemStats::update(float deltaTime)
{
    timer += deltaTime;

    if (has_baselines && timer < SAMPLE_INTERVAL){
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    double elapsed = 0.0;
    if (has_baselines)
    {
        elapsed = std::chrono::duration<double>(now - last_sample).count();
        if (elapsed < 0.001) {
            elapsed = 0.001;
        }
    }

    last_sample = now;
    timer = 0.0f;

    sample(elapsed);
}

void SystemStats::readProcessStatus()
{
#ifdef __linux__
    unsigned long value = 0;

    if (readKeyedValue("/proc/self/status", "VmRSS:", value)){
        process_rss = static_cast<std::size_t>(value) * BYTES_PER_KB;
    }

    if (readKeyedValue("/proc/self/status", "VmHWM:", value)){
        process_peak_rss = static_cast<std::size_t>(value) * BYTES_PER_KB;
    }

    if (readKeyedValue("/proc/self/status", "Threads:", value)){
        threads = static_cast<int>(value);
    }

#endif
}

void SystemStats::readSystemMemory()
{
#ifdef __linux__
    unsigned long value = 0;

    if (readKeyedValue("/proc/meminfo", "MemTotal:", value)){
        system_total = static_cast<std::size_t>(value) * BYTES_PER_KB;

    }

    if (readKeyedValue("/proc/meminfo", "MemAvailable:", value)){
        system_available = static_cast<std::size_t>(value) * BYTES_PER_KB;
    }

#endif
}

void SystemStats::sample(double elapsedSeconds)
{
#ifdef __linux__
    ProcStatSample proc{};
    const bool proc_ok = readProcStat(proc);
    if (proc_ok)
    {
        minor_faults = proc.minflt;
        major_faults = proc.majflt;
    }

    unsigned long long busy = 0;
    unsigned long long total = 0;
    const bool sys_cpu_ok = readSystemCpu(busy, total);

    readProcessStatus();
    readSystemMemory();

    if (!has_baselines)
    {
        if (proc_ok)
        {
            prev_utime = proc.utime;
            prev_stime = proc.stime;
        }
        if (sys_cpu_ok)
        {
            prev_busy = busy;
            prev_total = total;
        }
        has_baselines = true;
        return;
    }

    if (proc_ok && elapsedSeconds > 0.0)
    {
        const long clock_ticks = sysconf(_SC_CLK_TCK);
        const double ticks_per_second = clock_ticks > 0 ? static_cast<double>(clock_ticks) : 100.0;

        const double current_ticks = static_cast<double>(proc.utime + proc.stime);
        const double previous_ticks = static_cast<double>(prev_utime + prev_stime);
        const double cpu_seconds = (current_ticks - previous_ticks) / ticks_per_second;
        const double percent = (cpu_seconds / elapsedSeconds) * 100.0;

        process_cpu = has_process_cpu
            ? process_cpu + (static_cast<float>(percent) - process_cpu) * SMOOTHING
            : static_cast<float>(percent);
        has_process_cpu = true;

        prev_utime = proc.utime;
        prev_stime = proc.stime;
    }

    if (sys_cpu_ok && total > prev_total)
    {
        const double delta_busy = static_cast<double>(busy - prev_busy);
        const double delta_total = static_cast<double>(total - prev_total);
        const double percent = delta_total > 0.0 ? (delta_busy / delta_total) * 100.0 : 0.0;

        system_cpu = has_system_cpu
            ? system_cpu + (static_cast<float>(percent) - system_cpu) * SMOOTHING
            : static_cast<float>(percent);
        has_system_cpu = true;

        prev_busy = busy;
        prev_total = total;
    }
#else
    (void)elapsedSeconds;
#endif
}
