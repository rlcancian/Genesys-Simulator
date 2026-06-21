#include "Smart_ModalModelPetriNet_Persistence.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_ModalModelPetriNet_Persistence::Smart_ModalModelPetriNet_Persistence() {}

int Smart_ModalModelPetriNet_Persistence::main(int argc, char** argv) {
    Simulator* genesys_save = new Simulator();
    genesys_save->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys_save->getTraceManager());
    PluginManager* plugins_save = genesys_save->getPluginManager();
    plugins_save->autoInsertPlugins();

    Model* model_save = genesys_save->getModelManager()->newModel();
    Create* create = plugins_save->newInstance<Create>(model_save);
    Dispose* dispose = plugins_save->newInstance<Dispose>(model_save);

    ModalModelPetriNet* modal = new ModalModelPetriNet(model_save, "Persistence_Network");
    PetriPlace* p_input = new PetriPlace(model_save, "Persisted_Input");
    PetriPlace* p_output = new PetriPlace(model_save, "Persisted_Output");

    p_input->setInitialNode(true);
    p_input->addTokens(5, "cyan");

    modal->addNode(p_input);
    modal->addNode(p_output);
    modal->setEntryNode(p_input);

    PetriTransition* t_move = new PetriTransition(p_input, p_output, "T_Move_Tokens");
    t_move->setInputArcWeight(p_input, "cyan", 1);
    t_move->setOutputArcWeight(p_output, "dark_cyan", 1);
    modal->addTransition(t_move);

    create->getConnectionManager()->insert(modal);
    modal->getConnectionManager()->insert(dispose);

    model_save->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);

    // saves and destroys the entire environment from memory
    std::string filename = "./models/Smart_PetriNet_Persistence.gen";
    model_save->save(filename);
    delete genesys_save;

    // reloading and execution
    Simulator* genesys_load = new Simulator();
    genesys_load->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys_load->getTraceManager());
    genesys_load->getPluginManager()->autoInsertPlugins();

    Model* model_load = genesys_load->getModelManager()->newModel();

    // reloads from the file, if it fails, the model remains empty and does not execute
    if(model_load->load(filename)) {
        genesys_load->getTraceManager()->trace(TraceManager::Level::L1_errorFatal, "Model reloaded successfully! Starting persistence simulation...");
        model_load->getSimulation()->start();
    } else {
        genesys_load->getTraceManager()->trace(TraceManager::Level::L1_errorFatal, "CRITICAL ERROR: Failed to reload the .gen file.");
    }

    delete genesys_load;
    return 0;
}
