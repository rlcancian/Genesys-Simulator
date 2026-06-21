#pragma once

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_ModalModelPetriNet_Concurrency : public BaseGenesysTerminalApplication {
public:
    Smart_ModalModelPetriNet_Concurrency();
public:
    virtual int main(int argc, char** argv) override;
};
