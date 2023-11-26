#include "../BaseGenesysTerminalApplication.h"
#include "DistributedExecutionManager.h"
#include "ParallelExecutionManager.h"
#include "Utils.h"
#include "../../kernel/simulator/Simulator.h"

#ifndef PARALLEL_DISTRIBUTED_MANAGER_H
#define PARALLEL_DISTRIBUTED_MANAGER_H

// TODO

// Desfazer depedência do DEM com o modelo para que classe pai (esta) mande por parâmetro
// as informações sobre o modelo

// Testar future e se não funcionar trocar para pthread ou outro tipo (boost)

// Criar interação com terminal para executar modelo

class ParallelAndDistributedManager: public BaseGenesysTerminalApplication
{
private:
    ParallelExecutionManager* _parallelExecutionManager;
    DistributedExecutionManager* _distributedExecutionManager;
    Simulator* _simulator;
    Model* _model;
    bool _isClient;
public:
    ParallelAndDistributedManager();
    ~ParallelAndDistributedManager();

    int main(int argc, char** argv);

    // Start parallel and distributed execution, depending whether it its a client or a server
    void execute(Model* model, std::string filename);
    void executeServer();
    void findServers();
    void executeClient(std::string filename);

    void setIsClient(bool boolean);
    bool getIsClient();

    void log(char* msg, TraceManager::Level level);
    void logError(char* msg);
    void logEvent(char* msg);

    std::string readFile(const std::string &filename);

    // Transform model to file so it can be sent to servers
    std::string modelToFile(std::string filename);
};

#endif