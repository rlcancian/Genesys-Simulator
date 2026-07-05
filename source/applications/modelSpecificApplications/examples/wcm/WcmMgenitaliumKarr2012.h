#ifndef WCMMGENITALIUMKARR2012_H
#define WCMMGENITALIUMKARR2012_H

#include "../../../BaseGenesysTerminalApplication.h"

/**
 * Whole-cell simulation analog of the M. genitalium model (Karr et al. 2012).
 *
 * Reference:
 *   Karr JR, Sanghvi JC, Macklin DN, et al. "A Whole-Cell Computational Model
 *   Predicts Phenotype from Genotype." Cell 150(2):389–401, 2012.
 *   doi:10.1016/j.cell.2012.05.044
 *
 * Architecture:
 *   This model uses all four GenESyS WholeCellModeling plugins in sequence:
 *
 *     SimClock → Transcription → Translation → MetabolicSSA → CellDivision
 *                                                              ├─ port 0 (no division) → Dispose
 *                                                              └─ port 1 (division)    → RecordDivision → Dispose
 *
 *   StochasticTranscription: Poisson tau-leaping for 10 representative M. genitalium genes
 *   StochasticTranslation:   Poisson tau-leaping mapping mRNA_ → prot_ species
 *   StochasticReactionComponent: Gillespie SSA for ATP/GTP metabolism + FtsZ ring polymerization
 *   CellDivisionEvent:       Triggers when FtsZ ring reaches 35% completion (350/1000 units)
 *                            (mass-only trigger disabled; FtsZ-driven as in Karr et al.)
 *
 * Biologically realistic parameters (from CovertLab/WholeCell parameters.json, MIT):
 *   RNAP_free          = 200 molecules (Karr et al. 2012, Supplementary Table S2)
 *   ribosome_free      = 400 molecules
 *   Transcription rate = 50 nt/s
 *   Translation rate   = 16 AA/s
 *   Mean gene length   = 900 nt
 *   Mean protein len   = 300 AA
 *   Cell cycle         ≈ 600 min (modeled as 600 × 60 s steps)
 *
 * 10 representative genes (selected from M. genitalium 473-gene model):
 *   MG_001 — metabolic (glycolytic enzyme)
 *   MG_006 — ATP synthase subunit (energy metabolism)
 *   MG_055 — DNA polymerase III subunit alpha (DNA replication)
 *   MG_139 — RNAP beta subunit (transcription machinery)
 *   MG_175 — 30S ribosomal protein S1 (translation machinery)
 *   MG_196 — FtsZ (cell division; drives the division trigger)
 *   MG_203 — RNAP sigma factor (transcription regulation)
 *   MG_339 — phosphoglycerate mutase (central metabolism)
 *   MG_398 — pyruvate kinase (central metabolism)
 *   MG_462 — ribosomal protein L7/L12 (translation machinery)
 *
 * Simulation: 3 replications × 36000 s (10 h ≈ 1 M. genitalium cell cycle), 60 s steps.
 *
 * Run via GENESYS_TERMINAL_EXAMPLE:
 *   cmake -DGENESYS_TERMINAL_EXAMPLE=wcm/WcmMgenitaliumKarr2012.cpp ...
 */
class WcmMgenitaliumKarr2012 : public BaseGenesysTerminalApplication {
public:
    WcmMgenitaliumKarr2012();
    virtual int main(int argc, char** argv) override;
};

#endif /* WCMMGENITALIUMKARR2012_H */
