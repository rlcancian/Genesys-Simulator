#ifndef CPPMODELEXPORTER_H
#define CPPMODELEXPORTER_H

#include <string>

class Simulator;

namespace Ui {
class MainWindow;
}

// This service encapsulates generation of C++ model code shown in the GUI editor.
class CppModelExporter {
public:
    // MainWindow provides explicit dependencies once, keeping wrappers thin and stable.
    CppModelExporter(Simulator* simulator, Ui::MainWindow* ui);

    std::string addCppCodeLine(const std::string& line, unsigned int indent = 0) const;
    void actualizeModelCppCode() const;

private:
    Simulator* _simulator;
    Ui::MainWindow* _ui;
};

#endif // CPPMODELEXPORTER_H
