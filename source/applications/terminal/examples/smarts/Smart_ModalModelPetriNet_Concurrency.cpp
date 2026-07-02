#include "Smart_ModalModelPetriNet_Concurrency.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_ModalModelPetriNet_Concurrency::Smart_ModalModelPetriNet_Concurrency() {}

int Smart_ModalModelPetriNet_Concurrency::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys->getTraceManager());
    PluginManager* plugins = genesys->getPluginManager();
    plugins->autoInsertPlugins();
    Model* model = genesys->getModelManager()->newModel();

    Create* create = plugins->newInstance<Create>(model);
    Dispose* dispose = plugins->newInstance<Dispose>(model);

    ModalModelPetriNet* factory = new ModalModelPetriNet(model, "Factory_CPN");
    factory->setMaxTransitionsPerDispatch(10);

    PetriPlace* p_stock = new PetriPlace(model, "Raw_Material_Stock");
    PetriPlace* p_metal_parts = new PetriPlace(model, "Ready_Metal_Parts");
    PetriPlace* p_plastic_parts = new PetriPlace(model, "Ready_Plastic_Parts");
    PetriPlace* p_final_product = new PetriPlace(model, "Final_Products");

    // place for the competing machine
    PetriPlace* p_extra_parts = new PetriPlace(model, "Extra_Metal_Parts");

    p_stock->setInitialNode(true);
    factory->setEntryNode(p_stock);

    // limit the stock to force competition
    p_stock->addTokens(1, "raw_metal");
    p_stock->addTokens(1, "raw_plastic");

    factory->addNode(p_stock);
    factory->addNode(p_metal_parts);
    factory->addNode(p_extra_parts);
    factory->addNode(p_plastic_parts);
    factory->addNode(p_final_product);

    // MACHINE A (priority 1 - expected winner)
    PetriTransition* t_forge = new PetriTransition(p_stock, p_metal_parts, "T_Forge_Metal");
    t_forge->setInputArcWeight(p_stock, "raw_metal", 1);
    t_forge->setOutputArcWeight(p_metal_parts, "metal_part", 1);
    t_forge->setPriority(1);
    factory->addTransition(t_forge);

    // COMPETING MACHINE (priority 2 - expected loser)
    PetriTransition* t_forge_extra = new PetriTransition(p_stock, p_extra_parts, "T_Forge_Extra");
    t_forge_extra->setInputArcWeight(p_stock, "raw_metal", 1);
    t_forge_extra->setOutputArcWeight(p_extra_parts, "extra_metal_part", 1);
    t_forge_extra->setPriority(2);
    factory->addTransition(t_forge_extra);

    // MACHINE B
    PetriTransition* t_mold = new PetriTransition(p_stock, p_plastic_parts, "T_Mold_Plastic");
    t_mold->setInputArcWeight(p_stock, "raw_plastic", 1);
    t_mold->setOutputArcWeight(p_plastic_parts, "plastic_part", 1);
    t_mold->setPriority(1);
    factory->addTransition(t_mold);

    // MACHINE C
    PetriTransition* t_assemble = new PetriTransition(p_metal_parts, p_final_product, "T_Final_Assembly");
    t_assemble->setInputArcWeight(p_metal_parts, "metal_part", 1);
    t_assemble->setInputArcWeight(p_plastic_parts, "plastic_part", 1);
    t_assemble->setOutputArcWeight(p_final_product, "complete_product", 1);
    t_assemble->setPriority(0);
    factory->addTransition(t_assemble);

    create->getConnectionManager()->insert(factory);
    factory->getConnectionManager()->insert(dispose);

    model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
    model->getSimulation()->start();

    delete genesys;
    return 0;
}
