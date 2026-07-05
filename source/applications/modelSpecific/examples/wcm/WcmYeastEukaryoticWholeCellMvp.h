#ifndef WCMYEASTEUKARYOTICWHOLECELLMVP_H
#define WCMYEASTEUKARYOTICWHOLECELLMVP_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Didactic eukaryotic whole-cell MVP inspired by budding yeast.
 *
 * This example combines:
 * - a compact GEM-inspired FBA core;
 * - explicit biological compartments;
 * - compartment exchange and organelle energy proxies;
 * - stochastic expression and growth; and
 * - a simplified eukaryotic cell-cycle coordinator.
 *
 * It is a didactic whole-cell example for Saccharomyces cerevisiae-inspired
 * architecture. It is not a canonical public yeast whole-cell model.
 */
class WcmYeastEukaryoticWholeCellMvp : public BaseGenesysTerminalApplication {
public:
	WcmYeastEukaryoticWholeCellMvp();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* WCMYEASTEUKARYOTICWHOLECELLMVP_H */
