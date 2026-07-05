#ifndef WCMGENEEXPRESSION_H
#define WCMGENEEXPRESSION_H

#include "../../../BaseGenesysTerminalApplication.h"

/**
 * Central dogma simulation: stochastic transcription + translation + SSA degradation.
 *
 * Models 4 genes using two complementary approximations:
 *   - Poisson tau-leaping for transcription (StochasticTranscription) and
 *     translation (StochasticTranslation) — valid for high-copy-number RNAP/ribosome
 *   - Gillespie Direct Method SSA for low-copy-number mRNA and protein degradation
 *
 * Biological parameters (M. genitalium ballpark):
 *   RNAP_free      = 200 molecules
 *   ribosome_free  = 400 molecules
 *   4 genes: geneA (metabolic), geneB (regulatory), geneC (structural), geneD (transport)
 *   Transcription: elongRate=50 nt/s, meanGeneLen=900 nt, bindProb=0.25, Δt=60 s
 *   Translation:   elongRate=16 AA/s, meanProtLen=300 AA, Δt=60 s
 *   mRNA decay:    k = 0.01/s  (mean lifetime ≈ 100 s)
 *   Protein decay: k = 0.001/s (mean lifetime ≈ 1000 s ≈ 16 min)
 *
 * Simulation: 10 replications × 3600 s, 60 s time steps.
 *
 * Run via GENESYS_TERMINAL_EXAMPLE:
 *   cmake -DGENESYS_TERMINAL_EXAMPLE=wcm/WcmGeneExpression.cpp ...
 */
class WcmGeneExpression : public BaseGenesysTerminalApplication {
public:
    WcmGeneExpression();
    virtual int main(int argc, char** argv) override;
};

#endif /* WCMGENEEXPRESSION_H */
