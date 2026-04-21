QT += core gui printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++2b strict_c++

SOURCE_ROOT = $$clean_path($$PWD/../../../..)
GUI_ROOT = $$PWD

INCLUDEPATH += \
    $$SOURCE_ROOT \
    $$GUI_ROOT \
    $$GUI_ROOT/codeeditor \
    $$GUI_ROOT/propertyeditor \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser

DEPENDPATH += $$INCLUDEPATH

# Enables temporary GUI diagnostic debug symbols and frame pointers only in debug builds.
debug {
    CONFIG += force_debug_info
    QMAKE_CFLAGS_DEBUG += -g3 -O0 -fno-omit-frame-pointer
    QMAKE_CXXFLAGS_DEBUG += -g3 -O0 -fno-omit-frame-pointer
    QMAKE_LFLAGS_DEBUG += -rdynamic
}

# Enables optional ASan/UBSan instrumentation for GUI diagnostics when explicitly requested.
gui_diagnostics:debug {
    QMAKE_CFLAGS_DEBUG += -fsanitize=address,undefined
    QMAKE_CXXFLAGS_DEBUG += -fsanitize=address,undefined
    QMAKE_LFLAGS_DEBUG += -fsanitize=address,undefined
}


# Remova o pacote padrão de warnings do qmake
CONFIG -= warn_on
# Recrie manualmente o conjunto de warnings, deixando o -Wno no final
#QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-parameter -Wno-error=unused-parameter
#QMAKE_CXXFLAGS += -pedantic -Wno-unused -Wmissing-field-initializers
# Silenciar "unused parameter" de forma global
#QMAKE_CXXFLAGS += -Wno-unused-parameter
# Se o projeto trata warnings como erro em algum kit:
#QMAKE_CXXFLAGS += -Wno-error=unused-parameter
# Específico por compilador (opcional, mas robusto):
#QMAKE_CXXFLAGS_CLANG += -Wno-unused-parameter -Wno-error=unused-parameter
#QMAKE_CXXFLAGS_GCC   += -Wno-unused-parameter -Wno-error=unused-parameter

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$files($$GUI_ROOT/*.cpp, true) \
    $$files($$SOURCE_ROOT/kernel/simulator/*.cpp, false) \
    $$files($$SOURCE_ROOT/kernel/statistics/*.cpp, false) \
    $$SOURCE_ROOT/kernel/util/Util.cpp \
    $$files($$SOURCE_ROOT/parser/*.cpp, false) \
    $$files($$SOURCE_ROOT/plugins/components/*.cpp, true) \
    $$files($$SOURCE_ROOT/plugins/data/*.cpp, true) \
    $$SOURCE_ROOT/plugins/PluginConnectorStaticImpl1.cpp \
    $$SOURCE_ROOT/plugins/PluginConnectorDummyImpl1.cpp \
    $$SOURCE_ROOT/tools/FitterDummyImpl.cpp \
    $$SOURCE_ROOT/tools/HypothesisTesterDefaultImpl1.cpp \
    $$SOURCE_ROOT/tools/OptimizerDefaultImpl1.cpp \
    $$SOURCE_ROOT/tools/ProbabilityDistribution.cpp \
    $$SOURCE_ROOT/tools/ProbabilityDistributionBase.cpp \
    $$SOURCE_ROOT/tools/SimulationResultsDataset.cpp \
    $$SOURCE_ROOT/tools/SolverDefaultImpl1.cpp \
    $$SOURCE_ROOT/tools/FactorialDesign/FactorialDesign.cpp \
    $$SOURCE_ROOT/applications/BaseGenesysTerminalApplication.cpp \
    $$SOURCE_ROOT/applications/terminal/GenesysShell/GenesysShell.cpp

# Keep this source set aligned with source/applications/gui/qt/GenesysQtGUI/CMakeLists.txt.
SOURCES -= \
    $$GUI_ROOT/qcustomplot.cpp \
    $$files($$GUI_ROOT/build/*.cpp, true) \
    $$SOURCE_ROOT/kernel/simulator/PluginConnectorDummyBootstrap.cpp \
    $$SOURCE_ROOT/tools/main.cpp

HEADERS += \
    $$files($$GUI_ROOT/*.h, true) \
    $$files($$SOURCE_ROOT/kernel/*.h, true) \
    $$files($$SOURCE_ROOT/parser/*.h, false) \
    $$files($$SOURCE_ROOT/parser/*.hh, false) \
    $$files($$SOURCE_ROOT/plugins/*.h, false) \
    $$files($$SOURCE_ROOT/plugins/components/*.h, true) \
    $$files($$SOURCE_ROOT/plugins/data/*.h, true) \
    $$files($$SOURCE_ROOT/tools/*.h, true) \
    $$SOURCE_ROOT/applications/BaseGenesysTerminalApplication.h \
    $$SOURCE_ROOT/applications/GenesysApplication_if.h \
    $$SOURCE_ROOT/applications/TraitsApp.h \
    $$SOURCE_ROOT/applications/terminal/GenesysShell/GenesysShell.h \
    $$SOURCE_ROOT/applications/terminal/GenesysShell/GenesysShell_if.h \
    $$SOURCE_ROOT/applications/terminal/TraitsTerminalApp.h \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtAbstractEditorFactoryBase \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtAbstractPropertyBrowser \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtAbstractPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtBoolPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtBrowserItem \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtButtonPropertyBrowser \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtCharEditorFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtCharPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtCheckBoxFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtColorEditorFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtColorPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtCursorEditorFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtCursorPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtDateEditFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtDatePropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtDateTimeEditFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtDateTimePropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtDoublePropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtDoubleSpinBoxFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtEnumEditorFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtEnumPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtFlagPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtFontEditorFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtFontPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtGroupBoxPropertyBrowser \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtGroupPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtIntPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtKeySequenceEditorFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtKeySequencePropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtLineEditFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtLocalePropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtPointFPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtPointPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtProperty \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtRectFPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtRectPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtScrollBarFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtSizeFPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtSizePolicyPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtSizePropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtSliderFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtSpinBoxFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtStringPropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtTimeEditFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtTimePropertyManager \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtTreePropertyBrowser \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtVariantEditorFactory \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtVariantProperty \
    $$GUI_ROOT/propertyeditor/qtpropertybrowser/QtVariantPropertyManager

HEADERS -= \
    $$GUI_ROOT/qcustomplot.h \
    $$files($$GUI_ROOT/build/*.h, true)

FORMS += \
    dialogs/DialogTimerConfigure.ui \
    dialogs/Dialogmodelinformation.ui \
    dialogs/dialogBreakpoint.ui \
    dialogs/dialogpluginmanager.ui \
    dialogs/dialogsimulationconfigure.ui \
    dialogs/dialogsystempreferences.ui \
    mainwindow.ui

TRANSLATIONS += \
    GenesysQtGUI_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../../../tools/README_tools.md \
    ../../../terminal/examples/arenaSmarts/Arrivals Element Stops Entities Arriving After a Set Time Modificado.doe \
    propertyeditor/qtpropertybrowser/CMakeLists.txt \
    propertyeditor/qtpropertybrowser/images/cursor-arrow.png \
    propertyeditor/qtpropertybrowser/images/cursor-busy.png \
    propertyeditor/qtpropertybrowser/images/cursor-closedhand.png \
    propertyeditor/qtpropertybrowser/images/cursor-cross.png \
    propertyeditor/qtpropertybrowser/images/cursor-forbidden.png \
    propertyeditor/qtpropertybrowser/images/cursor-hand.png \
    propertyeditor/qtpropertybrowser/images/cursor-hsplit.png \
    propertyeditor/qtpropertybrowser/images/cursor-ibeam.png \
    propertyeditor/qtpropertybrowser/images/cursor-openhand.png \
    propertyeditor/qtpropertybrowser/images/cursor-sizeall.png \
    propertyeditor/qtpropertybrowser/images/cursor-sizeb.png \
    propertyeditor/qtpropertybrowser/images/cursor-sizef.png \
    propertyeditor/qtpropertybrowser/images/cursor-sizeh.png \
    propertyeditor/qtpropertybrowser/images/cursor-sizev.png \
    propertyeditor/qtpropertybrowser/images/cursor-uparrow.png \
    propertyeditor/qtpropertybrowser/images/cursor-vsplit.png \
    propertyeditor/qtpropertybrowser/images/cursor-wait.png \
    propertyeditor/qtpropertybrowser/images/cursor-whatsthis.png

RESOURCES += \
    GenesysQtGUI_resources.qrc \
    propertyeditor/qtpropertybrowser/qtpropertybrowser.qrc
