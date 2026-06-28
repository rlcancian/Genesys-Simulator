#include "Smart_ModalModelPetriNet_ColorsBranching.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_ModalModelPetriNet_ColorsBranching::Smart_ModalModelPetriNet_ColorsBranching() {}

int Smart_ModalModelPetriNet_ColorsBranching::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys->getTraceManager());
    PluginManager* plugins = genesys->getPluginManager();
    plugins->autoInsertPlugins();
    Model* model = genesys->getModelManager()->newModel();

    Create* create = plugins->newInstance<Create>(model);
    ModalModelPetriNet* modal = new ModalModelPetriNet(model, "ColoredPetriSubModel");
    Dispose* dispose = plugins->newInstance<Dispose>(model);

    // declaring places
    PetriPlace* p1_initial_mix = new PetriPlace(model, "Place1_InitialMix");
    PetriPlace* p2_red_supply = new PetriPlace(model, "Place2_RedSupply");
    PetriPlace* p3_purple_stage = new PetriPlace(model, "Place3_PurpleStage");
    PetriPlace* p4_green_stage = new PetriPlace(model, "Place4_GreenStage");
    PetriPlace* p5_dark_purple = new PetriPlace(model, "Place5_DarkPurple");
    PetriPlace* p6_waste = new PetriPlace(model, "Place6_Waste");
    PetriPlace* p7_cyan_stage = new PetriPlace(model, "Place7_CyanStage");
    PetriPlace* p8_final_product = new PetriPlace(model, "Place8_FinalProduct");

    p1_initial_mix->setInitialNode(true);
    p2_red_supply->setInitialNode(true);

    // adding tokens
    // 4 blues so 2 can mix with red and 1 can be used later by the green branch
    p1_initial_mix->addTokens(4, "blue");
    p1_initial_mix->addTokens(2, "green");
    p2_red_supply->addTokens(2, "red");

    modal->addNode(p1_initial_mix);
    modal->addNode(p2_red_supply);
    modal->addNode(p3_purple_stage);
    modal->addNode(p4_green_stage);
    modal->addNode(p5_dark_purple);
    modal->addNode(p6_waste);
    modal->addNode(p7_cyan_stage);
    modal->addNode(p8_final_product);
    modal->setEntryNode(p1_initial_mix);

    // transition 1: mix blue + red -> purple
    PetriTransition* t1_mix = new PetriTransition(p1_initial_mix, p3_purple_stage, "T1_MixColors");
    t1_mix->setInputArcWeight(p1_initial_mix, "blue", 1);
    t1_mix->setInputArcWeight(p2_red_supply, "red", 1);
    t1_mix->setOutputArcWeight(p3_purple_stage, "purple", 1);
    modal->addTransition(t1_mix);

    // transition 2: separate green -> light green
    PetriTransition* t2_separate_green = new PetriTransition(p1_initial_mix, p4_green_stage, "T2_SeparateGreen");
    t2_separate_green->setInputArcWeight(p1_initial_mix, "green", 1);
    t2_separate_green->setOutputArcWeight(p4_green_stage, "light_green", 1);
    modal->addTransition(t2_separate_green);

    // transition 3: refine purple -> dark purple + waste (multiple outputs)
    PetriTransition* t3_refine_purple = new PetriTransition(p3_purple_stage, p5_dark_purple, "T3_RefinePurple");
    t3_refine_purple->setInputArcWeight(p3_purple_stage, "purple", 1);
    t3_refine_purple->setOutputArcWeight(p5_dark_purple, "dark_purple", 1);
    t3_refine_purple->setOutputArcWeight(p6_waste, "waste", 1); // secondary output
    modal->addTransition(t3_refine_purple);

    // transition 4: process green with leftover blue -> cyan
    PetriTransition* t4_process_green = new PetriTransition(p4_green_stage, p7_cyan_stage, "T4_ProcessGreen");
    t4_process_green->setInputArcWeight(p4_green_stage, "light_green", 1);
    t4_process_green->setInputArcWeight(p1_initial_mix, "blue", 1); // Re-uses the initial place
    t4_process_green->setOutputArcWeight(p7_cyan_stage, "cyan", 1);
    modal->addTransition(t4_process_green);

    // transition 5: final assembly merging branches -> rainbow product
    PetriTransition* t5_final_assembly = new PetriTransition(p5_dark_purple, p8_final_product, "T5_FinalAssembly");
    t5_final_assembly->setInputArcWeight(p5_dark_purple, "dark_purple", 1);
    t5_final_assembly->setInputArcWeight(p7_cyan_stage, "cyan", 1);
    t5_final_assembly->setOutputArcWeight(p8_final_product, "rainbow_product", 1);
    modal->addTransition(t5_final_assembly);

    create->getConnectionManager()->insert(modal);
    modal->getConnectionManager()->insert(dispose);

    model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
    model->getSimulation()->start();

    delete genesys;
    return 0;
}
