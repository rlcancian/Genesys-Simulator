#include <iostream>

#include "core/DistributedLibraryInfo.h"

// Entry point for the standalone distributed orchestrator (master). This is a build
// skeleton: it only confirms the reusable library links. Discovery, scheduling, remote
// execution, aggregation and result output are added in later tasks.
int main() {
    std::cout << genesys::distributed::distributedLayerName() << std::endl;
    return 0;
}
