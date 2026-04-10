#ifndef CPPMODELEXPORTER_H
#define CPPMODELEXPORTER_H

#include <string>

class QPlainTextEdit;
class Simulator;

// Document the service that generates C++ model representation text.
/**
 * @brief Service responsible for C++ code representation of the current simulation model.
 *
 * The refactoring keeps C++ representation generation outside MainWindow by delegating this
 * concern to a dedicated service. MainWindow wrappers call into this class when model or UI
 * state changes require regeneration.
 *
 * Responsibilities:
 * - format indented C++ output lines used by exporter routines;
 * - rebuild the full C++ representation shown in the GUI code pane.
 *
 * Boundaries:
 * - it does not compile or execute generated code;
 * - it does not persist files directly;
 * - it does not manage lifecycle/simulation/scene controller flows.
 */
class CppModelExporter {
public:
    /** @brief Creates the exporter service for the C++ model-representation pane. */
    CppModelExporter(Simulator* simulator, QPlainTextEdit* cppCodeEditor);

    /**
     * @brief Formats one C++ output line with the requested indentation level.
     * @return Formatted line that can be appended to the representation buffer.
     */
    std::string addCppCodeLine(const std::string& line, unsigned int indent = 0) const;
    /** @brief Rebuilds the full C++ model representation shown in the GUI editor pane. */
    void actualizeModelCppCode() const;

private:
    Simulator* _simulator;
    QPlainTextEdit* _cppCodeEditor;
};

#endif // CPPMODELEXPORTER_H
