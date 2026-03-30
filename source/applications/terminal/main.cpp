//
// Created by rafaelcancian on 30/03/2026.
//

#include "TraitsTerminalApp.h"

int main(int argc, char *argv[]) {
    GenesysApplication_if *app = new TraitsTerminalApp<GenesysApplication_if>::Application();
    return app->main(argc, argv);
}
