#pragma once

#include "../GenesysApplication_if.h"

class BaseGenesysWebApplication : public GenesysApplication_if {
public:
    int main(int argc, char** argv) override;

private:
    static unsigned short _readPort(int argc, char** argv);
    static unsigned long _readMaxRequests(int argc, char** argv);
};
