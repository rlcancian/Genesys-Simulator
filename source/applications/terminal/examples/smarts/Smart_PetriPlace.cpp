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
    ModalModelPetriNet* modal = new ModalModelPetriNet(model, "SubModeloPetriColorida");
    Dispose* dispose = plugins->newInstance<Dispose>(model);

    PetriPlace* p1_azul = new PetriPlace(model, "Lugar1_Azul");
    PetriPlace* p2_vermelho = new PetriPlace(model, "Lugar2_Vermelho");
    PetriPlace* p3_roxo = new PetriPlace(model, "Lugar3_Roxo");

    p1_azul->setInitialNode(true);
    p2_vermelho->setInitialNode(true);

    p1_azul->addTokens(2, "blue");
    p2_vermelho->addTokens(1, "red");

    modal->addNode(p1_azul);
    modal->addNode(p2_vermelho);
    modal->addNode(p3_roxo);

    modal->setEntryNode(p1_azul);

    PetriTransition* t1_mistura = new PetriTransition(p1_azul, p3_roxo, "TransicaoMistura");

    t1_mistura->setInputArcWeight(p1_azul, "blue", 1);
    t1_mistura->setInputArcWeight(p2_vermelho, "red", 1);

    t1_mistura->setOutputArcWeight(p3_roxo, "purple", 1);

    modal->addTransition(t1_mistura);

    create->getConnectionManager()->insert(modal);
    modal->getConnectionManager()->insert(dispose);

    model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
    model->save("./models/Smart_PetriPlace.gen");

    // a expectativa é que, ao simular, T1 dispare uma vez.
    // P1 terminará com 1 "blue".
    // P2 terminará com 0 "red".
    // P3 terminará com 1 "purple".
    model->getSimulation()->start();

    delete genesys;
    return 0;
}
