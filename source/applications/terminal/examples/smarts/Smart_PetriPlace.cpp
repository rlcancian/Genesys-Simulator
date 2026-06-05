#include "Smart_PetriPlace.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_PetriPlace::Smart_PetriPlace() {
}

int Smart_PetriPlace::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys->getTraceManager());
    PluginManager* plugins = genesys->getPluginManager();
    plugins->autoInsertPlugins();
    Model* model = genesys->getModelManager()->newModel();

    Create* create = plugins->newInstance<Create>(model);
    Dispose* dispose = plugins->newInstance<Dispose>(model);


    ModalModelPetriNet* modal_vazio = new ModalModelPetriNet(model, "Modal_Sem_Nos");

    ModalModelPetriNet* modal = new ModalModelPetriNet(model, "SubModeloPetriColorida");

    PetriPlace* p1_azul = new PetriPlace(model, "Lugar1_Azul");
    PetriPlace* p2_vermelho = new PetriPlace(model, "Lugar2_Vermelho");
    PetriPlace* p3_roxo = new PetriPlace(model, "Lugar3_Roxo");

    p1_azul->setInitialNode(true);
    p1_azul->addTokens(2, "blue");

    modal->addNode(p1_azul);
    modal->addNode(p2_vermelho);
    modal->addNode(p3_roxo);
    modal->setEntryNode(p1_azul);

    PetriTransition* t_isolada = new PetriTransition(p1_azul, p3_roxo, "Trans_Isolada");
    modal->addTransition(t_isolada);

    PetriTransition* t_erro_in = new PetriTransition(p1_azul, p3_roxo, "Trans_Erro_Entrada");
    t_erro_in->setInputArcWeight(nullptr, "blue", 1);
    t_erro_in->setInputArcWeight(p1_azul, "", 1);
    t_erro_in->setInputArcWeight(p2_vermelho, "red", 0);
    modal->addTransition(t_erro_in);

    PetriTransition* t_erro_out = new PetriTransition(p1_azul, p3_roxo, "Trans_Erro_Saida");
    t_erro_out->setOutputArcWeight(nullptr, "purple", 1);
    t_erro_out->setOutputArcWeight(p3_roxo, "", 1);
    t_erro_out->setOutputArcWeight(p3_roxo, "purple", 0);
    modal->addTransition(t_erro_out);

    create->getConnectionManager()->insert(modal);
    modal->getConnectionManager()->insert(dispose);

    model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
    model->save("./models/Smart_PetriPlace.gen");

    model->getSimulation()->start();

    delete genesys;
    return 0;
}
