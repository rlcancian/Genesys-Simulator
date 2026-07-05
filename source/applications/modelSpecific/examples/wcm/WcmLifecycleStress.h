#ifndef WCMLIFECYCLESTRESS_H
#define WCMLIFECYCLESTRESS_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Demonstrates starvation and death routing in a whole-cell stress scenario.
 *
 * This example focuses on lifecycle checkpointing rather than biosynthesis. It
 * uses a low-energy whole-cell state plus the CellCycleCheckpointComponent and
 * CellFateDecisionComponent to show the transition:
 * newborn -> starved -> dead.
 */
class WcmLifecycleStress : public BaseGenesysTerminalApplication {
public:
	WcmLifecycleStress();
	virtual int main(int argc, char** argv) override;
};

#endif /* WCMLIFECYCLESTRESS_H */
