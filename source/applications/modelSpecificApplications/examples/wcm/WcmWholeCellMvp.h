#ifndef WCMWHOLECELLMVP_H
#define WCMWHOLECELLMVP_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Minimal whole-cell MVP combining metabolism, expression, growth, and division.
 *
 * This example is intentionally small and didactic. It combines:
 * - deterministic biochemical metabolism through BioNetwork;
 * - runtime projection into WholeCellState;
 * - stochastic transcription and translation;
 * - energy-gated mass growth;
 * - mass-triggered cell division.
 *
 * The result is not a calibrated organism model. It is a compact whole-cell
 * analog intended to validate the GenESyS architecture for hybrid
 * deterministic/stochastic cell simulations.
 */
class WcmWholeCellMvp : public BaseGenesysTerminalApplication {
public:
	WcmWholeCellMvp();
	virtual int main(int argc, char** argv) override;
};

#endif /* WCMWHOLECELLMVP_H */
