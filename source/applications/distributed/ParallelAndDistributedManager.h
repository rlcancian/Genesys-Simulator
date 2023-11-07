#include "DistributedExecutionManager.h"
#include "ParallelExecutionManager.h"
#include "Utils.h"

#ifndef PARALLEL_DISTRIBUTED_MANAGER_H
#define PARALLEL_DISTRIBUTED_MANAGER_H

// TODO

// Desfazer depedência do DEM com o modelo para que classe pai (esta) mande por parâmetro
// as informações sobre o modelo

// Testar future e se não funcionar trocar para pthread ou outro tipo (boost)

// Criar interação com terminal para executar modelo

class ParallelAndDistributedManager
{
private:
    ParallelExecutionManager* _parallelExecutionManager;
    DistributedExecutionManager* _distributedExecutionManager;
    Model* _model;
    bool isClient;
public:
    ParallelAndDistributedManager(Model* model);
    ~ParallelAndDistributedManager();

    // Start parallel and distributed execution, depending whether it its a client or a server
    void execute();
    void executeServer();
    void executeClient();

    // Transform model to file so it can be sent to servers
    std::string modelToFile(std::string filename);
};

ParallelAndDistributedManager::ParallelAndDistributedManager(Model* model)
{
    // Desfazer depedência do DEM com o modelo para que classe pai (esta) mande por parâmetro
    // as informações sobre o modelo
    this->_model = model;
    this->_parallelExecutionManager = new ParallelExecutionManager();
    this->_distributedExecutionManager = new DistributedExecutionManager(model);

}

ParallelAndDistributedManager::~ParallelAndDistributedManager()
{
    delete this->_parallelExecutionManager;
    delete this->_distributedExecutionManager;
}

#endif