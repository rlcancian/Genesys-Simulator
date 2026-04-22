/*
 * File:   Smart_BioReversibleKineticLaw.h
 */

#ifndef SMART_BIOREVERSIBLEKINETICLAW_H
#define SMART_BIOREVERSIBLEKINETICLAW_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_BioReversibleKineticLaw : public BaseGenesysTerminalApplication {
public:
	Smart_BioReversibleKineticLaw();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_BIOREVERSIBLEKINETICLAW_H */
