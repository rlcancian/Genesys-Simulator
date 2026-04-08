#ifndef CPPMODELEXPORTER_H
#define CPPMODELEXPORTER_H

#include <string>

class QPlainTextEdit;
class Simulator;

// This Phase-1 service encapsulates generation of C++ model code shown in the GUI editor.
class CppModelExporter {
public:
    // MainWindow provides explicit dependencies once, keeping wrappers thin and stable.
    CppModelExporter(Simulator* simulator, QPlainTextEdit* cppCodeEditor);

    std::string addCppCodeLine(const std::string& line, unsigned int indent = 0) const;
    void actualizeModelCppCode() const;

private:
    Simulator* _simulator;
    QPlainTextEdit* _cppCodeEditor;
};

#endif // CPPMODELEXPORTER_H
