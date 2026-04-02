//
// Created by rafaelcancian on 30/03/2026.
//

#include <memory>
#include "TraitsTerminalApp.h"

#if defined(GENESYS_TERMINAL_USE_TRAITS_APP)
using SelectedTerminalApplication = TraitsTerminalApp<GenesysApplication_if>::Application;
#else
class SelectedTerminalApplication : public GenesysApplication_if {
public:
    int main(int argc, char* argv[]) override {
        (void) argc;
        (void) argv;
        return 0;
    }
};
#endif

int main(int argc, char *argv[]) {
    auto app = std::make_unique<SelectedTerminalApplication>();
    return app->main(argc, argv);
}
