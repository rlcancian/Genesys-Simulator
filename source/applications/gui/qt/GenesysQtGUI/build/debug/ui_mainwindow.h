/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "codeeditor/CodeEditor.h"
#include "graphicals/ModelGraphicsView.h"
#include "propertyeditor/ObjectPropertyBrowser.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionModelNew;
    QAction *actionModelOpen;
    QAction *actionModelSave;
    QAction *actionSimulatorExit;
    QAction *actionSimulationStart;
    QAction *actionSimulationStep;
    QAction *actionSimulationStop;
    QAction *actionSimulationPause;
    QAction *actionSimulationResume;
    QAction *actionModelClose;
    QAction *actionModelCheck;
    QAction *actionAboutAbout;
    QAction *actionAboutLicence;
    QAction *actionAboutGetInvolved;
    QAction *actionShowGrid;
    QAction *actionShowSnap;
    QAction *actionEditUndo;
    QAction *actionEditRedo;
    QAction *actionEditFind;
    QAction *actionReplace;
    QAction *actionEditCut;
    QAction *actionEditCopy;
    QAction *actionEditPaste;
    QAction *actionShowRule;
    QAction *actionShowGuides;
    QAction *actionZoom_In;
    QAction *actionZoom_Out;
    QAction *actionZoom_All;
    QAction *actionDrawLine;
    QAction *actionDrawRectangle;
    QAction *actionDrawEllipse;
    QAction *actionAnimateSimulatedTime;
    QAction *actionAnimateVariable;
    QAction *actionAnimateExpression;
    QAction *actionAnimateResource;
    QAction *actionAnimateQueue;
    QAction *actionAnimateStation;
    QAction *actionGModelShowConnect;
    QAction *actionGModelComponentBreakpoint;
    QAction *actionArranje;
    QAction *actionSimulatorPreferences;
    QAction *actionEditDelete;
    QAction *actionArranjeLeft;
    QAction *actionArranjeCenter;
    QAction *actionArranjeRight;
    QAction *actionArranjeTop;
    QAction *actionArranjeMiddle;
    QAction *actionArranjeBototm;
    QAction *actionShowInternalElements;
    QAction *actionShowAttachedElements;
    QAction *actionSimulationConfigure;
    QAction *actionDrawText;
    QAction *actionDrawPoligon;
    QAction *actionAnimateCounter;
    QAction *actionAnimateEntity;
    QAction *actionAnimateEvent;
    QAction *actionAnimateAttribute;
    QAction *actionAnimateStatistics;
    QAction *actionViewConfigure;
    QAction *actionSimulatorsPluginManager;
    QAction *actionModelInformation;
    QAction *actionToolsDataAnalyzer;
    QAction *actionToolsOptimizator;
    QAction *actionToolsParserGrammarChecker;
    QAction *actionViewGroup;
    QAction *actionViewUngroup;
    QAction *actionAnimatePlot;
    QAction *actionDiagrams;
    QAction *actionActivateGraphicalSimulation;
    QAction *actionSelectAll;
    QAction *actionParallelization;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_7;
    QSplitter *splitter;
    ModelGraphicsView *graphicsView;
    QTabWidget *tabWidgetCentral;
    QWidget *tabCentralModel;
    QVBoxLayout *verticalLayout_12;
    QTabWidget *tabWidgetModel;
    QWidget *tabModelSimLanguage;
    QVBoxLayout *verticalLayout_15;
    CodeEditor *TextCodeEditor;
    QWidget *tabModelCpp;
    QVBoxLayout *verticalLayout_13;
    CodeEditor *plainTextEditCppCode;
    QWidget *tabModelDiagram;
    QVBoxLayout *verticalLayout_4;
    QFrame *frame;
    QVBoxLayout *verticalLayout_11;
    QScrollArea *scrollArea_Graphic;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_5;
    QLabel *label_ModelGraphic;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBox_ShowElements;
    QCheckBox *checkBox_ShowInternals;
    QCheckBox *checkBox_ShowLevels;
    QCheckBox *checkBox_ShowRecursive;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_Export;
    QWidget *tabModelComponents;
    QVBoxLayout *verticalLayout_3;
    QTreeWidget *treeWidgetComponents;
    QWidget *tabModelDataDefinitions;
    QVBoxLayout *verticalLayout_2;
    QTreeWidget *treeWidgetDataDefnitions;
    QWidget *tabCentralSimulation;
    QVBoxLayout *verticalLayout_6;
    QTabWidget *tabWidgetSimulation;
    QWidget *tabDebugBreakpoints;
    QVBoxLayout *verticalLayout_8;
    QTableWidget *tableWidget_Breakpoints;
    QWidget *tabDebugVariables;
    QVBoxLayout *verticalLayout_14;
    QTableWidget *tableWidget_Variables;
    QWidget *tabDebugEntities;
    QVBoxLayout *verticalLayout_16;
    QTableWidget *tableWidget_Entities;
    QWidget *tabDebugTraces;
    QVBoxLayout *verticalLayout_17;
    QTextEdit *textEdit_Simulation;
    QWidget *tabDebugEvents;
    QVBoxLayout *verticalLayout_9;
    QTableWidget *tableWidget_Simulation_Event;
    QWidget *tabCentralWidget;
    QVBoxLayout *verticalLayout_10;
    QTabWidget *tabWidgetReports;
    QWidget *tabReportsReports;
    QVBoxLayout *verticalLayout_18;
    QTextEdit *textEdit_Reports;
    QWidget *TabReportsResults;
    QWidget *tabReportsPlots;
    QHBoxLayout *horizontalLayoutZoomReplication;
    QLabel *labelMousePos;
    QFrame *line;
    QLabel *label_3;
    QLabel *label_ZoomValue;
    QSlider *horizontalSlider_ZoomGraphical;
    QFrame *line_2;
    QLabel *label_2;
    QLabel *label_ReplicationNum;
    QLabel *label;
    QProgressBar *progressBarSimulation;
    QFrame *line_3;
    QLabel *labelAnimationSpeed;
    QSlider *horizontalSliderAnimationSpeed;
    QMenuBar *menubar;
    QMenu *menuModel;
    QMenu *menuSimulation;
    QMenu *menuAbout;
    QMenu *menuView;
    QMenu *menuZoom;
    QMenu *menuViewDraw;
    QMenu *menuViewAnimate;
    QMenu *menuViewArranje;
    QMenu *menuViewShow;
    QMenu *menuEdit;
    QMenu *menuTools;
    QMenu *menuSimulator;
    QStatusBar *statusbar;
    QToolBar *toolBarModel;
    QDockWidget *dockWidgetConsole;
    QWidget *dockWidgetContentsConsole;
    QVBoxLayout *verticalLayout;
    QTextEdit *textEdit_Console;
    QDockWidget *dockWidgetPlugins;
    QWidget *dockWidgetContentsPlugin;
    QHBoxLayout *horizontalLayout_2;
    QTreeWidget *treeWidget_Plugins;
    QDockWidget *dockWidgetPropertyEditor;
    QWidget *dockWidgetContents;
    QHBoxLayout *horizontalLayout_4;
    ObjectPropertyBrowser *treeViewPropertyEditor;
    QToolBar *toolBarGraphicalModel;
    QToolBar *toolBarEdit;
    QToolBar *toolBarView;
    QToolBar *toolBarSimulation;
    QToolBar *toolBarAbout;
    QToolBar *toolBarDraw;
    QToolBar *toolBarAnimate;
    QToolBar *toolBarArranje;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1081, 777);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resources/icons/genesysico.gif"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        MainWindow->setDockNestingEnabled(true);
        actionModelNew = new QAction(MainWindow);
        actionModelNew->setObjectName("actionModelNew");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/new document.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionModelNew->setIcon(icon1);
        actionModelOpen = new QAction(MainWindow);
        actionModelOpen->setObjectName("actionModelOpen");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/upload.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionModelOpen->setIcon(icon2);
        actionModelSave = new QAction(MainWindow);
        actionModelSave->setObjectName("actionModelSave");
        actionModelSave->setEnabled(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/download.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionModelSave->setIcon(icon3);
        actionSimulatorExit = new QAction(MainWindow);
        actionSimulatorExit->setObjectName("actionSimulatorExit");
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/exit.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionSimulatorExit->setIcon(icon4);
        actionSimulationStart = new QAction(MainWindow);
        actionSimulationStart->setObjectName("actionSimulationStart");
        actionSimulationStart->setEnabled(true);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/play.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionSimulationStart->setIcon(icon5);
        actionSimulationStep = new QAction(MainWindow);
        actionSimulationStep->setObjectName("actionSimulationStep");
        actionSimulationStep->setEnabled(false);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/next track.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionSimulationStep->setIcon(icon6);
        actionSimulationStop = new QAction(MainWindow);
        actionSimulationStop->setObjectName("actionSimulationStop");
        actionSimulationStop->setEnabled(false);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/stop.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionSimulationStop->setIcon(icon7);
        actionSimulationPause = new QAction(MainWindow);
        actionSimulationPause->setObjectName("actionSimulationPause");
        actionSimulationPause->setEnabled(false);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/pause.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionSimulationPause->setIcon(icon8);
        actionSimulationResume = new QAction(MainWindow);
        actionSimulationResume->setObjectName("actionSimulationResume");
        actionSimulationResume->setEnabled(false);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/fast-forward.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionSimulationResume->setIcon(icon9);
        actionModelClose = new QAction(MainWindow);
        actionModelClose->setObjectName("actionModelClose");
        actionModelClose->setEnabled(false);
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/close.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionModelClose->setIcon(icon10);
        actionModelCheck = new QAction(MainWindow);
        actionModelCheck->setObjectName("actionModelCheck");
        actionModelCheck->setEnabled(false);
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/yes.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionModelCheck->setIcon(icon11);
        actionAboutAbout = new QAction(MainWindow);
        actionAboutAbout->setObjectName("actionAboutAbout");
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/icons1/resources/icons/pack1/ico/106.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAboutAbout->setIcon(icon12);
        actionAboutLicence = new QAction(MainWindow);
        actionAboutLicence->setObjectName("actionAboutLicence");
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/icons2/resources/icons/pack2/ico/28.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAboutLicence->setIcon(icon13);
        actionAboutGetInvolved = new QAction(MainWindow);
        actionAboutGetInvolved->setObjectName("actionAboutGetInvolved");
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/user group.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAboutGetInvolved->setIcon(icon14);
        actionShowGrid = new QAction(MainWindow);
        actionShowGrid->setObjectName("actionShowGrid");
        actionShowGrid->setCheckable(true);
        actionShowGrid->setEnabled(true);
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/resources/ToolBar/_grade.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        actionShowGrid->setIcon(icon15);
        actionShowSnap = new QAction(MainWindow);
        actionShowSnap->setObjectName("actionShowSnap");
        actionShowSnap->setCheckable(true);
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/question.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionShowSnap->setIcon(icon16);
        actionEditUndo = new QAction(MainWindow);
        actionEditUndo->setObjectName("actionEditUndo");
        actionEditUndo->setCheckable(false);
        actionEditUndo->setEnabled(false);
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/undo.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionEditUndo->setIcon(icon17);
        actionEditUndo->setVisible(true);
        actionEditRedo = new QAction(MainWindow);
        actionEditRedo->setObjectName("actionEditRedo");
        actionEditRedo->setEnabled(false);
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/redo.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionEditRedo->setIcon(icon18);
        actionEditFind = new QAction(MainWindow);
        actionEditFind->setObjectName("actionEditFind");
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/find.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionEditFind->setIcon(icon19);
        actionReplace = new QAction(MainWindow);
        actionReplace->setObjectName("actionReplace");
        actionReplace->setIcon(icon19);
        actionEditCut = new QAction(MainWindow);
        actionEditCut->setObjectName("actionEditCut");
        actionEditCut->setEnabled(false);
        QIcon icon20;
        icon20.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/cut.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionEditCut->setIcon(icon20);
        actionEditCopy = new QAction(MainWindow);
        actionEditCopy->setObjectName("actionEditCopy");
        actionEditCopy->setEnabled(false);
        QIcon icon21;
        icon21.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/copy.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionEditCopy->setIcon(icon21);
        actionEditPaste = new QAction(MainWindow);
        actionEditPaste->setObjectName("actionEditPaste");
        actionEditPaste->setEnabled(false);
        QIcon icon22;
        icon22.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/paste.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionEditPaste->setIcon(icon22);
        actionShowRule = new QAction(MainWindow);
        actionShowRule->setObjectName("actionShowRule");
        actionShowRule->setCheckable(true);
        QIcon icon23;
        icon23.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/rule.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon23.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/rule.png"), QSize(), QIcon::Normal, QIcon::On);
        actionShowRule->setIcon(icon23);
        actionShowGuides = new QAction(MainWindow);
        actionShowGuides->setObjectName("actionShowGuides");
        actionShowGuides->setCheckable(true);
        actionShowGuides->setIcon(icon16);
        actionZoom_In = new QAction(MainWindow);
        actionZoom_In->setObjectName("actionZoom_In");
        QIcon icon24;
        icon24.addFile(QString::fromUtf8(":/icons2/resources/icons/pack2/ico/83.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_In->setIcon(icon24);
        actionZoom_Out = new QAction(MainWindow);
        actionZoom_Out->setObjectName("actionZoom_Out");
        QIcon icon25;
        icon25.addFile(QString::fromUtf8(":/icons2/resources/icons/pack2/ico/82.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_Out->setIcon(icon25);
        actionZoom_All = new QAction(MainWindow);
        actionZoom_All->setObjectName("actionZoom_All");
        QIcon icon26;
        icon26.addFile(QString::fromUtf8(":/icons2/resources/icons/pack2/ico/84.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_All->setIcon(icon26);
        actionDrawLine = new QAction(MainWindow);
        actionDrawLine->setObjectName("actionDrawLine");
        actionDrawLine->setCheckable(true);
        QIcon icon27;
        icon27.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/hline.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDrawLine->setIcon(icon27);
        actionDrawRectangle = new QAction(MainWindow);
        actionDrawRectangle->setObjectName("actionDrawRectangle");
        actionDrawRectangle->setCheckable(true);
        QIcon icon28;
        icon28.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/rectangle.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDrawRectangle->setIcon(icon28);
        actionDrawEllipse = new QAction(MainWindow);
        actionDrawEllipse->setObjectName("actionDrawEllipse");
        actionDrawEllipse->setCheckable(true);
        QIcon icon29;
        icon29.addFile(QString::fromUtf8(":/resources/ToolBar/_elipse.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        actionDrawEllipse->setIcon(icon29);
        actionAnimateSimulatedTime = new QAction(MainWindow);
        actionAnimateSimulatedTime->setObjectName("actionAnimateSimulatedTime");
        actionAnimateSimulatedTime->setCheckable(true);
        QIcon icon30;
        icon30.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/alarm clock.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimateSimulatedTime->setIcon(icon30);
        actionAnimateVariable = new QAction(MainWindow);
        actionAnimateVariable->setObjectName("actionAnimateVariable");
        actionAnimateVariable->setCheckable(true);
        QIcon icon31;
        icon31.addFile(QString::fromUtf8(":/resources/ToolBar/_animvalor.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimateVariable->setIcon(icon31);
        actionAnimateExpression = new QAction(MainWindow);
        actionAnimateExpression->setObjectName("actionAnimateExpression");
        actionAnimateExpression->setIcon(icon31);
        actionAnimateResource = new QAction(MainWindow);
        actionAnimateResource->setObjectName("actionAnimateResource");
        actionAnimateResource->setIcon(icon16);
        actionAnimateQueue = new QAction(MainWindow);
        actionAnimateQueue->setObjectName("actionAnimateQueue");
        QIcon icon32;
        icon32.addFile(QString::fromUtf8(":/resources/ToolBar/_animfila.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimateQueue->setIcon(icon32);
        actionAnimateStation = new QAction(MainWindow);
        actionAnimateStation->setObjectName("actionAnimateStation");
        actionAnimateStation->setIcon(icon16);
        actionGModelShowConnect = new QAction(MainWindow);
        actionGModelShowConnect->setObjectName("actionGModelShowConnect");
        actionGModelShowConnect->setCheckable(true);
        QIcon icon33;
        icon33.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/vertical.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionGModelShowConnect->setIcon(icon33);
        actionGModelComponentBreakpoint = new QAction(MainWindow);
        actionGModelComponentBreakpoint->setObjectName("actionGModelComponentBreakpoint");
        QIcon icon34;
        icon34.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/abort.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionGModelComponentBreakpoint->setIcon(icon34);
        actionArranje = new QAction(MainWindow);
        actionArranje->setObjectName("actionArranje");
        actionSimulatorPreferences = new QAction(MainWindow);
        actionSimulatorPreferences->setObjectName("actionSimulatorPreferences");
        QIcon icon35;
        icon35.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/preferences.gif"), QSize(), QIcon::Normal, QIcon::On);
        actionSimulatorPreferences->setIcon(icon35);
        actionEditDelete = new QAction(MainWindow);
        actionEditDelete->setObjectName("actionEditDelete");
        QIcon icon36;
        QString iconThemeName = QString::fromUtf8("edit-delete");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon36 = QIcon::fromTheme(iconThemeName);
        } else {
            icon36.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionEditDelete->setIcon(icon36);
        actionArranjeLeft = new QAction(MainWindow);
        actionArranjeLeft->setObjectName("actionArranjeLeft");
        QIcon icon37;
        icon37.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/alignleft.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionArranjeLeft->setIcon(icon37);
        actionArranjeCenter = new QAction(MainWindow);
        actionArranjeCenter->setObjectName("actionArranjeCenter");
        QIcon icon38;
        icon38.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/aligncenter.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionArranjeCenter->setIcon(icon38);
        actionArranjeRight = new QAction(MainWindow);
        actionArranjeRight->setObjectName("actionArranjeRight");
        QIcon icon39;
        icon39.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/alignright.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionArranjeRight->setIcon(icon39);
        actionArranjeTop = new QAction(MainWindow);
        actionArranjeTop->setObjectName("actionArranjeTop");
        QIcon icon40;
        icon40.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/aligntop.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionArranjeTop->setIcon(icon40);
        actionArranjeMiddle = new QAction(MainWindow);
        actionArranjeMiddle->setObjectName("actionArranjeMiddle");
        QIcon icon41;
        icon41.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/alignmiddle.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionArranjeMiddle->setIcon(icon41);
        actionArranjeBototm = new QAction(MainWindow);
        actionArranjeBototm->setObjectName("actionArranjeBototm");
        QIcon icon42;
        icon42.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/alignbottom.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionArranjeBototm->setIcon(icon42);
        actionShowInternalElements = new QAction(MainWindow);
        actionShowInternalElements->setObjectName("actionShowInternalElements");
        actionShowInternalElements->setCheckable(true);
        QIcon icon43;
        icon43.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/red pin.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionShowInternalElements->setIcon(icon43);
        actionShowAttachedElements = new QAction(MainWindow);
        actionShowAttachedElements->setObjectName("actionShowAttachedElements");
        actionShowAttachedElements->setCheckable(true);
        QIcon icon44;
        icon44.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/yellow pin.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionShowAttachedElements->setIcon(icon44);
        actionSimulationConfigure = new QAction(MainWindow);
        actionSimulationConfigure->setObjectName("actionSimulationConfigure");
        actionSimulationConfigure->setEnabled(true);
        QIcon icon45;
        icon45.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/pinion.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionSimulationConfigure->setIcon(icon45);
        actionSimulationConfigure->setVisible(true);
        actionDrawText = new QAction(MainWindow);
        actionDrawText->setObjectName("actionDrawText");
        actionDrawText->setCheckable(true);
        QIcon icon46;
        icon46.addFile(QString::fromUtf8(":/icons1/resources/icons/pack1/ico/190.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionDrawText->setIcon(icon46);
        actionDrawPoligon = new QAction(MainWindow);
        actionDrawPoligon->setObjectName("actionDrawPoligon");
        actionDrawPoligon->setCheckable(true);
        QIcon icon47;
        icon47.addFile(QString::fromUtf8(":/resources/ToolBar/_retang.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        actionDrawPoligon->setIcon(icon47);
        actionAnimateCounter = new QAction(MainWindow);
        actionAnimateCounter->setObjectName("actionAnimateCounter");
        actionAnimateCounter->setCheckable(true);
        QIcon icon48;
        icon48.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/coin.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimateCounter->setIcon(icon48);
        actionAnimateEntity = new QAction(MainWindow);
        actionAnimateEntity->setObjectName("actionAnimateEntity");
        QIcon icon49;
        icon49.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/person.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimateEntity->setIcon(icon49);
        actionAnimateEvent = new QAction(MainWindow);
        actionAnimateEvent->setObjectName("actionAnimateEvent");
        QIcon icon50;
        icon50.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/calendar.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimateEvent->setIcon(icon50);
        actionAnimateAttribute = new QAction(MainWindow);
        actionAnimateAttribute->setObjectName("actionAnimateAttribute");
        QIcon icon51;
        icon51.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/sum.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimateAttribute->setIcon(icon51);
        actionAnimateStatistics = new QAction(MainWindow);
        actionAnimateStatistics->setObjectName("actionAnimateStatistics");
        QIcon icon52;
        icon52.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/pie chart.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimateStatistics->setIcon(icon52);
        actionViewConfigure = new QAction(MainWindow);
        actionViewConfigure->setObjectName("actionViewConfigure");
        QIcon icon53;
        icon53.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/picture.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionViewConfigure->setIcon(icon53);
        actionSimulatorsPluginManager = new QAction(MainWindow);
        actionSimulatorsPluginManager->setObjectName("actionSimulatorsPluginManager");
        QIcon icon54;
        icon54.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/componentblue.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionSimulatorsPluginManager->setIcon(icon54);
        actionModelInformation = new QAction(MainWindow);
        actionModelInformation->setObjectName("actionModelInformation");
        QIcon icon55;
        iconThemeName = QString::fromUtf8("user-available");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon55 = QIcon::fromTheme(iconThemeName);
        } else {
            icon55.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionModelInformation->setIcon(icon55);
        actionToolsDataAnalyzer = new QAction(MainWindow);
        actionToolsDataAnalyzer->setObjectName("actionToolsDataAnalyzer");
        QIcon icon56;
        icon56.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/diagram.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionToolsDataAnalyzer->setIcon(icon56);
        actionToolsOptimizator = new QAction(MainWindow);
        actionToolsOptimizator->setObjectName("actionToolsOptimizator");
        QIcon icon57;
        icon57.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/lightning.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionToolsOptimizator->setIcon(icon57);
        actionToolsParserGrammarChecker = new QAction(MainWindow);
        actionToolsParserGrammarChecker->setObjectName("actionToolsParserGrammarChecker");
        QIcon icon58;
        icon58.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/percent.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionToolsParserGrammarChecker->setIcon(icon58);
        actionViewGroup = new QAction(MainWindow);
        actionViewGroup->setObjectName("actionViewGroup");
        QIcon icon59;
        icon59.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/group.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionViewGroup->setIcon(icon59);
        actionViewUngroup = new QAction(MainWindow);
        actionViewUngroup->setObjectName("actionViewUngroup");
        QIcon icon60;
        icon60.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/ungroup.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionViewUngroup->setIcon(icon60);
        actionAnimatePlot = new QAction(MainWindow);
        actionAnimatePlot->setObjectName("actionAnimatePlot");
        QIcon icon61;
        icon61.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/3d bar chart.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnimatePlot->setIcon(icon61);
        actionDiagrams = new QAction(MainWindow);
        actionDiagrams->setObjectName("actionDiagrams");
        actionDiagrams->setCheckable(true);
        QIcon icon62;
        iconThemeName = QString::fromUtf8("window-new");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon62 = QIcon::fromTheme(iconThemeName);
        } else {
            icon62.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionDiagrams->setIcon(icon62);
        actionActivateGraphicalSimulation = new QAction(MainWindow);
        actionActivateGraphicalSimulation->setObjectName("actionActivateGraphicalSimulation");
        actionActivateGraphicalSimulation->setCheckable(true);
        QIcon icon63;
        icon63.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/eye.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionActivateGraphicalSimulation->setIcon(icon63);
        actionSelectAll = new QAction(MainWindow);
        actionSelectAll->setObjectName("actionSelectAll");
        actionSelectAll->setCheckable(false);
        QIcon icon64;
        icon64.addFile(QString::fromUtf8(":/mxgraph/resources/icons/pack4/iconsMxGraph/select.gif"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelectAll->setIcon(icon64);
        actionParallelization = new QAction(MainWindow);
        actionParallelization->setObjectName("actionParallelization");
        QIcon icon65;
        icon65.addFile(QString::fromUtf8(":/icons1/resources/icons/pack1/ico/200.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionParallelization->setIcon(icon65);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_7 = new QVBoxLayout(centralwidget);
        verticalLayout_7->setObjectName("verticalLayout_7");
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName("splitter");
        splitter->setOrientation(Qt::Vertical);
        graphicsView = new ModelGraphicsView(splitter);
        graphicsView->setObjectName("graphicsView");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(4);
        sizePolicy.setVerticalStretch(4);
        sizePolicy.setHeightForWidth(graphicsView->sizePolicy().hasHeightForWidth());
        graphicsView->setSizePolicy(sizePolicy);
        graphicsView->setMinimumSize(QSize(0, 250));
        graphicsView->setFrameShape(QFrame::Box);
        graphicsView->setFrameShadow(QFrame::Sunken);
        graphicsView->setLineWidth(3);
        graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        splitter->addWidget(graphicsView);
        tabWidgetCentral = new QTabWidget(splitter);
        tabWidgetCentral->setObjectName("tabWidgetCentral");
        QFont font;
        font.setPointSize(10);
        tabWidgetCentral->setFont(font);
        tabWidgetCentral->setTabPosition(QTabWidget::West);
        tabWidgetCentral->setTabShape(QTabWidget::Triangular);
        tabWidgetCentral->setTabBarAutoHide(true);
        tabCentralModel = new QWidget();
        tabCentralModel->setObjectName("tabCentralModel");
        verticalLayout_12 = new QVBoxLayout(tabCentralModel);
        verticalLayout_12->setObjectName("verticalLayout_12");
        tabWidgetModel = new QTabWidget(tabCentralModel);
        tabWidgetModel->setObjectName("tabWidgetModel");
        tabWidgetModel->setTabPosition(QTabWidget::North);
        tabWidgetModel->setTabShape(QTabWidget::Triangular);
        tabWidgetModel->setTabBarAutoHide(true);
        tabModelSimLanguage = new QWidget();
        tabModelSimLanguage->setObjectName("tabModelSimLanguage");
        verticalLayout_15 = new QVBoxLayout(tabModelSimLanguage);
        verticalLayout_15->setObjectName("verticalLayout_15");
        TextCodeEditor = new CodeEditor(tabModelSimLanguage);
        TextCodeEditor->setObjectName("TextCodeEditor");

        verticalLayout_15->addWidget(TextCodeEditor);

        tabWidgetModel->addTab(tabModelSimLanguage, QString());
        tabModelCpp = new QWidget();
        tabModelCpp->setObjectName("tabModelCpp");
        verticalLayout_13 = new QVBoxLayout(tabModelCpp);
        verticalLayout_13->setObjectName("verticalLayout_13");
        plainTextEditCppCode = new CodeEditor(tabModelCpp);
        plainTextEditCppCode->setObjectName("plainTextEditCppCode");
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Courier 10 Pitch")});
        font1.setPointSize(10);
        plainTextEditCppCode->setFont(font1);

        verticalLayout_13->addWidget(plainTextEditCppCode);

        tabWidgetModel->addTab(tabModelCpp, QString());
        tabModelDiagram = new QWidget();
        tabModelDiagram->setObjectName("tabModelDiagram");
        verticalLayout_4 = new QVBoxLayout(tabModelDiagram);
        verticalLayout_4->setObjectName("verticalLayout_4");
        frame = new QFrame(tabModelDiagram);
        frame->setObjectName("frame");
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_11 = new QVBoxLayout(frame);
        verticalLayout_11->setObjectName("verticalLayout_11");
        scrollArea_Graphic = new QScrollArea(frame);
        scrollArea_Graphic->setObjectName("scrollArea_Graphic");
        scrollArea_Graphic->setAutoFillBackground(false);
        scrollArea_Graphic->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scrollArea_Graphic->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 98, 36));
        scrollAreaWidgetContents->setAutoFillBackground(true);
        verticalLayout_5 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_5->setObjectName("verticalLayout_5");
        label_ModelGraphic = new QLabel(scrollAreaWidgetContents);
        label_ModelGraphic->setObjectName("label_ModelGraphic");
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(3);
        sizePolicy1.setHeightForWidth(label_ModelGraphic->sizePolicy().hasHeightForWidth());
        label_ModelGraphic->setSizePolicy(sizePolicy1);
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Window, brush);
        QBrush brush1(QColor(239, 239, 239, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
        label_ModelGraphic->setPalette(palette);
        label_ModelGraphic->setAutoFillBackground(false);
        label_ModelGraphic->setFrameShadow(QFrame::Plain);
        label_ModelGraphic->setAlignment(Qt::AlignCenter);

        verticalLayout_5->addWidget(label_ModelGraphic);

        scrollArea_Graphic->setWidget(scrollAreaWidgetContents);

        verticalLayout_11->addWidget(scrollArea_Graphic);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        checkBox_ShowElements = new QCheckBox(frame);
        checkBox_ShowElements->setObjectName("checkBox_ShowElements");
        checkBox_ShowElements->setChecked(true);

        horizontalLayout->addWidget(checkBox_ShowElements);

        checkBox_ShowInternals = new QCheckBox(frame);
        checkBox_ShowInternals->setObjectName("checkBox_ShowInternals");
        checkBox_ShowInternals->setChecked(true);

        horizontalLayout->addWidget(checkBox_ShowInternals);

        checkBox_ShowLevels = new QCheckBox(frame);
        checkBox_ShowLevels->setObjectName("checkBox_ShowLevels");
        checkBox_ShowLevels->setChecked(true);

        horizontalLayout->addWidget(checkBox_ShowLevels);

        checkBox_ShowRecursive = new QCheckBox(frame);
        checkBox_ShowRecursive->setObjectName("checkBox_ShowRecursive");
        checkBox_ShowRecursive->setChecked(true);

        horizontalLayout->addWidget(checkBox_ShowRecursive);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton_Export = new QPushButton(frame);
        pushButton_Export->setObjectName("pushButton_Export");

        horizontalLayout->addWidget(pushButton_Export);


        verticalLayout_11->addLayout(horizontalLayout);


        verticalLayout_4->addWidget(frame);

        tabWidgetModel->addTab(tabModelDiagram, QString());
        tabModelComponents = new QWidget();
        tabModelComponents->setObjectName("tabModelComponents");
        verticalLayout_3 = new QVBoxLayout(tabModelComponents);
        verticalLayout_3->setObjectName("verticalLayout_3");
        treeWidgetComponents = new QTreeWidget(tabModelComponents);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(3, QString::fromUtf8("4"));
        __qtreewidgetitem->setText(2, QString::fromUtf8("3"));
        __qtreewidgetitem->setText(1, QString::fromUtf8("2"));
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidgetComponents->setHeaderItem(__qtreewidgetitem);
        treeWidgetComponents->setObjectName("treeWidgetComponents");
        treeWidgetComponents->setAutoScrollMargin(8);
        treeWidgetComponents->setRootIsDecorated(true);
        treeWidgetComponents->setSortingEnabled(true);
        treeWidgetComponents->setColumnCount(4);

        verticalLayout_3->addWidget(treeWidgetComponents);

        tabWidgetModel->addTab(tabModelComponents, QString());
        tabModelDataDefinitions = new QWidget();
        tabModelDataDefinitions->setObjectName("tabModelDataDefinitions");
        verticalLayout_2 = new QVBoxLayout(tabModelDataDefinitions);
        verticalLayout_2->setObjectName("verticalLayout_2");
        treeWidgetDataDefnitions = new QTreeWidget(tabModelDataDefinitions);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem();
        __qtreewidgetitem1->setText(3, QString::fromUtf8("4"));
        __qtreewidgetitem1->setText(2, QString::fromUtf8("3"));
        __qtreewidgetitem1->setText(1, QString::fromUtf8("2"));
        __qtreewidgetitem1->setText(0, QString::fromUtf8("1"));
        treeWidgetDataDefnitions->setHeaderItem(__qtreewidgetitem1);
        treeWidgetDataDefnitions->setObjectName("treeWidgetDataDefnitions");
        treeWidgetDataDefnitions->setAutoScrollMargin(8);
        treeWidgetDataDefnitions->setRootIsDecorated(true);
        treeWidgetDataDefnitions->setSortingEnabled(true);
        treeWidgetDataDefnitions->setColumnCount(4);

        verticalLayout_2->addWidget(treeWidgetDataDefnitions);

        tabWidgetModel->addTab(tabModelDataDefinitions, QString());

        verticalLayout_12->addWidget(tabWidgetModel);

        tabWidgetCentral->addTab(tabCentralModel, QString());
        tabCentralSimulation = new QWidget();
        tabCentralSimulation->setObjectName("tabCentralSimulation");
        verticalLayout_6 = new QVBoxLayout(tabCentralSimulation);
        verticalLayout_6->setObjectName("verticalLayout_6");
        tabWidgetSimulation = new QTabWidget(tabCentralSimulation);
        tabWidgetSimulation->setObjectName("tabWidgetSimulation");
        tabWidgetSimulation->setTabPosition(QTabWidget::North);
        tabWidgetSimulation->setTabShape(QTabWidget::Triangular);
        tabWidgetSimulation->setTabBarAutoHide(true);
        tabDebugBreakpoints = new QWidget();
        tabDebugBreakpoints->setObjectName("tabDebugBreakpoints");
        verticalLayout_8 = new QVBoxLayout(tabDebugBreakpoints);
        verticalLayout_8->setObjectName("verticalLayout_8");
        tableWidget_Breakpoints = new QTableWidget(tabDebugBreakpoints);
        if (tableWidget_Breakpoints->columnCount() < 3)
            tableWidget_Breakpoints->setColumnCount(3);
        tableWidget_Breakpoints->setObjectName("tableWidget_Breakpoints");
        tableWidget_Breakpoints->setEnabled(false);
        tableWidget_Breakpoints->setColumnCount(3);
        tableWidget_Breakpoints->horizontalHeader()->setProperty("showSortIndicator", QVariant(true));

        verticalLayout_8->addWidget(tableWidget_Breakpoints);

        tabWidgetSimulation->addTab(tabDebugBreakpoints, QString());
        tabDebugVariables = new QWidget();
        tabDebugVariables->setObjectName("tabDebugVariables");
        verticalLayout_14 = new QVBoxLayout(tabDebugVariables);
        verticalLayout_14->setObjectName("verticalLayout_14");
        tableWidget_Variables = new QTableWidget(tabDebugVariables);
        if (tableWidget_Variables->columnCount() < 3)
            tableWidget_Variables->setColumnCount(3);
        tableWidget_Variables->setObjectName("tableWidget_Variables");
        tableWidget_Variables->setEnabled(false);
        tableWidget_Variables->setColumnCount(3);
        tableWidget_Variables->horizontalHeader()->setProperty("showSortIndicator", QVariant(true));

        verticalLayout_14->addWidget(tableWidget_Variables);

        tabWidgetSimulation->addTab(tabDebugVariables, QString());
        tabDebugEntities = new QWidget();
        tabDebugEntities->setObjectName("tabDebugEntities");
        verticalLayout_16 = new QVBoxLayout(tabDebugEntities);
        verticalLayout_16->setObjectName("verticalLayout_16");
        tableWidget_Entities = new QTableWidget(tabDebugEntities);
        if (tableWidget_Entities->columnCount() < 3)
            tableWidget_Entities->setColumnCount(3);
        tableWidget_Entities->setObjectName("tableWidget_Entities");
        tableWidget_Entities->setEnabled(false);
        tableWidget_Entities->setColumnCount(3);
        tableWidget_Entities->horizontalHeader()->setProperty("showSortIndicator", QVariant(true));

        verticalLayout_16->addWidget(tableWidget_Entities);

        tabWidgetSimulation->addTab(tabDebugEntities, QString());
        tabDebugTraces = new QWidget();
        tabDebugTraces->setObjectName("tabDebugTraces");
        verticalLayout_17 = new QVBoxLayout(tabDebugTraces);
        verticalLayout_17->setObjectName("verticalLayout_17");
        textEdit_Simulation = new QTextEdit(tabDebugTraces);
        textEdit_Simulation->setObjectName("textEdit_Simulation");
        textEdit_Simulation->setFont(font);
        textEdit_Simulation->setLineWrapMode(QTextEdit::NoWrap);
        textEdit_Simulation->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_17->addWidget(textEdit_Simulation);

        tabWidgetSimulation->addTab(tabDebugTraces, QString());
        tabDebugEvents = new QWidget();
        tabDebugEvents->setObjectName("tabDebugEvents");
        verticalLayout_9 = new QVBoxLayout(tabDebugEvents);
        verticalLayout_9->setObjectName("verticalLayout_9");
        tableWidget_Simulation_Event = new QTableWidget(tabDebugEvents);
        if (tableWidget_Simulation_Event->columnCount() < 3)
            tableWidget_Simulation_Event->setColumnCount(3);
        tableWidget_Simulation_Event->setObjectName("tableWidget_Simulation_Event");
        tableWidget_Simulation_Event->setFont(font);
        tableWidget_Simulation_Event->setWordWrap(false);
        tableWidget_Simulation_Event->setRowCount(0);
        tableWidget_Simulation_Event->setColumnCount(3);

        verticalLayout_9->addWidget(tableWidget_Simulation_Event);

        tabWidgetSimulation->addTab(tabDebugEvents, QString());

        verticalLayout_6->addWidget(tabWidgetSimulation);

        tabWidgetCentral->addTab(tabCentralSimulation, QString());
        tabCentralWidget = new QWidget();
        tabCentralWidget->setObjectName("tabCentralWidget");
        verticalLayout_10 = new QVBoxLayout(tabCentralWidget);
        verticalLayout_10->setObjectName("verticalLayout_10");
        tabWidgetReports = new QTabWidget(tabCentralWidget);
        tabWidgetReports->setObjectName("tabWidgetReports");
        tabWidgetReports->setTabShape(QTabWidget::Triangular);
        tabWidgetReports->setTabBarAutoHide(true);
        tabReportsReports = new QWidget();
        tabReportsReports->setObjectName("tabReportsReports");
        verticalLayout_18 = new QVBoxLayout(tabReportsReports);
        verticalLayout_18->setObjectName("verticalLayout_18");
        textEdit_Reports = new QTextEdit(tabReportsReports);
        textEdit_Reports->setObjectName("textEdit_Reports");
        textEdit_Reports->setFont(font1);
        textEdit_Reports->setLineWrapMode(QTextEdit::NoWrap);
        textEdit_Reports->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_18->addWidget(textEdit_Reports);

        tabWidgetReports->addTab(tabReportsReports, QString());
        TabReportsResults = new QWidget();
        TabReportsResults->setObjectName("TabReportsResults");
        tabWidgetReports->addTab(TabReportsResults, QString());
        tabReportsPlots = new QWidget();
        tabReportsPlots->setObjectName("tabReportsPlots");
        tabWidgetReports->addTab(tabReportsPlots, QString());

        verticalLayout_10->addWidget(tabWidgetReports);

        tabWidgetCentral->addTab(tabCentralWidget, QString());
        splitter->addWidget(tabWidgetCentral);

        verticalLayout_7->addWidget(splitter);

        horizontalLayoutZoomReplication = new QHBoxLayout();
        horizontalLayoutZoomReplication->setSpacing(12);
        horizontalLayoutZoomReplication->setObjectName("horizontalLayoutZoomReplication");
        labelMousePos = new QLabel(centralwidget);
        labelMousePos->setObjectName("labelMousePos");
        QFont font2;
        font2.setPointSize(9);
        labelMousePos->setFont(font2);

        horizontalLayoutZoomReplication->addWidget(labelMousePos);

        line = new QFrame(centralwidget);
        line->setObjectName("line");
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayoutZoomReplication->addWidget(line);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");
        label_3->setFont(font2);

        horizontalLayoutZoomReplication->addWidget(label_3);

        label_ZoomValue = new QLabel(centralwidget);
        label_ZoomValue->setObjectName("label_ZoomValue");

        horizontalLayoutZoomReplication->addWidget(label_ZoomValue);

        horizontalSlider_ZoomGraphical = new QSlider(centralwidget);
        horizontalSlider_ZoomGraphical->setObjectName("horizontalSlider_ZoomGraphical");
        horizontalSlider_ZoomGraphical->setMinimum(1);
        horizontalSlider_ZoomGraphical->setMaximum(1000);
        horizontalSlider_ZoomGraphical->setSingleStep(1);
        horizontalSlider_ZoomGraphical->setValue(500);
        horizontalSlider_ZoomGraphical->setOrientation(Qt::Horizontal);

        horizontalLayoutZoomReplication->addWidget(horizontalSlider_ZoomGraphical);

        line_2 = new QFrame(centralwidget);
        line_2->setObjectName("line_2");
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayoutZoomReplication->addWidget(line_2);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");
        label_2->setFont(font2);

        horizontalLayoutZoomReplication->addWidget(label_2);

        label_ReplicationNum = new QLabel(centralwidget);
        label_ReplicationNum->setObjectName("label_ReplicationNum");
        label_ReplicationNum->setFont(font2);

        horizontalLayoutZoomReplication->addWidget(label_ReplicationNum);

        label = new QLabel(centralwidget);
        label->setObjectName("label");

        horizontalLayoutZoomReplication->addWidget(label);

        progressBarSimulation = new QProgressBar(centralwidget);
        progressBarSimulation->setObjectName("progressBarSimulation");
        progressBarSimulation->setFont(font2);
        progressBarSimulation->setValue(0);
        progressBarSimulation->setInvertedAppearance(false);

        horizontalLayoutZoomReplication->addWidget(progressBarSimulation);

        line_3 = new QFrame(centralwidget);
        line_3->setObjectName("line_3");
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);

        horizontalLayoutZoomReplication->addWidget(line_3);

        labelAnimationSpeed = new QLabel(centralwidget);
        labelAnimationSpeed->setObjectName("labelAnimationSpeed");

        horizontalLayoutZoomReplication->addWidget(labelAnimationSpeed);

        horizontalSliderAnimationSpeed = new QSlider(centralwidget);
        horizontalSliderAnimationSpeed->setObjectName("horizontalSliderAnimationSpeed");
        horizontalSliderAnimationSpeed->setMinimum(1);
        horizontalSliderAnimationSpeed->setMaximum(10);
        horizontalSliderAnimationSpeed->setPageStep(1);
        horizontalSliderAnimationSpeed->setValue(5);
        horizontalSliderAnimationSpeed->setSliderPosition(5);
        horizontalSliderAnimationSpeed->setOrientation(Qt::Horizontal);

        horizontalLayoutZoomReplication->addWidget(horizontalSliderAnimationSpeed);


        verticalLayout_7->addLayout(horizontalLayoutZoomReplication);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1081, 26));
        menuModel = new QMenu(menubar);
        menuModel->setObjectName("menuModel");
        menuSimulation = new QMenu(menubar);
        menuSimulation->setObjectName("menuSimulation");
        menuSimulation->setEnabled(true);
        menuAbout = new QMenu(menubar);
        menuAbout->setObjectName("menuAbout");
        menuView = new QMenu(menubar);
        menuView->setObjectName("menuView");
        menuZoom = new QMenu(menuView);
        menuZoom->setObjectName("menuZoom");
        menuViewDraw = new QMenu(menuView);
        menuViewDraw->setObjectName("menuViewDraw");
        menuViewAnimate = new QMenu(menuView);
        menuViewAnimate->setObjectName("menuViewAnimate");
        menuViewArranje = new QMenu(menuView);
        menuViewArranje->setObjectName("menuViewArranje");
        menuViewShow = new QMenu(menuView);
        menuViewShow->setObjectName("menuViewShow");
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName("menuEdit");
        menuTools = new QMenu(menubar);
        menuTools->setObjectName("menuTools");
        menuSimulator = new QMenu(menubar);
        menuSimulator->setObjectName("menuSimulator");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        toolBarModel = new QToolBar(MainWindow);
        toolBarModel->setObjectName("toolBarModel");
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarModel);
        dockWidgetConsole = new QDockWidget(MainWindow);
        dockWidgetConsole->setObjectName("dockWidgetConsole");
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(dockWidgetConsole->sizePolicy().hasHeightForWidth());
        dockWidgetConsole->setSizePolicy(sizePolicy2);
        dockWidgetConsole->setMinimumSize(QSize(118, 141));
        QIcon icon66;
        icon66.addFile(QString::fromUtf8(":/icons4/resources/icons/pack4/console.png"), QSize(), QIcon::Normal, QIcon::Off);
        dockWidgetConsole->setWindowIcon(icon66);
        dockWidgetConsole->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        dockWidgetConsole->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContentsConsole = new QWidget();
        dockWidgetContentsConsole->setObjectName("dockWidgetContentsConsole");
        verticalLayout = new QVBoxLayout(dockWidgetContentsConsole);
        verticalLayout->setObjectName("verticalLayout");
        textEdit_Console = new QTextEdit(dockWidgetContentsConsole);
        textEdit_Console->setObjectName("textEdit_Console");
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(textEdit_Console->sizePolicy().hasHeightForWidth());
        textEdit_Console->setSizePolicy(sizePolicy3);
        textEdit_Console->setMinimumSize(QSize(100, 100));
        QFont font3;
        font3.setPointSize(8);
        textEdit_Console->setFont(font3);
        textEdit_Console->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout->addWidget(textEdit_Console);

        dockWidgetConsole->setWidget(dockWidgetContentsConsole);
        MainWindow->addDockWidget(Qt::BottomDockWidgetArea, dockWidgetConsole);
        dockWidgetPlugins = new QDockWidget(MainWindow);
        dockWidgetPlugins->setObjectName("dockWidgetPlugins");
        QIcon icon67;
        icon67.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/component.ico"), QSize(), QIcon::Normal, QIcon::Off);
        dockWidgetPlugins->setWindowIcon(icon67);
        dockWidgetPlugins->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        dockWidgetPlugins->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContentsPlugin = new QWidget();
        dockWidgetContentsPlugin->setObjectName("dockWidgetContentsPlugin");
        horizontalLayout_2 = new QHBoxLayout(dockWidgetContentsPlugin);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        treeWidget_Plugins = new QTreeWidget(dockWidgetContentsPlugin);
        QTreeWidgetItem *__qtreewidgetitem2 = new QTreeWidgetItem();
        __qtreewidgetitem2->setText(0, QString::fromUtf8("1"));
        treeWidget_Plugins->setHeaderItem(__qtreewidgetitem2);
        treeWidget_Plugins->setObjectName("treeWidget_Plugins");
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(1);
        sizePolicy4.setHeightForWidth(treeWidget_Plugins->sizePolicy().hasHeightForWidth());
        treeWidget_Plugins->setSizePolicy(sizePolicy4);
        treeWidget_Plugins->setToolTipDuration(-1);
        treeWidget_Plugins->setFrameShape(QFrame::StyledPanel);
        treeWidget_Plugins->setDragEnabled(true);
        treeWidget_Plugins->setDragDropMode(QAbstractItemView::DragDrop);
        treeWidget_Plugins->setDefaultDropAction(Qt::CopyAction);
        treeWidget_Plugins->setAlternatingRowColors(true);
        treeWidget_Plugins->setAutoExpandDelay(8);
        treeWidget_Plugins->setIndentation(7);
        treeWidget_Plugins->setSortingEnabled(true);
        treeWidget_Plugins->setAnimated(true);
        treeWidget_Plugins->setWordWrap(true);
        treeWidget_Plugins->header()->setVisible(false);

        horizontalLayout_2->addWidget(treeWidget_Plugins);

        dockWidgetPlugins->setWidget(dockWidgetContentsPlugin);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidgetPlugins);
        dockWidgetPropertyEditor = new QDockWidget(MainWindow);
        dockWidgetPropertyEditor->setObjectName("dockWidgetPropertyEditor");
        QIcon icon68;
        icon68.addFile(QString::fromUtf8(":/icons3/resources/icons/pack3/ico/table.ico"), QSize(), QIcon::Normal, QIcon::Off);
        dockWidgetPropertyEditor->setWindowIcon(icon68);
        dockWidgetPropertyEditor->setFloating(false);
        dockWidgetPropertyEditor->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        dockWidgetPropertyEditor->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName("dockWidgetContents");
        horizontalLayout_4 = new QHBoxLayout(dockWidgetContents);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        treeViewPropertyEditor = new ObjectPropertyBrowser(dockWidgetContents);
        treeViewPropertyEditor->setObjectName("treeViewPropertyEditor");
        sizePolicy4.setHeightForWidth(treeViewPropertyEditor->sizePolicy().hasHeightForWidth());
        treeViewPropertyEditor->setSizePolicy(sizePolicy4);

        horizontalLayout_4->addWidget(treeViewPropertyEditor);

        dockWidgetPropertyEditor->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidgetPropertyEditor);
        toolBarGraphicalModel = new QToolBar(MainWindow);
        toolBarGraphicalModel->setObjectName("toolBarGraphicalModel");
        toolBarGraphicalModel->setToolButtonStyle(Qt::ToolButtonIconOnly);
        toolBarGraphicalModel->setFloatable(true);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarGraphicalModel);
        MainWindow->insertToolBarBreak(toolBarGraphicalModel);
        toolBarEdit = new QToolBar(MainWindow);
        toolBarEdit->setObjectName("toolBarEdit");
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarEdit);
        toolBarView = new QToolBar(MainWindow);
        toolBarView->setObjectName("toolBarView");
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarView);
        toolBarSimulation = new QToolBar(MainWindow);
        toolBarSimulation->setObjectName("toolBarSimulation");
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarSimulation);
        toolBarAbout = new QToolBar(MainWindow);
        toolBarAbout->setObjectName("toolBarAbout");
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarAbout);
        toolBarDraw = new QToolBar(MainWindow);
        toolBarDraw->setObjectName("toolBarDraw");
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarDraw);
        toolBarAnimate = new QToolBar(MainWindow);
        toolBarAnimate->setObjectName("toolBarAnimate");
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarAnimate);
        toolBarArranje = new QToolBar(MainWindow);
        toolBarArranje->setObjectName("toolBarArranje");
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBarArranje);

        menubar->addAction(menuSimulator->menuAction());
        menubar->addAction(menuModel->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuSimulation->menuAction());
        menubar->addAction(menuTools->menuAction());
        menubar->addAction(menuAbout->menuAction());
        menuModel->addAction(actionModelNew);
        menuModel->addSeparator();
        menuModel->addAction(actionModelOpen);
        menuModel->addAction(actionModelSave);
        menuModel->addAction(actionModelClose);
        menuModel->addSeparator();
        menuModel->addAction(actionModelInformation);
        menuModel->addAction(actionModelCheck);
        menuSimulation->addAction(actionSimulationConfigure);
        menuSimulation->addSeparator();
        menuSimulation->addAction(actionSimulationStart);
        menuSimulation->addAction(actionSimulationStep);
        menuSimulation->addAction(actionSimulationStop);
        menuSimulation->addSeparator();
        menuSimulation->addAction(actionSimulationPause);
        menuSimulation->addAction(actionSimulationResume);
        menuAbout->addAction(actionAboutAbout);
        menuAbout->addAction(actionAboutLicence);
        menuAbout->addAction(actionAboutGetInvolved);
        menuView->addAction(menuViewShow->menuAction());
        menuView->addAction(menuZoom->menuAction());
        menuView->addAction(menuViewArranje->menuAction());
        menuView->addSeparator();
        menuView->addAction(actionGModelShowConnect);
        menuView->addSeparator();
        menuView->addAction(menuViewDraw->menuAction());
        menuView->addAction(menuViewAnimate->menuAction());
        menuView->addSeparator();
        menuView->addAction(actionViewGroup);
        menuView->addAction(actionViewUngroup);
        menuView->addSeparator();
        menuView->addAction(actionViewConfigure);
        menuZoom->addAction(actionZoom_In);
        menuZoom->addAction(actionZoom_Out);
        menuZoom->addAction(actionZoom_All);
        menuViewDraw->addAction(actionDrawLine);
        menuViewDraw->addAction(actionDrawRectangle);
        menuViewDraw->addAction(actionDrawEllipse);
        menuViewDraw->addAction(actionDrawPoligon);
        menuViewDraw->addAction(actionDrawText);
        menuViewAnimate->addAction(actionAnimateSimulatedTime);
        menuViewAnimate->addAction(actionAnimateAttribute);
        menuViewAnimate->addAction(actionAnimateEntity);
        menuViewAnimate->addAction(actionAnimateEvent);
        menuViewAnimate->addAction(actionAnimateCounter);
        menuViewAnimate->addAction(actionAnimateStatistics);
        menuViewAnimate->addAction(actionAnimatePlot);
        menuViewAnimate->addSeparator();
        menuViewAnimate->addAction(actionAnimateVariable);
        menuViewAnimate->addAction(actionAnimateResource);
        menuViewAnimate->addAction(actionAnimateQueue);
        menuViewAnimate->addAction(actionAnimateExpression);
        menuViewArranje->addAction(actionArranjeLeft);
        menuViewArranje->addAction(actionArranjeCenter);
        menuViewArranje->addAction(actionArranjeRight);
        menuViewArranje->addSeparator();
        menuViewArranje->addAction(actionArranjeTop);
        menuViewArranje->addAction(actionArranjeMiddle);
        menuViewArranje->addAction(actionArranjeBototm);
        menuViewShow->addAction(actionShowGrid);
        menuViewShow->addAction(actionShowRule);
        menuViewShow->addAction(actionShowSnap);
        menuViewShow->addAction(actionShowGuides);
        menuViewShow->addAction(actionDiagrams);
        menuViewShow->addSeparator();
        menuViewShow->addAction(actionShowInternalElements);
        menuViewShow->addAction(actionShowAttachedElements);
        menuEdit->addAction(actionEditUndo);
        menuEdit->addAction(actionEditRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionEditCut);
        menuEdit->addAction(actionEditCopy);
        menuEdit->addAction(actionEditPaste);
        menuEdit->addAction(actionEditDelete);
        menuEdit->addSeparator();
        menuEdit->addAction(actionEditFind);
        menuEdit->addSeparator();
        menuTools->addAction(actionToolsDataAnalyzer);
        menuTools->addAction(actionToolsOptimizator);
        menuTools->addAction(actionToolsParserGrammarChecker);
        menuTools->addAction(actionParallelization);
        menuSimulator->addAction(actionSimulatorsPluginManager);
        menuSimulator->addSeparator();
        menuSimulator->addAction(actionSimulatorPreferences);
        menuSimulator->addSeparator();
        menuSimulator->addAction(actionSimulatorExit);
        toolBarModel->addAction(actionModelNew);
        toolBarModel->addAction(actionModelOpen);
        toolBarModel->addAction(actionModelSave);
        toolBarModel->addAction(actionModelClose);
        toolBarModel->addSeparator();
        toolBarModel->addAction(actionModelInformation);
        toolBarModel->addAction(actionModelCheck);
        toolBarModel->addSeparator();
        toolBarModel->addAction(actionSimulatorExit);
        toolBarGraphicalModel->addAction(actionGModelShowConnect);
        toolBarGraphicalModel->addAction(actionGModelComponentBreakpoint);
        toolBarGraphicalModel->addSeparator();
        toolBarEdit->addAction(actionEditUndo);
        toolBarEdit->addAction(actionEditRedo);
        toolBarEdit->addSeparator();
        toolBarEdit->addAction(actionEditCut);
        toolBarEdit->addAction(actionEditCopy);
        toolBarEdit->addAction(actionEditPaste);
        toolBarEdit->addAction(actionEditDelete);
        toolBarView->addAction(actionShowGrid);
        toolBarView->addAction(actionShowRule);
        toolBarView->addAction(actionShowSnap);
        toolBarView->addAction(actionShowGuides);
        toolBarView->addAction(actionSelectAll);
        toolBarView->addSeparator();
        toolBarView->addAction(actionZoom_In);
        toolBarView->addAction(actionZoom_Out);
        toolBarView->addAction(actionZoom_All);
        toolBarView->addSeparator();
        toolBarView->addAction(actionViewGroup);
        toolBarView->addAction(actionViewUngroup);
        toolBarView->addSeparator();
        toolBarSimulation->addAction(actionSimulationConfigure);
        toolBarSimulation->addAction(actionSimulationStart);
        toolBarSimulation->addAction(actionSimulationStep);
        toolBarSimulation->addAction(actionSimulationStop);
        toolBarSimulation->addAction(actionSimulationPause);
        toolBarSimulation->addAction(actionSimulationResume);
        toolBarSimulation->addAction(actionActivateGraphicalSimulation);
        toolBarAbout->addAction(actionAboutAbout);
        toolBarAbout->addAction(actionAboutLicence);
        toolBarAbout->addAction(actionAboutGetInvolved);
        toolBarDraw->addAction(actionDrawLine);
        toolBarDraw->addAction(actionDrawRectangle);
        toolBarDraw->addAction(actionDrawEllipse);
        toolBarDraw->addAction(actionDrawPoligon);
        toolBarDraw->addAction(actionDrawText);
        toolBarDraw->addSeparator();
        toolBarAnimate->addAction(actionAnimateSimulatedTime);
        toolBarAnimate->addAction(actionAnimateEntity);
        toolBarAnimate->addAction(actionAnimateEvent);
        toolBarAnimate->addAction(actionAnimateAttribute);
        toolBarAnimate->addAction(actionAnimateStatistics);
        toolBarAnimate->addAction(actionAnimateCounter);
        toolBarAnimate->addSeparator();
        toolBarAnimate->addAction(actionAnimateExpression);
        toolBarAnimate->addAction(actionAnimateVariable);
        toolBarAnimate->addAction(actionAnimatePlot);
        toolBarAnimate->addAction(actionAnimateResource);
        toolBarAnimate->addAction(actionAnimateQueue);
        toolBarAnimate->addAction(actionAnimateStation);
        toolBarArranje->addAction(actionArranjeLeft);
        toolBarArranje->addAction(actionArranjeCenter);
        toolBarArranje->addAction(actionArranjeRight);
        toolBarArranje->addSeparator();
        toolBarArranje->addAction(actionArranjeTop);
        toolBarArranje->addAction(actionArranjeMiddle);
        toolBarArranje->addAction(actionArranjeBototm);

        retranslateUi(MainWindow);

        tabWidgetCentral->setCurrentIndex(2);
        tabWidgetModel->setCurrentIndex(4);
        tabWidgetSimulation->setCurrentIndex(0);
        tabWidgetReports->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "GenESyS - Generic and Expansible System Simulator", nullptr));
        actionModelNew->setText(QCoreApplication::translate("MainWindow", "New", nullptr));
#if QT_CONFIG(tooltip)
        actionModelNew->setToolTip(QCoreApplication::translate("MainWindow", "New Model", nullptr));
#endif // QT_CONFIG(tooltip)
        actionModelOpen->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
#if QT_CONFIG(tooltip)
        actionModelOpen->setToolTip(QCoreApplication::translate("MainWindow", "Open Model", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionModelOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionModelSave->setText(QCoreApplication::translate("MainWindow", "Save", nullptr));
#if QT_CONFIG(tooltip)
        actionModelSave->setToolTip(QCoreApplication::translate("MainWindow", "Save Model", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionModelSave->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSimulatorExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
#if QT_CONFIG(tooltip)
        actionSimulatorExit->setToolTip(QCoreApplication::translate("MainWindow", "Exit Genesys", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSimulatorExit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSimulationStart->setText(QCoreApplication::translate("MainWindow", "Start", nullptr));
#if QT_CONFIG(tooltip)
        actionSimulationStart->setToolTip(QCoreApplication::translate("MainWindow", "Start Simulation", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSimulationStart->setShortcut(QCoreApplication::translate("MainWindow", "F5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSimulationStep->setText(QCoreApplication::translate("MainWindow", "Step", nullptr));
#if QT_CONFIG(tooltip)
        actionSimulationStep->setToolTip(QCoreApplication::translate("MainWindow", "Step Simulation", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSimulationStep->setShortcut(QCoreApplication::translate("MainWindow", "F6", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSimulationStop->setText(QCoreApplication::translate("MainWindow", "Stop", nullptr));
#if QT_CONFIG(tooltip)
        actionSimulationStop->setToolTip(QCoreApplication::translate("MainWindow", "Stop Simulation", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSimulationStop->setShortcut(QCoreApplication::translate("MainWindow", "F7", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSimulationPause->setText(QCoreApplication::translate("MainWindow", "Pause", nullptr));
#if QT_CONFIG(tooltip)
        actionSimulationPause->setToolTip(QCoreApplication::translate("MainWindow", "Pause Simulation", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSimulationPause->setShortcut(QCoreApplication::translate("MainWindow", "F8", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSimulationResume->setText(QCoreApplication::translate("MainWindow", "Resume", nullptr));
#if QT_CONFIG(tooltip)
        actionSimulationResume->setToolTip(QCoreApplication::translate("MainWindow", "Resume Simulation", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSimulationResume->setShortcut(QCoreApplication::translate("MainWindow", "F9", nullptr));
#endif // QT_CONFIG(shortcut)
        actionModelClose->setText(QCoreApplication::translate("MainWindow", "Close", nullptr));
#if QT_CONFIG(tooltip)
        actionModelClose->setToolTip(QCoreApplication::translate("MainWindow", "Close Model", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionModelClose->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionModelCheck->setText(QCoreApplication::translate("MainWindow", "Check", nullptr));
#if QT_CONFIG(tooltip)
        actionModelCheck->setToolTip(QCoreApplication::translate("MainWindow", "Check Model", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionModelCheck->setShortcut(QCoreApplication::translate("MainWindow", "F4", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAboutAbout->setText(QCoreApplication::translate("MainWindow", "Genesys", nullptr));
#if QT_CONFIG(tooltip)
        actionAboutAbout->setToolTip(QCoreApplication::translate("MainWindow", "About Genesys", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAboutLicence->setText(QCoreApplication::translate("MainWindow", "Licence", nullptr));
#if QT_CONFIG(tooltip)
        actionAboutLicence->setToolTip(QCoreApplication::translate("MainWindow", "About Licence", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAboutGetInvolved->setText(QCoreApplication::translate("MainWindow", "Get Involved", nullptr));
#if QT_CONFIG(tooltip)
        actionAboutGetInvolved->setToolTip(QCoreApplication::translate("MainWindow", "Get Involved in Genesys", nullptr));
#endif // QT_CONFIG(tooltip)
        actionShowGrid->setText(QCoreApplication::translate("MainWindow", "Grid", nullptr));
        actionShowSnap->setText(QCoreApplication::translate("MainWindow", "Snap", nullptr));
        actionEditUndo->setText(QCoreApplication::translate("MainWindow", "Undo", nullptr));
#if QT_CONFIG(shortcut)
        actionEditUndo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEditRedo->setText(QCoreApplication::translate("MainWindow", "Redo", nullptr));
#if QT_CONFIG(shortcut)
        actionEditRedo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Y", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEditFind->setText(QCoreApplication::translate("MainWindow", "Find", nullptr));
#if QT_CONFIG(shortcut)
        actionEditFind->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+F", nullptr));
#endif // QT_CONFIG(shortcut)
        actionReplace->setText(QCoreApplication::translate("MainWindow", "Replace", nullptr));
#if QT_CONFIG(shortcut)
        actionReplace->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+R", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEditCut->setText(QCoreApplication::translate("MainWindow", "Cut", nullptr));
#if QT_CONFIG(shortcut)
        actionEditCut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEditCopy->setText(QCoreApplication::translate("MainWindow", "Copy", nullptr));
#if QT_CONFIG(shortcut)
        actionEditCopy->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEditPaste->setText(QCoreApplication::translate("MainWindow", "Paste", nullptr));
#if QT_CONFIG(shortcut)
        actionEditPaste->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionShowRule->setText(QCoreApplication::translate("MainWindow", "Rule", nullptr));
        actionShowGuides->setText(QCoreApplication::translate("MainWindow", "Guides", nullptr));
        actionZoom_In->setText(QCoreApplication::translate("MainWindow", "Zoom In", nullptr));
#if QT_CONFIG(shortcut)
        actionZoom_In->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+=", nullptr));
#endif // QT_CONFIG(shortcut)
        actionZoom_Out->setText(QCoreApplication::translate("MainWindow", "Zoom Out", nullptr));
#if QT_CONFIG(shortcut)
        actionZoom_Out->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+-", nullptr));
#endif // QT_CONFIG(shortcut)
        actionZoom_All->setText(QCoreApplication::translate("MainWindow", "Zoom All", nullptr));
#if QT_CONFIG(shortcut)
        actionZoom_All->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+0", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDrawLine->setText(QCoreApplication::translate("MainWindow", "Line", nullptr));
        actionDrawRectangle->setText(QCoreApplication::translate("MainWindow", "Rectangle", nullptr));
        actionDrawEllipse->setText(QCoreApplication::translate("MainWindow", "Ellipse", nullptr));
        actionAnimateSimulatedTime->setText(QCoreApplication::translate("MainWindow", "Simulated Time", nullptr));
#if QT_CONFIG(tooltip)
        actionAnimateSimulatedTime->setToolTip(QCoreApplication::translate("MainWindow", "Simulated Time", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAnimateVariable->setText(QCoreApplication::translate("MainWindow", "Variable", nullptr));
        actionAnimateExpression->setText(QCoreApplication::translate("MainWindow", "Expression", nullptr));
        actionAnimateResource->setText(QCoreApplication::translate("MainWindow", "Resource", nullptr));
        actionAnimateQueue->setText(QCoreApplication::translate("MainWindow", "Queue", nullptr));
        actionAnimateStation->setText(QCoreApplication::translate("MainWindow", "Station", nullptr));
        actionGModelShowConnect->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        actionGModelComponentBreakpoint->setText(QCoreApplication::translate("MainWindow", "Component Breakpoint", nullptr));
#if QT_CONFIG(tooltip)
        actionGModelComponentBreakpoint->setToolTip(QCoreApplication::translate("MainWindow", "Component Breakpoint", nullptr));
#endif // QT_CONFIG(tooltip)
        actionArranje->setText(QCoreApplication::translate("MainWindow", "Arranje", nullptr));
        actionSimulatorPreferences->setText(QCoreApplication::translate("MainWindow", "Preferences", nullptr));
#if QT_CONFIG(tooltip)
        actionSimulatorPreferences->setToolTip(QCoreApplication::translate("MainWindow", "Application Preferences", nullptr));
#endif // QT_CONFIG(tooltip)
        actionEditDelete->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        actionArranjeLeft->setText(QCoreApplication::translate("MainWindow", "Left", nullptr));
        actionArranjeCenter->setText(QCoreApplication::translate("MainWindow", "Center", nullptr));
        actionArranjeRight->setText(QCoreApplication::translate("MainWindow", "Right", nullptr));
        actionArranjeTop->setText(QCoreApplication::translate("MainWindow", "Top", nullptr));
        actionArranjeMiddle->setText(QCoreApplication::translate("MainWindow", "Middle", nullptr));
        actionArranjeBototm->setText(QCoreApplication::translate("MainWindow", "Bototm", nullptr));
        actionShowInternalElements->setText(QCoreApplication::translate("MainWindow", "Internal Elements", nullptr));
        actionShowAttachedElements->setText(QCoreApplication::translate("MainWindow", "Attached Elements", nullptr));
        actionSimulationConfigure->setText(QCoreApplication::translate("MainWindow", "Configure", nullptr));
        actionDrawText->setText(QCoreApplication::translate("MainWindow", "Text", nullptr));
#if QT_CONFIG(tooltip)
        actionDrawText->setToolTip(QCoreApplication::translate("MainWindow", "Draw Text", nullptr));
#endif // QT_CONFIG(tooltip)
        actionDrawPoligon->setText(QCoreApplication::translate("MainWindow", "Poligon", nullptr));
#if QT_CONFIG(tooltip)
        actionDrawPoligon->setToolTip(QCoreApplication::translate("MainWindow", "Draw Poligon", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAnimateCounter->setText(QCoreApplication::translate("MainWindow", "Counter", nullptr));
        actionAnimateEntity->setText(QCoreApplication::translate("MainWindow", "Entity", nullptr));
#if QT_CONFIG(tooltip)
        actionAnimateEntity->setToolTip(QCoreApplication::translate("MainWindow", "Anime Entity", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAnimateEvent->setText(QCoreApplication::translate("MainWindow", "Event", nullptr));
#if QT_CONFIG(tooltip)
        actionAnimateEvent->setToolTip(QCoreApplication::translate("MainWindow", "Anime Event", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAnimateAttribute->setText(QCoreApplication::translate("MainWindow", "Attribute", nullptr));
#if QT_CONFIG(tooltip)
        actionAnimateAttribute->setToolTip(QCoreApplication::translate("MainWindow", "Anime Attribute", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAnimateStatistics->setText(QCoreApplication::translate("MainWindow", "Statistics", nullptr));
#if QT_CONFIG(tooltip)
        actionAnimateStatistics->setToolTip(QCoreApplication::translate("MainWindow", "Anime Statistics", nullptr));
#endif // QT_CONFIG(tooltip)
        actionViewConfigure->setText(QCoreApplication::translate("MainWindow", "Configure", nullptr));
#if QT_CONFIG(tooltip)
        actionViewConfigure->setToolTip(QCoreApplication::translate("MainWindow", "Configure View", nullptr));
#endif // QT_CONFIG(tooltip)
        actionSimulatorsPluginManager->setText(QCoreApplication::translate("MainWindow", "Plugin Manager", nullptr));
        actionModelInformation->setText(QCoreApplication::translate("MainWindow", "Information", nullptr));
#if QT_CONFIG(tooltip)
        actionModelInformation->setToolTip(QCoreApplication::translate("MainWindow", "Model Information", nullptr));
#endif // QT_CONFIG(tooltip)
        actionToolsDataAnalyzer->setText(QCoreApplication::translate("MainWindow", "Data Analyzer", nullptr));
        actionToolsOptimizator->setText(QCoreApplication::translate("MainWindow", "Optimizator", nullptr));
        actionToolsParserGrammarChecker->setText(QCoreApplication::translate("MainWindow", "Parser Grammar", nullptr));
        actionViewGroup->setText(QCoreApplication::translate("MainWindow", "Group", nullptr));
        actionViewUngroup->setText(QCoreApplication::translate("MainWindow", "Ungroup", nullptr));
        actionAnimatePlot->setText(QCoreApplication::translate("MainWindow", "Plot", nullptr));
        actionDiagrams->setText(QCoreApplication::translate("MainWindow", "Diagrams", nullptr));
        actionActivateGraphicalSimulation->setText(QCoreApplication::translate("MainWindow", "ActivateGraphicalSimulation", nullptr));
#if QT_CONFIG(tooltip)
        actionActivateGraphicalSimulation->setToolTip(QCoreApplication::translate("MainWindow", "Activate graphical simulation", nullptr));
#endif // QT_CONFIG(tooltip)
        actionSelectAll->setText(QCoreApplication::translate("MainWindow", "actionSelectAll", nullptr));
#if QT_CONFIG(tooltip)
        actionSelectAll->setToolTip(QCoreApplication::translate("MainWindow", "select all elements in scene", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSelectAll->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionParallelization->setText(QCoreApplication::translate("MainWindow", "Parallelization", nullptr));
        tabWidgetModel->setTabText(tabWidgetModel->indexOf(tabModelSimLanguage), QCoreApplication::translate("MainWindow", "SimulLang", nullptr));
        tabWidgetModel->setTabText(tabWidgetModel->indexOf(tabModelCpp), QCoreApplication::translate("MainWindow", "C++", nullptr));
        label_ModelGraphic->setText(QString());
        checkBox_ShowElements->setText(QCoreApplication::translate("MainWindow", "Show elements", nullptr));
        checkBox_ShowInternals->setText(QCoreApplication::translate("MainWindow", "Show internals", nullptr));
        checkBox_ShowLevels->setText(QCoreApplication::translate("MainWindow", "Show Levels", nullptr));
        checkBox_ShowRecursive->setText(QCoreApplication::translate("MainWindow", "Recursive", nullptr));
        pushButton_Export->setText(QCoreApplication::translate("MainWindow", "Export", nullptr));
        tabWidgetModel->setTabText(tabWidgetModel->indexOf(tabModelDiagram), QCoreApplication::translate("MainWindow", "Diagram", nullptr));
        tabWidgetModel->setTabText(tabWidgetModel->indexOf(tabModelComponents), QCoreApplication::translate("MainWindow", "Components", nullptr));
        tabWidgetModel->setTabText(tabWidgetModel->indexOf(tabModelDataDefinitions), QCoreApplication::translate("MainWindow", "DataDefinitions", nullptr));
        tabWidgetCentral->setTabText(tabWidgetCentral->indexOf(tabCentralModel), QCoreApplication::translate("MainWindow", "Model", nullptr));
        tabWidgetSimulation->setTabText(tabWidgetSimulation->indexOf(tabDebugBreakpoints), QCoreApplication::translate("MainWindow", "Breakpoints", nullptr));
        tabWidgetSimulation->setTabText(tabWidgetSimulation->indexOf(tabDebugVariables), QCoreApplication::translate("MainWindow", "Variables", nullptr));
        tabWidgetSimulation->setTabText(tabWidgetSimulation->indexOf(tabDebugEntities), QCoreApplication::translate("MainWindow", "Entities", nullptr));
        tabWidgetSimulation->setTabText(tabWidgetSimulation->indexOf(tabDebugTraces), QCoreApplication::translate("MainWindow", "Traces", nullptr));
        tabWidgetSimulation->setTabText(tabWidgetSimulation->indexOf(tabDebugEvents), QCoreApplication::translate("MainWindow", "Events", nullptr));
        tabWidgetCentral->setTabText(tabWidgetCentral->indexOf(tabCentralSimulation), QCoreApplication::translate("MainWindow", "Simulation", nullptr));
        tabWidgetReports->setTabText(tabWidgetReports->indexOf(tabReportsReports), QCoreApplication::translate("MainWindow", "Reports", nullptr));
        tabWidgetReports->setTabText(tabWidgetReports->indexOf(TabReportsResults), QCoreApplication::translate("MainWindow", "Results", nullptr));
        tabWidgetReports->setTabText(tabWidgetReports->indexOf(tabReportsPlots), QCoreApplication::translate("MainWindow", "Plots", nullptr));
        tabWidgetCentral->setTabText(tabWidgetCentral->indexOf(tabCentralWidget), QCoreApplication::translate("MainWindow", "Results", nullptr));
        labelMousePos->setText(QCoreApplication::translate("MainWindow", "<-,->", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Zoom:", nullptr));
        label_ZoomValue->setText(QCoreApplication::translate("MainWindow", "0.0", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Replication ", nullptr));
        label_ReplicationNum->setText(QCoreApplication::translate("MainWindow", "0/-", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", ":", nullptr));
        labelAnimationSpeed->setText(QCoreApplication::translate("MainWindow", "Animation speed", nullptr));
        menuModel->setTitle(QCoreApplication::translate("MainWindow", "Model", nullptr));
        menuSimulation->setTitle(QCoreApplication::translate("MainWindow", "Simulation", nullptr));
        menuAbout->setTitle(QCoreApplication::translate("MainWindow", "About", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
        menuZoom->setTitle(QCoreApplication::translate("MainWindow", "Zoom", nullptr));
        menuViewDraw->setTitle(QCoreApplication::translate("MainWindow", "Draw", nullptr));
        menuViewAnimate->setTitle(QCoreApplication::translate("MainWindow", "Animate", nullptr));
        menuViewArranje->setTitle(QCoreApplication::translate("MainWindow", "Align", nullptr));
        menuViewShow->setTitle(QCoreApplication::translate("MainWindow", "Show", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "Tools", nullptr));
        menuSimulator->setTitle(QCoreApplication::translate("MainWindow", "Simulator", nullptr));
        toolBarModel->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
        dockWidgetConsole->setWindowTitle(QCoreApplication::translate("MainWindow", "Console", nullptr));
        textEdit_Console->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Noto Sans Brahmi'; font-size:8pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'DejaVu LGC Sans';\">Console</span></p></body></html>", nullptr));
        dockWidgetPlugins->setWindowTitle(QCoreApplication::translate("MainWindow", "Plugins", nullptr));
        dockWidgetPropertyEditor->setWindowTitle(QCoreApplication::translate("MainWindow", "Property Editor", nullptr));
        toolBarGraphicalModel->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBarGraphicalModel", nullptr));
        toolBarEdit->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar_2", nullptr));
        toolBarView->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar_3", nullptr));
        toolBarSimulation->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar_4", nullptr));
        toolBarAbout->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar_5", nullptr));
        toolBarDraw->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
        toolBarAnimate->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
        toolBarArranje->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
