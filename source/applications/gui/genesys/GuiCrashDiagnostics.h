#ifndef GUICRASHDIAGNOSTICS_H
#define GUICRASHDIAGNOSTICS_H

/**
 * @brief Crash diagnostics helpers for the Qt GUI process.
 *
 * This namespace keeps temporary signal-handling utilities isolated from the
 * rest of the GUI runtime so they can be installed only when needed during
 * crash investigation.
 */
namespace GuiCrashDiagnostics {

/**
 * @brief Installs temporary fatal-signal handlers that print a crash backtrace.
 */
void installFatalSignalHandlers();

} // namespace GuiCrashDiagnostics

#endif // GUICRASHDIAGNOSTICS_H
