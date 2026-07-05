#ifndef WCMCOMPARTMENTTRANSPORTWHOLECELLMVP_H
#define WCMCOMPARTMENTTRANSPORTWHOLECELLMVP_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Whole-cell MVP with compartment-aware metabolic transport.
 *
 * This example extends the flux-balance whole-cell path with a tiny
 * extracellular-to-cytosol transport chain. It demonstrates how projected
 * metabolic fluxes can feed:
 * - global energy proxies;
 * - compartment-scoped metabolite pools; and
 * - pathway activity checkpoints.
 *
 * The biological scope is still didactic. The purpose is to validate the
 * architecture for multi-compartment hybrid whole-cell simulations.
 */
class WcmCompartmentTransportWholeCellMvp : public BaseGenesysTerminalApplication {
public:
	WcmCompartmentTransportWholeCellMvp();
	virtual int main(int argc, char** argv) override;
};

#endif /* WCMCOMPARTMENTTRANSPORTWHOLECELLMVP_H */
