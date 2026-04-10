#ifndef GUICRASHDIAGNOSTICS_H
#define GUICRASHDIAGNOSTICS_H

namespace GuiCrashDiagnostics {

// Installs temporary fatal-signal handlers to print GUI crash backtraces.
void installFatalSignalHandlers();

} // namespace GuiCrashDiagnostics

#endif // GUICRASHDIAGNOSTICS_H
