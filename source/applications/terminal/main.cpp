//
// Created by rafaelcancian on 30/03/2026.
//

#include <memory>

#ifndef GENESYS_TERMINAL_APP_HEADER
#define GENESYS_TERMINAL_APP_HEADER "GenesysShell/GenesysShell.h"
#endif

#ifndef GENESYS_TERMINAL_APP_CLASS
#define GENESYS_TERMINAL_APP_CLASS GenesysShell
#endif

#include GENESYS_TERMINAL_APP_HEADER

using SelectedTerminalApplication = GENESYS_TERMINAL_APP_CLASS;

int main(int argc, char *argv[]) {
    auto app = std::make_unique<SelectedTerminalApplication>();
    return app->main(argc, argv);
}
