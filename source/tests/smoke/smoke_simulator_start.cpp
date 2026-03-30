#include <memory>

#include "kernel/simulator/Simulator.h"

int main() {
    auto simulator = std::make_unique<Simulator>();

    if (simulator->getPluginManager() == nullptr) {
        return 1;
    }
    if (simulator->getModelManager() == nullptr) {
        return 2;
    }
    if (simulator->getTraceManager() == nullptr) {
        return 3;
    }

    return 0;
}
