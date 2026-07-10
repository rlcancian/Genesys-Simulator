#ifndef SYSTEMPREFERENCES_H
#define SYSTEMPREFERENCES_H

#include "kernel/simulator/TraceManager.h"

#include <QString>
#include <string>
#include <vector>

/**
 * @brief Persistent application preferences used by the Qt GUI.
 *
 * The class behaves like a singleton configuration store backed by a local
 * preferences file. All access is static so the GUI can query and update the
 * current runtime settings without manually passing a preferences object around.
 */
class SystemPreferences {
public:
    /**
     * @brief Startup model selection remembered in the preferences file.
     */
    enum class StartupModelMode : unsigned short {
        NoModel = 0,
        NewModel = 1,
        OpenSpecificModel = 2,
        OpenLastModel = 3
    };

    /**
     * @brief Global visual style used by the GUI.
     */
    enum class VisualTheme {
        Light,
        Dark
    };

    /**
     * @brief Interface density/style preset used by the Qt application.
     */
    enum class InterfaceStyle {
        Classic,
        Modern
    };

    /**
     * @brief Automatic positioning policy for graphical elements.
     */
    enum class AutomaticPositioningStrategy {
        Legacy = 0,
        Centered = 1
    };

    /** @brief Loads preferences from the configuration file. */
    static bool load();
    /** @brief Saves the current preferences to disk. */
    static bool save();
    /** @brief Restores every preference to its built-in default value. */
    static void resetToDefaults();
    /** @brief Returns the full path to the active preferences file. */
    static QString configFilePath();

    /** @brief Returns whether the application should start maximized. */
    static bool startMaximized();
    /** @brief Updates the start-maximized preference. */
    static void setStartMaximized(bool newStartMaximized);

    /** @brief Returns whether plug-ins are loaded automatically at startup. */
    static bool autoLoadPlugins();
    /** @brief Updates the automatic plug-in loading preference. */
    static void setAutoLoadPlugins(bool newAutoLoadPlugins);

    /** @brief Returns the startup model mode. */
    static StartupModelMode startupModelMode();
    /** @brief Updates the startup model mode. */
    static void setStartupModelMode(StartupModelMode newStartupModelMode);
    /** @brief Returns the configured model index for the startup flow. */
    static unsigned short modelAtStart();
    /** @brief Updates the configured model index for the startup flow. */
    static void setModelAtStart(unsigned short newModelAtStart);

    /** @brief Returns the filename requested for startup loading. */
    static std::string modelfilename();
    /** @brief Updates the filename requested for startup loading. */
    static void setModelfilename(const std::string &newModelfilename);
    /** @brief Returns the last model filename successfully opened by the GUI. */
    static std::string lastModelFilename();
    /** @brief Updates the last model filename successfully opened by the GUI. */
    static void setLastModelFilename(const std::string& newLastModelFilename);
    /** @brief Returns the list of recently opened model files. */
    static std::vector<std::string> recentModelFiles();
    /** @brief Replaces the recent model file list. */
    static void setRecentModelFiles(const std::vector<std::string>& recentModelFiles);
    /** @brief Pushes a file to the recent model list with limit handling. */
    static void pushRecentModelFile(const std::string& modelFilename);
    /** @brief Returns the maximum number of recent model entries to keep. */
    static unsigned int recentModelFilesLimit();
    /** @brief Updates the maximum number of recent model entries. */
    static void setRecentModelFilesLimit(unsigned int recentModelFilesLimit);

    /** @brief Returns whether system packages should be checked at startup. */
    static bool checkSystemPackagesAtStart();
    /** @brief Updates the system-package check preference. */
    static void setCheckSystemPackagesAtStart(bool newCheckSystemPackagesAtStart);

    /** @brief Returns the current trace verbosity level. */
    static TraceManager::Level traceLevel();
    /** @brief Updates the trace verbosity level. */
    static void setTraceLevel(TraceManager::Level newTraceLevel);

    /** @brief Returns the selected visual theme. */
    static VisualTheme visualTheme();
    /** @brief Updates the selected visual theme. */
    static void setVisualTheme(VisualTheme newVisualTheme);
    /** @brief Returns the persisted name of the selected visual theme. */
    static std::string visualThemeName();

    /** @brief Returns the selected interface style preset. */
    static InterfaceStyle interfaceStyle();
    /** @brief Updates the selected interface style preset. */
    static void setInterfaceStyle(InterfaceStyle newInterfaceStyle);
    /** @brief Returns the persisted name of the selected interface style. */
    static std::string interfaceStyleName();

    /** @brief Returns the application font size used by the GUI. */
    static int applicationFontPointSize();
    /** @brief Updates the application font size used by the GUI. */
    static void setApplicationFontPointSize(int newApplicationFontPointSize);

    /** @brief Returns whether diagram widgets should derive colors from the theme. */
    static bool diagramUsesThemeColors();
    /** @brief Updates whether diagram widgets should derive colors from the theme. */
    static void setDiagramUsesThemeColors(bool newDiagramUsesThemeColors);
    /** @brief Returns the automatic placement strategy used by the diagram canvas. */
    static AutomaticPositioningStrategy automaticPositioningStrategy();
    /** @brief Updates the automatic placement strategy used by the diagram canvas. */
    static void setAutomaticPositioningStrategy(AutomaticPositioningStrategy strategy);

    /** @brief Returns the RGB color used for the main canvas background. */
    static unsigned int canvasBackgroundColor();
    /** @brief Returns the RGB color used for the disabled canvas background. */
    static unsigned int canvasDisabledBackgroundColor();
    /** @brief Returns the RGB color used for the diagram grid. */
    static unsigned int gridColor();

private:
    SystemPreferences(){};

    static QString defaultConfigDirectory();
    static StartupModelMode startupModelModeFromString(const QString& value);
    static QString startupModelModeToString(StartupModelMode value);
    static VisualTheme visualThemeFromString(const QString& value);
    static QString visualThemeToString(VisualTheme value);
    static InterfaceStyle interfaceStyleFromString(const QString& value);
    static QString interfaceStyleToString(InterfaceStyle value);
    static AutomaticPositioningStrategy automaticPositioningStrategyFromString(const QString& value);
    static QString automaticPositioningStrategyToString(AutomaticPositioningStrategy value);
    static TraceManager::Level traceLevelFromInt(int value);

    static bool _startMaximized;
    static bool _autoLoadPlugins;
    static bool _checkSystemPackagesAtStart;
    static StartupModelMode _startupModelMode;
    static std::string _modelfilenameToOpen;
    static std::string _lastModelFilename;
    static std::vector<std::string> _recentModelFiles;
    static unsigned int _recentModelFilesLimit;
    static TraceManager::Level _traceLevel;
    static VisualTheme _visualTheme;
    static InterfaceStyle _interfaceStyle;
    static int _applicationFontPointSize;
    static bool _diagramUsesThemeColors;
    static AutomaticPositioningStrategy _automaticPositioningStrategy;
};

#endif // SYSTEMPREFERENCES_H
