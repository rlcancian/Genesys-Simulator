
#include "Benchmark.h"
#include <thread>


void Benchmark::getMachineInfo(Machine_Info *info) {
    info->_benchmark_1 = 0;
    info->_memoria = 0;
    info->_nucleos = std::thread::hardware_concurrency();
};

void Benchmark::getBenchmark1(Machine_Info *info) {
    //@TODO
    return;
}