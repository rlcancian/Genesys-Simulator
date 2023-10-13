#include "Benchmark.h"
#include <thread>
#include <unistd.h>

Benchmark::Benchmark() {};

Benchmark::~Benchmark() {};

int Benchmark::getBenchmarkInfo() {
    return 5;
};

void Benchmark::getBenchmarkInfo(BenchmarkInfo *info) {
    info->_memoria = sysconf(_SC_PHYS_PAGES)*sysconf(_SC_PAGE_SIZE);
    info->_nucleos = std::thread::hardware_concurrency();
};

