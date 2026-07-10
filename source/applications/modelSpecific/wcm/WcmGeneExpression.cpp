#include "WcmGeneExpression.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"
#include "plugins/components/WholeCellModeling/StochasticReactionComponent.h"

#include "plugins/data/WholeCellModeling/WholeCellState.h"
#include "plugins/data/WholeCellModeling/StochasticReactionRule.h"

WcmGeneExpression::WcmGeneExpression() {}

int WcmGeneExpression::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    this->setDefaultTraceHandlers(genesys->getTraceManager());
    genesys->getPluginManager()->autoInsertPlugins();

    Model* model = genesys->getModelManager()->newModel();
    PluginManager* plugins = genesys->getPluginManager();

    model->getInfos()->setName("WCM Gene Expression");
    model->getInfos()->setDescription(
        "Central dogma: stochastic transcription (Poisson tau-leaping) + "
        "translation (Poisson tau-leaping) + mRNA/protein degradation (Gillespie SSA). "
        "Four genes with RNAP_free=200 and ribosome_free=400 (M. genitalium ballpark).");

    // Shared whole-cell state — all four component plugins read/write this object
    WholeCellState* state = plugins->newInstance<WholeCellState>(model, "CellState");

    // RNAP and ribosome pools (free molecules available for synthesis)
    state->setMoleculeCount("RNAP_free",      200);
    state->setMoleculeCount("ribosome_free",  400);

    // Four genes: initial mRNA and protein counts = 0
    const std::vector<std::string> genes = {"geneA", "geneB", "geneC", "geneD"};
    for (const auto& g : genes) {
        state->setMoleculeCount("mRNA_" + g, 0);
        state->setMoleculeCount("prot_" + g, 0);
    }

    // Degradation reactions (Gillespie SSA — small copy numbers, exact stochastic)
    // mRNA half-life ≈ 70 s (k_deg = 0.01/s) — within M. genitalium range
    // Protein half-life ≈ 1000 s (k_deg = 0.001/s) — stable housekeeping proteins
    for (const auto& g : genes) {
        StochasticReactionRule* rDegMRNA = plugins->newInstance<StochasticReactionRule>(
            model, "R_deg_mRNA_" + g);
        rDegMRNA->addReactant("mRNA_" + g, 1);
        rDegMRNA->setRateConstant(0.01);

        StochasticReactionRule* rDegProt = plugins->newInstance<StochasticReactionRule>(
            model, "R_deg_prot_" + g);
        rDegProt->addReactant("prot_" + g, 1);
        rDegProt->setRateConstant(0.001);
    }

    // Clock: one entity per 60 s drives one biological time step
    Create* clock = plugins->newInstance<Create>(model, "SimClock");
    clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
    clock->setMaxCreations(60);  // 60 steps × 60 s = 3600 s per replication

    // Transcription: Poisson tau-leaping over all mRNA_ species in WholeCellState
    // λ_gene = (elongRate / geneLen) × bindProb × RNAP_free × Δt
    //        = (50 / 900) × 0.25 × 200 × 60 ≈ 167 mRNA synthesized per step across all genes
    StochasticTranscription* txn = plugins->newInstance<StochasticTranscription>(model, "Transcription");
    txn->setWholeCellState(state);
    txn->setElongationRate(50.0);       // nt/s — M. genitalium RNA polymerase
    txn->setMeanGeneLength(900.0);      // nt — approximate M. genitalium average
    txn->setBindingProbability(0.25);   // fraction of time window a gene is accessible
    txn->setTimeWindow(60.0);           // s
    txn->setMRNASpeciesPrefix("mRNA_");
    txn->setRnapCountKey("RNAP_free");
    txn->setRandomSeed(101u);

    // Translation: Poisson tau-leaping over all prot_ species using mRNA_ as template
    // λ_gene = (elongRate / protLen) × mRNA_count × ribo_free / totalMRNA × Δt
    StochasticTranslation* tln = plugins->newInstance<StochasticTranslation>(model, "Translation");
    tln->setWholeCellState(state);
    tln->setElongationRate(16.0);       // AA/s — M. genitalium ribosome elongation rate
    tln->setMeanProteinLength(300.0);   // AA — approximate M. genitalium average
    tln->setTimeWindow(60.0);           // s
    tln->setMRNASpeciesPrefix("mRNA_");
    tln->setProteinSpeciesPrefix("prot_");
    tln->setRibosomeCountKey("ribosome_free");
    tln->setRandomSeed(202u);

    // SSA degradation runner — processes all StochasticReactionRule instances in model
    StochasticReactionComponent* degradation = plugins->newInstance<StochasticReactionComponent>(
        model, "Degradation_SSA");
    degradation->setWholeCellState(state);
    degradation->setTimeWindow(60.0);
    degradation->setRandomSeed(303u);

    Dispose* sink = plugins->newInstance<Dispose>(model, "Done");

    // Connections: each entity flows through transcription → translation → degradation → dispose
    clock->connectTo(txn);
    txn->connectTo(tln);
    tln->connectTo(degradation);
    degradation->connectTo(sink);

    // Simulation parameters
    ModelSimulation* sim = model->getSimulation();
    sim->setNumberOfReplications(10);
    sim->setReplicationLength(3600.0, Util::TimeUnit::second);
    sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
    model->getTracer()->setTraceLevel(TraceManager::Level::L2_results);

    model->save("./models/WcmGeneExpression.gen");
    sim->start();

    delete genesys;
    return 0;
}
