/*
 * File:   Smart_R_Simulator.h
 * Author: GenESyS
 *
 * Minimal smart model for the RSimulator component.
 */

#ifndef SMART_R_SIMULATOR_H
#define SMART_R_SIMULATOR_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Builds a minimal Create -> RSimulator -> Dispose model.
 *
 * The example exercises the component-level R integration and is intended as a
 * compact smoke model for RSimulator and its internal RSimulatorRunner.
 */
class Smart_R_Simulator : public BaseGenesysTerminalApplication {
public:
	/*! \brief Creates the smart R simulator example. */
	Smart_R_Simulator();

public:
	/*! \brief Builds, saves and simulates the example model. */
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_R_SIMULATOR_H */
