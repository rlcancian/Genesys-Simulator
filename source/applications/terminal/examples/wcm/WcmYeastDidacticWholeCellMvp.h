#ifndef WCMYEASTDIDACTICWHOLECELLMVP_H
#define WCMYEASTDIDACTICWHOCELLMVP_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Didactic yeast-oriented whole-cell MVP built on GEM-inspired metabolism.
 *
 * This example is intentionally small and pedagogical. It combines a compact
 * FBA network inspired by Saccharomyces cerevisiae carbon metabolism with
 * whole-cell state projection, compartment exchange, stochastic expression,
 * growth, lifecycle checkpointing, fate routing, and division.
 *
 * It is not a canonical or validated yeast whole-cell model. It should be
 * interpreted as a GenESyS architecture example informed by yeast-GEM style
 * metabolic concepts, not as a public operational WCM for S. cerevisiae.
 */
class WcmYeastDidacticWholeCellMvp : public BaseGenesysTerminalApplication {
public:
	WcmYeastDidacticWholeCellMvp();
	virtual int main(int argc, char** argv) override;
};

#endif /* WCMYEASTDIDACTICWHOCELLMVP_H */
