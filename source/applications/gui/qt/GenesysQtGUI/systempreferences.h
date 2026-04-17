#ifndef SYSTEMPREFERENCES_H
#define SYSTEMPREFERENCES_H

#include "kernel/simulator/TraceManager.h"

#include <QString>
#include <string>

class SystemPreferences {
public:
    enum class StartupModelMode : unsigned short {
        NoModel = 0,
        NewModel = 1,
        OpenSpecificModel = 2,
        OpenLastModel = 3
    };

    enum class VisualTheme {
        Light,
        Dark
    };

    enum class InterfaceStyle {
        Classic,
        Modern
    };

    static bool load();
    static bool save();
    static void resetToDefaults();
    static QString configFilePath();

    static bool startMaximized();
    static void setStartMaximized(bool newStartMaximized);

    static bool autoLoadPlugins();
    static void setAutoLoadPlugins(bool newAutoLoadPlugins);

    static StartupModelMode startupModelMode();
    static void setStartupModelMode(StartupModelMode newStartupModelMode);
    static unsigned short modelAtStart();
    static void setModelAtStart(unsigned short newModelAtStart);

    static std::string modelfilename();
    static void setModelfilename(const std::string &newModelfilename);
    static std::string lastModelFilename();
    static void setLastModelFilename(const std::string& newLastModelFilename);

    static bool checkSystemPackagesAtStart();
    static void setCheckSystemPackagesAtStart(bool newCheckSystemPackagesAtStart);

    static TraceManager::Level traceLevel();
    static void setTraceLevel(TraceManager::Level newTraceLevel);

    static VisualTheme visualTheme();
    static void setVisualTheme(VisualTheme newVisualTheme);
    static std::string visualThemeName();

    static InterfaceStyle interfaceStyle();
    static void setInterfaceStyle(InterfaceStyle newInterfaceStyle);
    static std::string interfaceStyleName();

    static int applicationFontPointSize();
    static void setApplicationFontPointSize(int newApplicationFontPointSize);

    static bool diagramUsesThemeColors();
    static void setDiagramUsesThemeColors(bool newDiagramUsesThemeColors);

    static unsigned int canvasBackgroundColor();
    static unsigned int canvasDisabledBackgroundColor();
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
    static TraceManager::Level traceLevelFromInt(int value);

    static bool _startMaximized;
    static bool _autoLoadPlugins;
    static bool _checkSystemPackagesAtStart;
    static StartupModelMode _startupModelMode;
    static std::string _modelfilenameToOpen;
    static std::string _lastModelFilename;
    static TraceManager::Level _traceLevel;
    static VisualTheme _visualTheme;
    static InterfaceStyle _interfaceStyle;
    static int _applicationFontPointSize;
    static bool _diagramUsesThemeColors;
};

#endif // SYSTEMPREFERENCES_H
