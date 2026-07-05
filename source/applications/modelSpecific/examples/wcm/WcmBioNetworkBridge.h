#ifndef WCMBIONETWORKBRIDGE_H
#define WCMBIONETWORKBRIDGE_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Demonstrates a minimal deterministic/stochastic whole-cell bridge.
 *
 * The example builds a small deterministic BioNetwork for nutrient conversion,
 * projects its final state into a WholeCellState, and then runs stochastic
 * transcription, translation, and degradation components. It is a didactic
 * integration test for GenESyS abstractions, not a complete organism model.
 */
class WcmBioNetworkBridge : public BaseGenesysTerminalApplication {
public:
	WcmBioNetworkBridge();
	virtual int main(int argc, char** argv) override;
};

#endif /* WCMBIONETWORKBRIDGE_H */
