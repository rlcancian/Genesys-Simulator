#include "WcmMgenitaliumKarr2012.h"

#include <string>
#include <vector>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/InputOutput/Record.h"
#include "plugins/components/WholeCellModeling/CellCycleCheckpointComponent.h"
#include "plugins/components/WholeCellModeling/CellFateDecisionComponent.h"
#include "plugins/components/WholeCellModeling/CellGrowthComponent.h"
#include "plugins/components/WholeCellModeling/FtsZPolymerizationComponent.h"
#include "plugins/components/WholeCellModeling/MetabolicSubmodelComponent.h"
#include "plugins/components/WholeCellModeling/ResourceAllocationComponent.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"
#include "plugins/components/WholeCellModeling/StochasticReactionComponent.h"
#include "plugins/components/WholeCellModeling/CellDivisionEvent.h"

#include "plugins/data/WholeCellModeling/WholeCellState.h"
#include "plugins/data/WholeCellModeling/StochasticReactionRule.h"

WcmMgenitaliumKarr2012::WcmMgenitaliumKarr2012() {}

int WcmMgenitaliumKarr2012::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    this->setDefaultTraceHandlers(genesys->getTraceManager());
    genesys->getPluginManager()->autoInsertPlugins();

    Model* model = genesys->getModelManager()->newModel();
    PluginManager* plugins = genesys->getPluginManager();

    model->getInfos()->setName("WCM M. genitalium Karr 2012 Analog");
    model->getInfos()->setDescription(
        "Whole-cell simulation of Mycoplasma genitalium inspired by Karr et al. (2012). "
        "10 representative genes, all four WCM plugins: Transcription, Translation, "
        "MetabolicSSA, and CellDivision. FtsZ-driven cell division. "
        "Parameters from CovertLab/WholeCell (MIT license).");

    // -----------------------------------------------------------------------
    // Shared whole-cell state
    // -----------------------------------------------------------------------
    WholeCellState* state = plugins->newInstance<WholeCellState>(model, "CellState");

    // Cellular geometry — M. genitalium at mid-cycle
    state->setCellVolume(4.7e-17);   // ~47 aL, mid-cycle volume
    state->setCellMass(3.93e-15);    // ~3.93 fg, initial dry mass (parameters.json)

    // Molecular machines (free, available for synthesis)
    // From Karr et al. 2012, Supplementary Table S2
    state->setMoleculeCount("RNAP_free",       200);
    state->setMoleculeCount("ribosome_free",   400);

    // ATP/GTP metabolite pools for energy metabolism
    state->setMetaboliteAmount("ATP",  2.0e-3);  // 2 mM intracellular (typical)
    state->setMetaboliteAmount("ADP",  0.5e-3);
    state->setMetaboliteAmount("GTP",  0.5e-3);
    state->setMetaboliteAmount("GDP",  0.1e-3);

    // FtsZ ring formation (drives cell division trigger)
    // FtsZ_ring_completion: 0–1000 (per-mille); division at >= 500 (50% ring)
    // FtsZ_monomer: initial pool from which the ring assembles
    state->setMoleculeCount("FtsZ_monomer",        500);  // ~500 FtsZ molecules in M. genitalium
    state->setMoleculeCount("FtsZ_ring_completion", 0);
    state->setLifecyclePhase("newborn");

    // 10 representative M. genitalium genes: initial state (new cell born after division)
    // mRNA and protein counts represent one daughter cell's allocation
    // Realistic starting counts derived from Karr et al. Supplementary Data
    const std::vector<std::pair<std::string, std::pair<int,int>>> geneInitial = {
        // {gene_id, {mRNA_initial, protein_initial}}
        {"MG_001",  {2,  80}},   // metabolic enzyme (glycolysis)
        {"MG_006",  {1,  40}},   // ATP synthase subunit
        {"MG_055",  {1,  10}},   // DNA polymerase III alpha
        {"MG_139",  {1,  50}},   // RNAP beta subunit
        {"MG_175",  {3, 120}},   // 30S ribosomal protein S1
        {"MG_196",  {2, 250}},   // FtsZ (abundant cell division protein)
        {"MG_203",  {1,  30}},   // RNAP sigma factor
        {"MG_339",  {2,  60}},   // phosphoglycerate mutase
        {"MG_398",  {2,  70}},   // pyruvate kinase
        {"MG_462",  {4, 160}},   // ribosomal protein L7/L12
    };

    for (const auto& [gene, counts] : geneInitial) {
        state->setMoleculeCount("mRNA_" + gene, counts.first);
        state->setMoleculeCount("prot_" + gene, counts.second);
    }

    // -----------------------------------------------------------------------
    // Metabolic + division-preparation SSA reactions
    // -----------------------------------------------------------------------

    // ATP regeneration (simplified glycolysis/substrate-level phosphorylation)
    // ADP → ATP at steady-state; modeled as first-order regeneration
    // Real M. genitalium uses a mixed phospholipid/substrate energy scheme;
    // here we use a simple unimolecular approximation.
    // Note: metabolite amounts are continuous, but StochasticReactionRule operates
    // on integer molecule counts; ATP and ADP are tracked as integer counts in
    // the molecule map (scaled: 1 unit = 1e6 molecules).
    state->setMoleculeCount("ATP_pool", 2000);  // 2000 units (arbitrary scale)
    state->setMoleculeCount("ADP_pool",  500);

    StochasticReactionRule* rAtpRegen = plugins->newInstance<StochasticReactionRule>(
        model, "R_ATP_Regeneration");
    rAtpRegen->addReactant("ADP_pool", 1);
    rAtpRegen->addProduct("ATP_pool", 1);
    rAtpRegen->setRateConstant(0.05);  // ADP → ATP, k = 0.05/s

    StochasticReactionRule* rAtpConsume = plugins->newInstance<StochasticReactionRule>(
        model, "R_ATP_Consumption");
    rAtpConsume->addReactant("ATP_pool", 1);
    rAtpConsume->addProduct("ADP_pool", 1);
    rAtpConsume->setRateConstant(0.02);  // ATP hydrolysis to ADP, k = 0.02/s

    // FtsZ ring kinetics are handled by FtsZPolymerizationComponent (Phase 3).
    // SSA reactions only cover mRNA/protein degradation and ATP cycle below.

    // mRNA degradation for all 10 genes (first-order, k = 0.01/s)
    for (const auto& [gene, counts] : geneInitial) {
        StochasticReactionRule* rDeg = plugins->newInstance<StochasticReactionRule>(
            model, "R_deg_mRNA_" + gene);
        rDeg->addReactant("mRNA_" + gene, 1);
        rDeg->setRateConstant(0.01);
    }

    // Protein degradation for all 10 genes (first-order, k = 0.001/s)
    for (const auto& [gene, counts] : geneInitial) {
        StochasticReactionRule* rDeg = plugins->newInstance<StochasticReactionRule>(
            model, "R_deg_prot_" + gene);
        rDeg->addReactant("prot_" + gene, 1);
        rDeg->setRateConstant(0.001);
    }

    // -----------------------------------------------------------------------
    // Component pipeline
    // -----------------------------------------------------------------------

    // Clock: 1 entity per 60 s biological time step
    // 600 steps × 60 s = 36000 s ≈ 10 h ≈ 1 M. genitalium cell cycle per replication
    Create* clock = plugins->newInstance<Create>(model, "SimClock");
    clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
    clock->setMaxCreations(600);

    // Phase 2: Cell growth — exponential mass increase each 60 s step
    CellGrowthComponent* growth = plugins->newInstance<CellGrowthComponent>(model, "CellGrowth");
    growth->setWholeCellState(state);
    growth->setGrowthRate(2.1393e-05);  // /s — states.MetabolicReaction.meanInitialGrowthRate
    growth->setDensity(1100.0);          // kg/m³ — states.Geometry.density
    growth->setDeltaT(60.0);

    // Phase 4: Metabolic submodel — ATP production each step
    MetabolicSubmodelComponent* metabolism = plugins->newInstance<MetabolicSubmodelComponent>(
        model, "Metabolism");
    metabolism->setWholeCellState(state);
    metabolism->setAtpYieldRate(1.0e-3); // mmol ATP/fL/s
    metabolism->setDeltaT(60.0);

    // Phase 5: Resource allocation — divide RNAP and ribosomes across genes
    ResourceAllocationComponent* resAlloc = plugins->newInstance<ResourceAllocationComponent>(
        model, "ResourceAllocation");
    resAlloc->setWholeCellState(state);
    resAlloc->setRnapCountKey("RNAP_free");
    resAlloc->setRibosomeCountKey("ribosome_free");
    resAlloc->setMRNASpeciesPrefix("mRNA_");
    resAlloc->setProteinSpeciesPrefix("prot_");

    // Transcription (Poisson tau-leaping)
    // bindingProbability calibrated to ~0.67 mRNA per gene per step (Phase 6a)
    // λ_gene = (50 nt/s / 900 nt) × 0.01 × RNAP_free × 60 s ≈ 0.67 per gene
    StochasticTranscription* txn = plugins->newInstance<StochasticTranscription>(model, "Transcription");
    txn->setWholeCellState(state);
    txn->setElongationRate(50.0);       // nt/s — M. genitalium RNAP elongation (parameters.json)
    txn->setMeanGeneLength(900.0);      // nt — weighted average across 473 genes
    txn->setBindingProbability(0.01);   // calibrated: ~0.67 mRNA/gene/step (Phase 6a fix)
    txn->setTimeWindow(60.0);           // s — tau-leaping step size
    txn->setMRNASpeciesPrefix("mRNA_");
    txn->setRnapCountKey("RNAP_free");
    txn->setRandomSeed(1001u);

    // Translation (Poisson tau-leaping)
    // λ_gene = (16 AA/s / 300 AA) × mRNA_gene × ribo_free / totalMRNA × 60 s
    StochasticTranslation* tln = plugins->newInstance<StochasticTranslation>(model, "Translation");
    tln->setWholeCellState(state);
    tln->setElongationRate(16.0);       // AA/s — M. genitalium ribosome (parameters.json)
    tln->setMeanProteinLength(300.0);   // AA — approximate weighted average
    tln->setTimeWindow(60.0);           // s
    tln->setMRNASpeciesPrefix("mRNA_");
    tln->setProteinSpeciesPrefix("prot_");
    tln->setRibosomeCountKey("ribosome_free");
    tln->setRandomSeed(2002u);

    // Metabolic SSA: ATP/GTP energy reactions + FtsZ ring assembly + all degradation reactions
    StochasticReactionComponent* metabolicSsa = plugins->newInstance<StochasticReactionComponent>(
        model, "MetabolicSSA");
    metabolicSsa->setWholeCellState(state);
    metabolicSsa->setTimeWindow(60.0);
    metabolicSsa->setAdvanceWholeCellClock(false);
    metabolicSsa->setRandomSeed(3003u);

    // Phase 3: FtsZ ring polymerization — real kinetics, ~35% at ~12000 s
    FtsZPolymerizationComponent* ftsZ = plugins->newInstance<FtsZPolymerizationComponent>(
        model, "FtsZPolymerization");
    ftsZ->setWholeCellState(state);
    ftsZ->setActivationFwd(1.1);    // /s — processes.FtsZPolymerization.activationFwd
    ftsZ->setActivationRev(0.01);   // /s
    ftsZ->setVolumeNorm(3.94e-5);   // calibrated for ~35% ring completion at ~12000 s
    ftsZ->setDeltaT(60.0);
    ftsZ->setFtsZRingKey("FtsZ_ring_completion");
    ftsZ->setFtsZMonomerKey("FtsZ_monomer");

    CellCycleCheckpointComponent* checkpoint = plugins->newInstance<CellCycleCheckpointComponent>(
        model, "LifecycleCheckpoint");
    checkpoint->setWholeCellState(state);
    checkpoint->setDeltaT(60.0);
    checkpoint->setEnergyMetaboliteKey("ATP");
    checkpoint->setStarvationAtpThreshold(5.0e-4);
    checkpoint->setDivisionMassThreshold(0.0);
    checkpoint->setFtsZRingKey("FtsZ_ring_completion");
    checkpoint->setFtsZThreshold(0.35);
    checkpoint->setAdvanceWholeCellClock(true);

    CellFateDecisionComponent* fate = plugins->newInstance<CellFateDecisionComponent>(
        model, "CellFate");
    fate->setWholeCellState(state);

    // Cell division: FtsZ ring threshold 35% (≥350/1000), mass threshold disabled
    CellDivisionEvent* divisionEvent = plugins->newInstance<CellDivisionEvent>(model, "CellDivision");
    divisionEvent->setWholeCellState(state);
    divisionEvent->setDivisionMassThreshold(0.0);   // FtsZ-only trigger
    divisionEvent->setFtsZThreshold(0.35);           // 350/1000 = 35% — reached ~12000 s (Phase 3)
    divisionEvent->setFtsZRingKey("FtsZ_ring_completion");
    divisionEvent->setRandomSeed(4004u);

    // Record component for division events (port 1 of CellDivisionEvent)
    Record* recordDivision = plugins->newInstance<Record>(model, "Division_Record");
    recordDivision->setExpression("TNOW");
    recordDivision->setExpressionName("Cell Division Time");
    recordDivision->setDatasetName("CellDivisionTimes");
    recordDivision->setRandomVariableName("Time of cell division");
    recordDivision->setDatasetDescription(
        "Simulation times at which cell division events occurred (M. genitalium Karr 2012 analog).");

    Dispose* sinkNormal   = plugins->newInstance<Dispose>(model, "Step_Done");
    Dispose* sinkDivision = plugins->newInstance<Dispose>(model, "Post_Division");
    Dispose* sinkStarved  = plugins->newInstance<Dispose>(model, "Starved_Done");
    Dispose* sinkDead     = plugins->newInstance<Dispose>(model, "Dead_Done");

    // -----------------------------------------------------------------------
    // Connections
    // -----------------------------------------------------------------------
    // Pipeline: Clock → Growth → Metabolism → ResourceAlloc →
    //           Transcription → Translation → MetabolicSSA(degradation) →
    //           FtsZPolymerization → LifecycleCheckpoint → CellFateDecision
    clock->connectTo(growth);
    growth->connectTo(metabolism);
    metabolism->connectTo(resAlloc);
    resAlloc->connectTo(txn);
    txn->connectTo(tln);
    tln->connectTo(metabolicSsa);
    metabolicSsa->connectTo(ftsZ);
    ftsZ->connectTo(checkpoint);
    checkpoint->connectTo(fate);

    // CellFateDecisionComponent:
    //   port 0 — viable flow without division
    //   port 1 — division-ready cell
    //   port 2 — starved but still viable
    //   port 3 — dead cell
    fate->connectTo(sinkNormal);
    fate->connectTo(divisionEvent);
    fate->connectTo(sinkStarved);
    fate->connectTo(sinkDead);

    // CellDivisionEvent:
    //   port 0 — fallback if division condition is no longer satisfied
    //   port 1 — division occurred
    divisionEvent->connectTo(sinkNormal);
    divisionEvent->connectTo(recordDivision);
    recordDivision->connectTo(sinkDivision);

    // -----------------------------------------------------------------------
    // Simulation parameters
    // -----------------------------------------------------------------------
    ModelSimulation* sim = model->getSimulation();
    sim->setNumberOfReplications(3);
    sim->setReplicationLength(36000.0, Util::TimeUnit::second);  // 10 h per replication
    sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
    model->getTracer()->setTraceLevel(TraceManager::Level::L2_results);

    model->save("./models/WcmMgenitaliumKarr2012.gen");
    sim->start();

    delete genesys;
    return 0;
}
