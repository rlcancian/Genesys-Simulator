#include "../BaseGenesysTerminalApplication.h"
#include "DistributedExecutionManager.h"
#include "ParallelExecutionManager.h"
#include "Utils.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/Plugin.h"

#include "../../plugins/components/Create.h"
#include "../../plugins/components/Process.h"
#include "../../plugins/components/Decide.h"
#include "../../plugins/components/Dispose.h"
#include "../../plugins/components/Assign.h"
#include "../../plugins/components/Record.h"
#include "../../plugins/data/Variable.h"
#include "../../kernel/simulator/Attribute.h"

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
    void execute(std::string filename);
    void executeServer();
    void executeClient(std::string filename);

    void setIsClient(bool boolean);
    bool getIsClient();

    void log(char* msg, TraceManager::Level level);
    void logError(char* msg);
    void logEvent(char* msg);

    bool readFile(const std::string &filename, std::string* fileOutput);
    void createModelTemp();

    // Transform model to file so it can be sent to servers
    std::string modelToFile(std::string filename);
    void readIPListFromFile(const std::string& filename);
};

#endif