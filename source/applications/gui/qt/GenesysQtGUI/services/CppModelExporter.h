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
    // This method preserves legacy indentation formatting for generated C++ lines.
    std::string addCppCodeLine(const std::string& line, unsigned int indent = 0) const;

    // This method regenerates and displays the C++ representation for the current model.
    void actualizeModelCppCode(Simulator* simulator, Ui::MainWindow* ui) const;
};

#endif // CPPMODELEXPORTER_H
