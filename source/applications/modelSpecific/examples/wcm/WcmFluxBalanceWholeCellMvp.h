#ifndef WCMFLUXBALANCEWHOLECELLMVP_H
#define WCMFLUXBALANCEWHOLECELLMVP_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Whole-cell MVP driven by flux-balance metabolism.
 *
 * This example demonstrates a second metabolic integration path for GenESyS
 * whole-cell models. Instead of using the deterministic BioNetwork runner, it
 * executes a small FBA problem through MetabolicFluxBalance, projects selected
 * fluxes into WholeCellState, and then runs expression, growth, lifecycle
 * checkpointing, fate routing, and division.
 */
class WcmFluxBalanceWholeCellMvp : public BaseGenesysTerminalApplication {
public:
	WcmFluxBalanceWholeCellMvp();
	virtual int main(int argc, char** argv) override;
};

#endif /* WCMFLUXBALANCEWHOLECELLMVP_H */
