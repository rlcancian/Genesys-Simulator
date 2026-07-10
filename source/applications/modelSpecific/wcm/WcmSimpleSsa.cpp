#include "WcmSimpleSsa.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/WholeCellModeling/StochasticReactionComponent.h"

#include "plugins/data/WholeCellModeling/WholeCellState.h"
#include "plugins/data/WholeCellModeling/StochasticReactionRule.h"

WcmSimpleSsa::WcmSimpleSsa() {}

int WcmSimpleSsa::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    this->setDefaultTraceHandlers(genesys->getTraceManager());
    genesys->getPluginManager()->autoInsertPlugins();

    Model* model = genesys->getModelManager()->newModel();
    PluginManager* plugins = genesys->getPluginManager();

    model->getInfos()->setName("WCM Simple SSA");
    model->getInfos()->setDescription(
        "Stochastic gene expression of a single gene via Gillespie SSA. "
        "Analogous to the canonical two-state gene model (Raj & van Oudenaarden 2008). "
        "Four reactions: transcription, mRNA degradation, translation, protein degradation.");

    // Shared whole-cell state
    WholeCellState* state = plugins->newInstance<WholeCellState>(model, "CellState");
    state->setMoleculeCount("DNA_geneA",   1);
    state->setMoleculeCount("RNAP",       10);
    state->setMoleculeCount("mRNA_geneA",  0);
    state->setMoleculeCount("prot_geneA",  0);

    // Reaction rules
    // R1: Transcription — DNA + RNAP catalytically produce one mRNA molecule
    StochasticReactionRule* r1 = plugins->newInstance<StochasticReactionRule>(model, "R1_Transcription");
    r1->addReactant("DNA_geneA", 1);
    r1->addReactant("RNAP", 1);
    r1->addProduct("DNA_geneA", 1);
    r1->addProduct("RNAP", 1);
    r1->addProduct("mRNA_geneA", 1);
    r1->setRateConstant(0.02);  // propensity = 0.02 * DNA * RNAP = 0.02 * 1 * 10 = 0.2/s

    // R2: mRNA degradation
    StochasticReactionRule* r2 = plugins->newInstance<StochasticReactionRule>(model, "R2_mRNA_Degradation");
    r2->addReactant("mRNA_geneA", 1);
    r2->setRateConstant(0.01);  // mean mRNA lifetime = 100 s

    // R3: Translation — ribosome-free translation (mRNA is catalyst)
    StochasticReactionRule* r3 = plugins->newInstance<StochasticReactionRule>(model, "R3_Translation");
    r3->addReactant("mRNA_geneA", 1);
    r3->addProduct("mRNA_geneA", 1);
    r3->addProduct("prot_geneA", 1);
    r3->setRateConstant(0.10);  // 0.1 proteins/s per mRNA molecule

    // R4: Protein degradation
    StochasticReactionRule* r4 = plugins->newInstance<StochasticReactionRule>(model, "R4_Protein_Degradation");
    r4->addReactant("prot_geneA", 1);
    r4->setRateConstant(0.001);  // mean protein lifetime = 1000 s

    // Clock source: one entity per 60 s drives the SSA forward
    Create* clock = plugins->newInstance<Create>(model, "SimClock");
    clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
    clock->setMaxCreations(60);  // 60 steps × 60 s = 3600 s of biological time per replication

    // SSA runner
    StochasticReactionComponent* ssa = plugins->newInstance<StochasticReactionComponent>(model, "GillespieSSA");
    ssa->setWholeCellState(state);
    ssa->setTimeWindow(60.0);   // Gillespie advances 60 s each step
    ssa->setRandomSeed(42u);

    // Sink
    Dispose* sink = plugins->newInstance<Dispose>(model, "Done");

    // Connections
    clock->connectTo(ssa);
    ssa->connectTo(sink);

    // Simulation parameters
    ModelSimulation* sim = model->getSimulation();
    sim->setNumberOfReplications(30);
    sim->setReplicationLength(3600.0, Util::TimeUnit::second);
    sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
    model->getTracer()->setTraceLevel(TraceManager::Level::L2_results);

    model->save("./models/WcmSimpleSsa.gen");
    sim->start();

    delete genesys;
    return 0;
}
