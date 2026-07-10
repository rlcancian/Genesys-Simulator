/*
 * File:   Smart_BioKineticLawRegulation.h
 */

#ifndef SMART_BIOKINETICLAWREGULATION_H
#define SMART_BIOKINETICLAWREGULATION_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_BioKineticLawRegulation : public BaseGenesysTerminalApplication {
public:
	Smart_BioKineticLawRegulation();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_BIOKINETICLAWREGULATION_H */
