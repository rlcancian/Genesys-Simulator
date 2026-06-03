#ifndef WCMSIMPLESSA_H
#define WCMSIMPLESSA_H

#include "../../../BaseGenesysTerminalApplication.h"

/**
 * Minimal whole-cell modeling example using the Gillespie Direct Method SSA.
 *
 * Models stochastic gene expression of a single gene via 4 reactions:
 *   R1: transcription  — DNA + RNAP → DNA + RNAP + mRNA_geneA   (k = 0.02/s)
 *   R2: mRNA degradation — mRNA_geneA → ∅                       (k = 0.01/s)
 *   R3: translation    — mRNA_geneA → mRNA_geneA + prot_geneA   (k = 0.10/s)
 *   R4: protein degradation — prot_geneA → ∅                    (k = 0.001/s)
 *
 * Steady-state analytic predictions:
 *   mRNA_geneA ≈ 2 molecules   (k_tx / k_deg_mRNA = 0.02 / 0.01)
 *   prot_geneA ≈ 200 molecules (k_tl * mRNA_ss / k_deg_prot = 0.1 * 2 / 0.001)
 *
 * Simulation runs 30 replications × 3600 s each (1 hour of biological time)
 * in 60 s Gillespie windows triggered by periodic clock entities.
 *
 * Run via GENESYS_TERMINAL_EXAMPLE:
 *   cmake -DGENESYS_TERMINAL_EXAMPLE=wcm/WcmSimpleSsa.cpp ...
 */
class WcmSimpleSsa : public BaseGenesysTerminalApplication {
public:
    WcmSimpleSsa();
    virtual int main(int argc, char** argv) override;
};

#endif /* WCMSIMPLESSA_H */
