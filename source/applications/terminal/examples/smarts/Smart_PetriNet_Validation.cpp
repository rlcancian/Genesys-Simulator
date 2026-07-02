#include "Smart_PetriNet_Validation.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_PetriNet_Validation::Smart_PetriNet_Validation() {
}

int Smart_PetriNet_Validation::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys->getTraceManager());
    PluginManager* plugins = genesys->getPluginManager();
    plugins->autoInsertPlugins();
    Model* model = genesys->getModelManager()->newModel();

    Create* create = plugins->newInstance<Create>(model);
    Dispose* dispose = plugins->newInstance<Dispose>(model);

    ModalModelPetriNet* modal_vazio = new ModalModelPetriNet(model, "Modal_Sem_Nos");
    ModalModelPetriNet* modal_erros = new ModalModelPetriNet(model, "Modal_Com_Erros");

    PetriPlace* p1 = new PetriPlace(model, "Lugar_Origem");
    PetriPlace* p2 = new PetriPlace(model, "Lugar_Destino");

    p1->setInitialNode(true);
    modal_erros->addNode(p1);
    modal_erros->addNode(p2);
    modal_erros->setEntryNode(p1);

    PetriTransition* t_isolada = new PetriTransition(p1, p2, "T_Isolada");
    modal_erros->addTransition(t_isolada);
    PetriTransition* t_erro_arcos = new PetriTransition(p1, p2, "T_Arcos_Invalidos");
    t_erro_arcos->setInputArcWeight(nullptr, "blue", 1); // lugar nulo
    t_erro_arcos->setInputArcWeight(p1, "", 1);          // cor vazia
    t_erro_arcos->setInputArcWeight(p2, "red", 0);       // peso zero
    modal_erros->addTransition(t_erro_arcos);

    create->getConnectionManager()->insert(modal_vazio);
    modal_vazio->getConnectionManager()->insert(modal_erros);
    modal_erros->getConnectionManager()->insert(dispose);

    model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
    model->save("./models/Smart_PetriNet_Validation.gen");

    model->getSimulation()->start();

    delete genesys;
    return 0;
}
