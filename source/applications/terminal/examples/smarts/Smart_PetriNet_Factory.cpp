#include "Smart_PetriNet_Factory.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_PetriNet_Factory::Smart_PetriNet_Factory() {
}

int Smart_PetriNet_Factory::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys->getTraceManager());
    PluginManager* plugins = genesys->getPluginManager();
    plugins->autoInsertPlugins();
    Model* model = genesys->getModelManager()->newModel();

    Create* create = plugins->newInstance<Create>(model);
    Dispose* dispose = plugins->newInstance<Dispose>(model);

    ModalModelPetriNet* fabrica = new ModalModelPetriNet(model, "Fabrica_CPN");

    fabrica->setMaxTransitionsPerDispatch(10); // aumento do limite de transições

    PetriPlace* p_estoque = new PetriPlace(model, "Estoque_Materias_Primas");
    PetriPlace* p_pecas_metal = new PetriPlace(model, "Pecas_Metalicas_Prontas");
    PetriPlace* p_pecas_plastico = new PetriPlace(model, "Pecas_Plasticas_Prontas");
    PetriPlace* p_produto_final = new PetriPlace(model, "Produtos_Finalizados");

    p_estoque->setInitialNode(true);
    fabrica->setEntryNode(p_estoque);

    p_estoque->addTokens(2, "metal_bruto");
    p_estoque->addTokens(2, "plastico_bruto");

    fabrica->addNode(p_estoque);
    fabrica->addNode(p_pecas_metal);
    fabrica->addNode(p_pecas_plastico);
    fabrica->addNode(p_produto_final);

    // maquina A: forja o metal
    PetriTransition* t_forjar = new PetriTransition(p_estoque, p_pecas_metal, "T_Forjar_Metal");
    t_forjar->setInputArcWeight(p_estoque, "metal_bruto", 1);
    t_forjar->setOutputArcWeight(p_pecas_metal, "peca_metal", 1);
    t_forjar->setPriority(1);
    fabrica->addTransition(t_forjar);

    // maquina B: molda o plastico
    PetriTransition* t_moldar = new PetriTransition(p_estoque, p_pecas_plastico, "T_Moldar_Plastico");
    t_moldar->setInputArcWeight(p_estoque, "plastico_bruto", 1);
    t_moldar->setOutputArcWeight(p_pecas_plastico, "peca_plastico", 1);
    t_moldar->setPriority(1);
    fabrica->addTransition(t_moldar);

    // maquina C: montagem final (exige duas origens diferentes)
    PetriTransition* t_montar = new PetriTransition(p_pecas_metal, p_produto_final, "T_Montagem_Final");
    t_montar->setInputArcWeight(p_pecas_metal, "peca_metal", 1);
    t_montar->setInputArcWeight(p_pecas_plastico, "peca_plastico", 1);
    t_montar->setOutputArcWeight(p_produto_final, "produto_completo", 1);

    t_montar->setPriority(0); // define t_montar como prioritária
    fabrica->addTransition(t_montar);

    create->getConnectionManager()->insert(fabrica);
    fabrica->getConnectionManager()->insert(dispose);

    model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
    model->save("./models/Smart_PetriNet_Factory.gen");
    model->getSimulation()->start();

    delete genesys;
    return 0;
}
