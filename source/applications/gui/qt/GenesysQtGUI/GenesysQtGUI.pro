QT += core gui
QT += printsupport
QT += designer
greaterThan(QT_MAJOR_VERSION, 6): QT += widgets
CONFIG += c++14

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
    ../../../../kernel/simulator/Attribute.cpp \
    ../../../../kernel/simulator/ComponentManager.cpp \
    ../../../../kernel/simulator/ConnectionManager.cpp \
    ../../../../kernel/simulator/Counter.cpp \
    ../../../../kernel/simulator/CppSerializer.cpp \
    ../../../../kernel/simulator/Entity.cpp \
    ../../../../kernel/simulator/EntityType.cpp \
    ../../../../kernel/simulator/Event.cpp \
    ../../../../kernel/simulator/ExperimentManager.cpp \
    ../../../../kernel/simulator/ExperimentManagerDefaultImpl1.cpp \
    ../../../../kernel/simulator/GenSerializer.cpp \
    ../../../../kernel/simulator/GenesysPropertyIntrospection.cpp \
    ../../../../kernel/simulator/JsonSerializer.cpp \
    ../../../../kernel/simulator/LicenceManager.cpp \
    ../../../../kernel/simulator/Model.cpp \
    ../../../../kernel/simulator/ModelCheckerDefaultImpl1.cpp \
    ../../../../kernel/simulator/ModelComponent.cpp \
    ../../../../kernel/simulator/ModelDataDefinition.cpp \
    ../../../../kernel/simulator/ModelDataManager.cpp \
    ../../../../kernel/simulator/ModelInfo.cpp \
    ../../../../kernel/simulator/ModelManager.cpp \
    ../../../../kernel/simulator/ModelPersistenceDefaultImpl2.cpp \
    ../../../../kernel/simulator/ModelSerializer.cpp \
    ../../../../kernel/simulator/ModelSimulation.cpp \
    ../../../../kernel/simulator/OnEventManager.cpp \
    ../../../../kernel/simulator/ParserChangesInformation.cpp \
    ../../../../kernel/simulator/ParserDefaultImpl1.cpp \
    ../../../../kernel/simulator/ParserDefaultImpl2.cpp \
    ../../../../kernel/simulator/ParserManager.cpp \
    ../../../../kernel/simulator/Persistence.cpp \
    ../../../../kernel/simulator/Plugin.cpp \
    ../../../../kernel/simulator/PluginInformation.cpp \
    ../../../../kernel/simulator/PluginManager.cpp \
    ../../../../kernel/simulator/SimulationExperiment.cpp \
    ../../../../kernel/simulator/SimulationReporterDefaultImpl1.cpp \
    ../../../../kernel/simulator/SimulationScenario.cpp \
    ../../../../kernel/simulator/Simulator.cpp \
    ../../../../kernel/simulator/SinkModelComponent.cpp \
    ../../../../kernel/simulator/SourceModelComponent.cpp \
    ../../../../kernel/simulator/StatisticsCollector.cpp \
    ../../../../kernel/simulator/TraceManager.cpp \
    ../../../../kernel/simulator/XmlSerializer.cpp \
    ../../../../kernel/statistics/CollectorDatafileDefaultImpl1.cpp \
    ../../../../kernel/statistics/CollectorDefaultImpl1.cpp \
    ../../../../kernel/statistics/SamplerDefaultImpl1.cpp \
    ../../../../kernel/statistics/SorttFile.cpp \
    ../../../../kernel/statistics/StatisticsDataFileDefaultImpl.cpp \
    ../../../../kernel/statistics/StatisticsDefaultImpl1.cpp \
    ../../../../kernel/util/Util.cpp \
    ../../../../parser/Genesys++-driver.cpp \
    ../../../../parser/Genesys++-scanner.cpp \
    ../../../../parser/GenesysParser.cpp \
    ../../../../parser/obj_t.cpp \
    ../../../../plugins/PluginConnectorDummyImpl1.cpp \
    ../../../../plugins/components/MaterialHandling/Access.cpp \
    ../../../../plugins/components/DiscreteProcessing/Assign.cpp \
    ../../../../plugins/components/Grouping/Batch.cpp \
    ../../../../plugins/components/DiscreteProcessing/Buffer.cpp \
    ../../../../plugins/components/DiscreteProcessing/Clone.cpp \
    ../../../../plugins/components/Logic/CppForG.cpp \
    ../../../../plugins/components/DiscreteProcessing/Create.cpp \
    ../../../../plugins/components/Decisions/Decide.cpp \
    ../../../../plugins/components/DiscreteProcessing/Delay.cpp \
    ../../../../plugins/components/Continuous/DiffEquations.cpp \
    ../../../../plugins/components/DiscreteProcessing/Dispose.cpp \
    ../../../../plugins/components/Decisions/DropOff.cpp \
    ../../../../plugins/components/DiscreteProcessing/DummyComponent.cpp \
    ../../../../plugins/components/MaterialHandling/Enter.cpp \
    ../../../../plugins/components/MaterialHandling/Exit.cpp \
    ../../../../plugins/components/Network/ModalModelDefault.cpp \
    ../../../../plugins/components/Network/ModalModelFSM.cpp \
    ../../../../plugins/components/Network/ModalModelPetriNet.cpp \
    ../../../../plugins/components/Decisions/PickableStationItem.cpp \
    ../../../../plugins/components/ElectronicsSimulation/SPICECircuit.cpp \
    ../../../../plugins/components/ElectronicsSimulation/SPICENode.cpp \
    ../../../../plugins/components/Decisions/Wait.cpp \
    ../../../../plugins/components/Continuous/LSODE.cpp \
    ../../../../plugins/components/MaterialHandling/Leave.cpp \
    ../../../../plugins/components/DiscreteProcessing/MarkovChain.cpp \
    ../../../../plugins/components/Decision/Match.cpp \
    ../../../../plugins/components/DiscreteProcessing/OLD_ODEelement.cpp \
    ../../../../plugins/components/Decisions/PickStation.cpp \
    ../../../../plugins/components/Decisions/PickUp.cpp \
    ../../../../plugins/components/DiscreteProcessing/Process.cpp \
    ../../../../plugins/components/DiscreteProcessing/QueueableItem.cpp \
    ../../../../plugins/components/InputOutput/Record.cpp \
    ../../../../plugins/components/DiscreteProcessing/Release.cpp \
    ../../../../plugins/components/Decisions/Remove.cpp \
    ../../../../plugins/components/MaterialHandling/Route.cpp \
    ../../../../plugins/components/Decisions/Search.cpp \
    ../../../../plugins/components/DiscreteProcessing/SeizableItem.cpp \
    ../../../../plugins/components/DiscreteProcessing/Seize.cpp \
    ../../../../plugins/components/Grouping/Separate.cpp \
    ../../../../plugins/components/Decisions/Signal.cpp \
    ../../../../plugins/components/MaterialHandling/Start.cpp \
    ../../../../plugins/components/MaterialHandling/Stop.cpp \
    ../../../../plugins/components/MaterialHandling/Store.cpp \
    ../../../../plugins/components/Logic/Submodel.cpp \
    ../../../../plugins/components/MaterialHandling/Unstore.cpp \
    ../../../../plugins/components/InputOutput/Write.cpp \
    ../../../../plugins/components/DiscreteProcessing/DefaultNode.cpp \
    ../../../../plugins/components/Network/DefaultTransitionExtensions.cpp \
    ../../../../plugins/components/Network/FSMState.cpp \
    ../../../../plugins/components/Network/PetriPlace.cpp \
    ../../../../plugins/data/DiscreteProcessing/AssignmentItem.cpp \
    ../../../../plugins/data/BiochemicalSimulation/BioNetwork.cpp \
    ../../../../plugins/data/BiochemicalSimulation/BioParameter.cpp \
    ../../../../plugins/data/BiochemicalSimulation/BioReaction.cpp \
    ../../../../plugins/data/BiochemicalSimulation/BioSpecies.cpp \
    ../../../../plugins/data/BiochemicalSimulation/BioSimulatorRunner.cpp \
    ../../../../plugins/data/DiscreteProcessing/CppCompiler.cpp \
    ../../../../plugins/data/DataDefinition/DummyElement.cpp \
    ../../../../plugins/data/DiscreteProcessing/EntityGroup.cpp \
    ../../../../plugins/data/DiscreteProcessing/Failure.cpp \
    ../../../../plugins/data/DiscreteProcessing/File.cpp \
    ../../../../plugins/data/DiscreteProcessing/Formula.cpp \
    ../../../../plugins/data/DiscreteProcessing/Label.cpp \
    ../../../../plugins/data/DiscreteProcessing/Queue.cpp \
    ../../../../plugins/data/DiscreteProcessing/Resource.cpp \
    ../../../../plugins/data/ElectronicsSimulation/SPICERunner.cpp \
    ../../../../plugins/data/DiscreteProcessing/Schedule.cpp \
    ../../../../plugins/data/DiscreteProcessing/Sequence.cpp \
    ../../../../plugins/data/DiscreteProcessing/Set.cpp \
    ../../../../plugins/data/DiscreteProcessing/SignalData.cpp \
    ../../../../plugins/data/DiscreteProcessing/Station.cpp \
    ../../../../plugins/data/DiscreteProcessing/Storage.cpp \
    ../../../../plugins/data/DiscreteProcessing/Variable.cpp \
    ../../../../tools/FactorialDesign/FactorialDesign.cpp \
    ../../../../tools/FitterDummyImpl.cpp \
    ../../../../tools/HypothesisTesterDefaultImpl1.cpp \
    ../../../../tools/OptimizerDefaultImpl1.cpp \
    ../../../../tools/ProbabilityDistribution.cpp \
    ../../../../tools/ProbabilityDistributionBase.cpp \
    ../../../../tools/SolverDefaultImpl1.cpp \
    ../../../BaseGenesysTerminalApplication.cpp \
    ../../../terminal/GenesysShell/GenesysShell.cpp \
    ../../../terminal/examples/arenaExamples/AirportSecurityExample.cpp \
    ../../../terminal/examples/arenaExamples/AirportSecurityExampleExtended.cpp \
    ../../../terminal/examples/arenaExamples/Airport_Extended1.cpp \
    ../../../terminal/examples/arenaExamples/Assembly_Line.cpp \
    ../../../terminal/examples/arenaExamples/Banking_Transactions.cpp \
    ../../../terminal/examples/arenaExamples/Example_BasicOrderShipping.cpp \
    ../../../terminal/examples/arenaExamples/Example_PortModel.cpp \
    ../../../terminal/examples/arenaExamples/Example_PublicTransport.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_AddingResource.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_AlternatingEntityCreation.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ArrivalsEntityTypeVsAttribute.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_AssignExample.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_AutomaticStatisticsCollection.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_BasicModeling.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_BatchAndSeparate.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ContinuousFlowEntities.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_Create.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_DecideNWayByChance.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_DefiningAttributesAsStrings.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_DefiningControlLogic.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_DefiningResourceCapacity.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_DelayBasedOnReplication.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_EntitiesProcessedByPriority.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_EvaluatingConditionsBeforeEnteringQueue.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_Expression.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_InventoryAndHoldingCosts.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_MaxArrivalsField.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ModelRunUntil1000Parts.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ModuleDisplayVariables.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_OverlappingResources.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ParallelProcessingOfEntities.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_PlacingEntitiesInQueueSets.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_PriorityExample.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ProcessArena.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_Record_Arena.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_RemovingAndReorderingEntitiesInAQueue.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ResourceCosting.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ResourceScheduleCosting.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ResourceSets.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_SeizingMultipleSimultaneosly.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_SelectingRouteBasedOnProbability.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_SelectingShorterQueue.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_SynchronizingParallelEntities.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_ValueAdded.cpp \
    ../../../terminal/examples/arenaSmarts/Smart_WaitForSignal.cpp \
    ../../../terminal/examples/book/Book_Cap02_Example01.cpp \
    ../../../terminal/examples/smarts/Smart_AssignWriteSeizes.cpp \
    ../../../terminal/examples/smarts/Smart_BatchSeparate.cpp \
    ../../../terminal/examples/smarts/Smart_Buffer.cpp \
    ../../../terminal/examples/smarts/Smart_Clone.cpp \
    ../../../terminal/examples/smarts/Smart_CppForG.cpp \
    ../../../terminal/examples/smarts/Smart_Delay.cpp \
    ../../../terminal/examples/smarts/Smart_Dummy.cpp \
    ../../../terminal/examples/smarts/Smart_Failures.cpp \
    ../../../terminal/examples/smarts/Smart_HoldSearchRemove.cpp \
    ../../../terminal/examples/smarts/Smart_LSODE.cpp \
    ../../../terminal/examples/smarts/Smart_MarkovChain.cpp \
    ../../../terminal/examples/smarts/Smart_Modalmodel.cpp \
    ../../../terminal/examples/smarts/Smart_ModelInfoModelSimulation.cpp \
    ../../../terminal/examples/smarts/Smart_ODE.cpp \
    ../../../terminal/examples/smarts/Smart_OnEvent.cpp \
    ../../../terminal/examples/smarts/Smart_Parser.cpp \
    ../../../terminal/examples/smarts/Smart_ParserModelFunctions.cpp \
    ../../../terminal/examples/smarts/Smart_Plugin.cpp \
    ../../../terminal/examples/smarts/Smart_Process.cpp \
    ../../../terminal/examples/smarts/Smart_ProcessSet.cpp \
    ../../../terminal/examples/smarts/Smart_Record.cpp \
    ../../../terminal/examples/smarts/Smart_RouteStation.cpp \
    ../../../terminal/examples/smarts/Smart_SeizeDelayRelease.cpp \
    ../../../terminal/examples/smarts/Smart_SeizeDelayReleaseMany.cpp \
    ../../../terminal/examples/smarts/Smart_SeizeDelayReleaseNoDataDefs.cpp \
    ../../../terminal/examples/smarts/Smart_Sequence.cpp \
    ../../../terminal/examples/smarts/Smart_SimulationControlResponse.cpp \
    ../../../terminal/examples/smarts/Smart_WaitScanCondition.cpp \
    ../../../terminal/examples/smarts/Smart_WaitSignal.cpp \
    ../../../terminal/examples/smarts/Smaty_DefaultModalModel.cpp \
    ../../../terminal/examples/teaching/AnElectronicAssemblyAndTestSystem.cpp \
    ../../../terminal/examples/teaching/ContinuousModel.cpp \
    ../../../terminal/examples/teaching/FullSimulationOfComplexModel.cpp \
    ../../../terminal/examples/teaching/Half_Adder.cpp \
    ../../../terminal/examples/teaching/Loja/Loja01.cpp \
    ../../../terminal/examples/teaching/OperatingSystem02.cpp \
    ../../../terminal/examples/teaching/OperatingSystem03.cpp \
    ../../../terminal/examples/teaching/Rectifier.cpp \
    ../../../web/BaseGenesysWebApplication.cpp \
    ../../../web/api/ApiRouter.cpp \
    ../../../web/auth/TokenService.cpp \
    ../../../web/http/SimpleHttpServer.cpp \
    ../../../web/service/SimulatorSessionService.cpp \
    ../../../web/session/SessionManager.cpp \
    ../../../web/worker/WorkerJobManager.cpp \
    codeeditor/CodeEditor.cpp \
    controllers/SimulationController.cpp \
    # Phase-3 GUI refactor controller for model-inspector responsibilities.
    controllers/ModelInspectorController.cpp \
    # Phase-4 GUI refactor controllers for trace and simulation-event responsibilities.
    controllers/TraceConsoleController.cpp \
    controllers/SimulationEventController.cpp \
    # Phase-5 GUI refactor controller for plugin-catalog responsibilities.
    controllers/PluginCatalogController.cpp \
    # Phase-6 GUI refactor controller for property-editor responsibilities.
    controllers/PropertyEditorController.cpp \
    # Phase-7 GUI refactor controller for model/application lifecycle responsibilities.
    controllers/ModelLifecycleController.cpp \
    # Phase-8 GUI refactor controller for simulation-command responsibilities.
    controllers/SimulationCommandController.cpp \
    # Phase-9 GUI refactor controller for edit-command responsibilities.
    controllers/EditCommandController.cpp \
    # Phase-10 GUI refactor controller for scene/view/drawing responsibilities.
    controllers/SceneToolController.cpp \
    # Graphical context-menu controller for canvas popup actions.
    controllers/GraphicalContextMenuController.cpp \
    # Phase-11 GUI refactor controller for dialog/utility responsibilities.
    controllers/DialogUtilityController.cpp \
    # Phase-1 GUI refactor services for model representations.
    services/ModelLanguageSynchronizer.cpp \
    services/GraphvizModelExporter.cpp \
    services/CppModelExporter.cpp \
    services/GraphicalModelSerializer.cpp \
    services/GraphicalModelBuilder.cpp \
    mainwindow_controller.cpp \
    mainwindow_modelrepresentations.cpp \
    mainwindow_scene.cpp \
    mainwindow_simulator.cpp \
    propertyeditor/DataComponentProperty.cpp \
    graphicals/ModelGraphicsScene.cpp \
    graphicals/ModelGraphicsView.cpp \
    propertyeditor/ObjectPropertyBrowser.cpp \
    propertyeditor/PropertyEditor.cpp \
    propertyeditor/DataComponentEditor.cpp \
    propertyeditor/ComboBoxEnum.cpp \
    actions/AddUndoCommand.cpp \
    actions/DeleteUndoCommand.cpp \
    actions/GroupUndoCommand.cpp \
    actions/MoveUndoCommand.cpp \
    actions/PasteUndoCommand.cpp \
    actions/UngroupUndoCommand.cpp \
    animations/AnimationCounter.cpp \
    animations/AnimationPlaceholder.cpp \
    animations/AnimationQueue.cpp \
    animations/AnimationTimer.cpp \
    animations/AnimationTransition.cpp \
    animations/AnimationVariable.cpp \
    dialogs/DialogFind.cpp \
    dialogs/DialogSelectCounter.cpp \
    dialogs/DialogSelectVariable.cpp \
    dialogs/DialogTimerConfigure.cpp \
    dialogs/Dialogmodelinformation.cpp \
    dialogs/dialogBreakpoint.cpp \
    dialogs/dialogpluginmanager.cpp \
    dialogs/dialogsimulationconfigure.cpp \
    dialogs/dialogsystempreferences.cpp \
    graphicals/GraphicalAnimateExpression.cpp \
    graphicals/GraphicalAssociation.cpp \
    graphicals/GraphicalComponentPort.cpp \
    graphicals/GraphicalConnection.cpp \
    graphicals/GraphicalDiagramConnection.cpp \
    graphicals/GraphicalImageAnimation.cpp \
    graphicals/GraphicalModelComponent.cpp \
    graphicals/GraphicalModelDataDefinition.cpp \
    guithememanager.cpp \
    GuiCrashDiagnostics.cpp \
    GuiScopeTrace.cpp \
    main.cpp \
    mainwindow.cpp \
    propertyeditor/qtpropertybrowser/qtbuttonpropertybrowser.cpp \
    propertyeditor/qtpropertybrowser/qteditorfactory.cpp \
    propertyeditor/qtpropertybrowser/qtgroupboxpropertybrowser.cpp \
    propertyeditor/qtpropertybrowser/qtpropertybrowser.cpp \
    propertyeditor/qtpropertybrowser/qtpropertybrowserutils.cpp \
    propertyeditor/qtpropertybrowser/qtpropertymanager.cpp \
    propertyeditor/qtpropertybrowser/qttreepropertybrowser.cpp \
    propertyeditor/qtpropertybrowser/qtvariantproperty.cpp \
    systempreferences.cpp

HEADERS += \
    ../../../../kernel/TraitsKernel.h \
    ../../../../kernel/simulator/Attribute.h \
    ../../../../kernel/simulator/ComponentManager.h \
    ../../../../kernel/simulator/ConnectionManager.h \
    ../../../../kernel/simulator/Counter.h \
    ../../../../kernel/simulator/CppSerializer.h \
    ../../../../kernel/simulator/DefineGetterSetter.h \
    ../../../../kernel/simulator/Entity.h \
    ../../../../kernel/simulator/EntityType.h \
    ../../../../kernel/simulator/Event.h \
    ../../../../kernel/simulator/ExperimentManager.h \
    ../../../../kernel/simulator/ExperimentManagerDefaultImpl1.h \
    ../../../../kernel/simulator/ExperimetManager_if.h \
    ../../../../kernel/simulator/GenSerializer.h \
    ../../../../kernel/simulator/GenesysPropertyIntrospection.h \
    ../../../../kernel/simulator/JsonSerializer.h \
    ../../../../kernel/simulator/LicenceManager.h \
    ../../../../kernel/simulator/Model.h \
    ../../../../kernel/simulator/ModelCheckerDefaultImpl1.h \
    ../../../../kernel/simulator/ModelChecker_if.h \
    ../../../../kernel/simulator/ModelComponent.h \
    ../../../../kernel/simulator/ModelDataDefinition.h \
    ../../../../kernel/simulator/ModelDataManager.h \
    ../../../../kernel/simulator/ModelInfo.h \
    ../../../../kernel/simulator/ModelManager.h \
    ../../../../kernel/simulator/ModelPersistenceDefaultImpl2.h \
    ../../../../kernel/simulator/ModelSerializer.h \
    ../../../../kernel/simulator/ModelSimulation.h \
    ../../../../kernel/simulator/OnEventManager.h \
    ../../../../kernel/simulator/ParserChangesInformation.h \
    ../../../../kernel/simulator/ParserDefaultImpl1.h \
    ../../../../kernel/simulator/ParserDefaultImpl2.h \
    ../../../../kernel/simulator/ParserManager.h \
    ../../../../kernel/simulator/Parser_if.h \
    ../../../../kernel/simulator/Persistence.h \
    ../../../../kernel/simulator/Plugin.h \
    ../../../../kernel/simulator/PluginConnector_if.h \
    ../../../../kernel/simulator/PluginInformation.h \
    ../../../../kernel/simulator/PluginManager.h \
    ../../../../kernel/simulator/PropertyGenesys.h \
    ../../../../kernel/simulator/ScenarioExperiment_if.h \
    ../../../../kernel/simulator/SimulationControlAndResponse.h \
    ../../../../kernel/simulator/SimulationExperiment.h \
    ../../../../kernel/simulator/SimulationReporterDefaultImpl1.h \
    ../../../../kernel/simulator/SimulationReporter_if.h \
    ../../../../kernel/simulator/SimulationScenario.h \
    ../../../../kernel/simulator/Simulator.h \
    ../../../../kernel/simulator/SinkModelComponent.h \
    ../../../../kernel/simulator/SourceModelComponent.h \
    ../../../../kernel/simulator/StatisticsCollector.h \
    ../../../../kernel/simulator/TraceManager.h \
    ../../../../kernel/simulator/XmlSerializer.h \
    ../../../../kernel/statistics/CollectorDatafileDefaultImpl1.h \
    ../../../../kernel/statistics/CollectorDatafile_if.h \
    ../../../../kernel/statistics/CollectorDefaultImpl1.h \
    ../../../../kernel/statistics/Collector_if.h \
    ../../../../kernel/statistics/SamplerDefaultImpl1.h \
    ../../../../kernel/statistics/Sampler_if.h \
    ../../../../kernel/statistics/SorttFile.h \
    ../../../../kernel/statistics/StatisticsDataFileDefaultImpl.h \
    ../../../../kernel/statistics/StatisticsDataFile_if.h \
    ../../../../kernel/statistics/StatisticsDefaultImpl1.h \
    ../../../../kernel/statistics/Statistics_if.h \
    ../../../../kernel/util/Exact.h \
    ../../../../kernel/util/List.h \
    ../../../../kernel/util/ListObservable.h \
    ../../../../kernel/util/Util.h \
    ../../../../parser/Genesys++-driver.h \
    ../../../../parser/GenesysParser.h \
    ../../../../parser/location.hh \
    ../../../../parser/obj_t.h \
    ../../../../parser/parserBisonFlex/bisonparser.yy \
    ../../../../parser/parserBisonFlex/lexerparser.ll \
    ../../../../parser/position.hh \
    ../../../../parser/stack.hh \
    ../../../../plugins/PluginConnectorDummyImpl1.h \
    ../../../../plugins/components/MaterialHandling/Access.h \
    ../../../../plugins/components/DiscreteProcessing/Assign.h \
    ../../../../plugins/components/Grouping/Batch.h \
    ../../../../plugins/components/DiscreteProcessing/Buffer.h \
    ../../../../plugins/components/DiscreteProcessing/Clone.h \
    ../../../../plugins/components/Logic/CppForG.h \
    ../../../../plugins/components/DiscreteProcessing/Create.h \
    ../../../../plugins/components/Decisions/Decide.h \
    ../../../../plugins/components/DiscreteProcessing/Delay.h \
    ../../../../plugins/components/Continuous/DiffEquations.h \
    ../../../../plugins/components/DiscreteProcessing/Dispose.h \
    ../../../../plugins/components/Decisions/DropOff.h \
    ../../../../plugins/components/DiscreteProcessing/DummyComponent.h \
    ../../../../plugins/components/MaterialHandling/Enter.h \
    ../../../../plugins/components/MaterialHandling/Exit.h \
    ../../../../plugins/components/Network/ModalModelDefault.h \
    ../../../../plugins/components/Network/ModalModelFSM.h \
    ../../../../plugins/components/Network/ModalModelPetriNet.h \
    ../../../../plugins/components/Decisions/PickableStationItem.h \
    ../../../../plugins/components/ElectronicsSimulation/SPICECircuit.h \
    ../../../../plugins/components/ElectronicsSimulation/SPICENode.h \
    ../../../../plugins/components/Decisions/Wait.h \
    ../../../../plugins/components/Continuous/LSODE.h \
    ../../../../plugins/components/MaterialHandling/Leave.h \
    ../../../../plugins/components/DiscreteProcessing/MarkovChain.h \
    ../../../../plugins/components/Decision/Match.h \
    ../../../../plugins/components/DiscreteProcessing/OLD_ODEelement.h \
    ../../../../plugins/components/Decisions/PickStation.h \
    ../../../../plugins/components/Decisions/PickUp.h \
    ../../../../plugins/components/DiscreteProcessing/Process.h \
    ../../../../plugins/components/DiscreteProcessing/QueueableItem.h \
    ../../../../plugins/components/InputOutput/Record.h \
    ../../../../plugins/components/DiscreteProcessing/Release.h \
    ../../../../plugins/components/Decisions/Remove.h \
    ../../../../plugins/components/MaterialHandling/Route.h \
    ../../../../plugins/components/Decisions/Search.h \
    ../../../../plugins/components/DiscreteProcessing/SeizableItem.h \
    ../../../../plugins/components/DiscreteProcessing/Seize.h \
    ../../../../plugins/components/Grouping/Separate.h \
    ../../../../plugins/components/Decisions/Signal.h \
    ../../../../plugins/components/MaterialHandling/Start.h \
    ../../../../plugins/components/MaterialHandling/Stop.h \
    ../../../../plugins/components/MaterialHandling/Store.h \
    ../../../../plugins/components/Logic/Submodel.h \
    ../../../../plugins/components/MaterialHandling/Unstore.h \
    ../../../../plugins/components/InputOutput/Write.h \
    ../../../../plugins/components/DiscreteProcessing/DefaultNode.h \
    ../../../../plugins/components/Network/DefaultTransitionExtensions.h \
    ../../../../plugins/components/Network/FSMState.h \
    ../../../../plugins/components/Network/PetriPlace.h \
    ../../../../plugins/data/DiscreteProcessing/AssignmentItem.h \
    ../../../../plugins/data/BiochemicalSimulation/BioNetwork.h \
    ../../../../plugins/data/BiochemicalSimulation/BioParameter.h \
    ../../../../plugins/data/BiochemicalSimulation/BioReaction.h \
    ../../../../plugins/data/BiochemicalSimulation/BioSpecies.h \
    ../../../../plugins/data/BiochemicalSimulation/BioSimulatorRunner.h \
    ../../../../plugins/data/DiscreteProcessing/CppCompiler.h \
    ../../../../plugins/data/DataDefinition/DummyElement.h \
    ../../../../plugins/data/DiscreteProcessing/EntityGroup.h \
    ../../../../plugins/data/DiscreteProcessing/Failure.h \
    ../../../../plugins/data/DiscreteProcessing/File.h \
    ../../../../plugins/data/DiscreteProcessing/Formula.h \
    ../../../../plugins/data/DiscreteProcessing/Label.h \
    ../../../../plugins/data/DiscreteProcessing/Queue.h \
    ../../../../plugins/data/DiscreteProcessing/Resource.h \
    ../../../../plugins/data/ElectronicsSimulation/SPICERunner.h \
    ../../../../plugins/data/DiscreteProcessing/Schedule.h \
    ../../../../plugins/data/DiscreteProcessing/Sequence.h \
    ../../../../plugins/data/DiscreteProcessing/Set.h \
    ../../../../plugins/data/DiscreteProcessing/SignalData.h \
    ../../../../plugins/data/DiscreteProcessing/Station.h \
    ../../../../plugins/data/DiscreteProcessing/Storage.h \
    ../../../../plugins/data/DiscreteProcessing/Variable.h \
    ../../../../tools/ContinuousDistribution_if.h \
    ../../../../tools/DataAnalyser_if.h \
    ../../../../tools/DataSet_if.h \
    ../../../../tools/DiscreteDistribution_if.h \
    ../../../../tools/Distribution_if.h \
    ../../../../tools/FactorialDesign/FactorialDesign.h \
    ../../../../tools/FitterDefaultImpl.h \
    ../../../../tools/FitterDummyImpl.h \
    ../../../../tools/Fitter_if.h \
    ../../../../tools/HypothesisTesterDefaultImpl1.h \
    ../../../../tools/HypothesisTester_if.h \
    ../../../../tools/MassActionOdeSystem.h \
    ../../../../tools/OdeSolver_if.h \
    ../../../../tools/OdeSystem_if.h \
    ../../../../tools/OptimizerDefaultImpl1.h \
    ../../../../tools/Optimizer_if.h \
    ../../../../tools/ProbabilityDistribution.h \
    ../../../../tools/ProbabilityDistributionBase.h \
    ../../../../tools/Quadrature_if.h \
    ../../../../tools/RootFinder_if.h \
    ../../../../tools/RungeKutta4OdeSolver.h \
    ../../../../tools/SolverDefaultImpl1.h \
    ../../../../tools/Solver_if.h \
    ../../../../tools/TraitsTools.h \
    ../../../BaseGenesysTerminalApplication.h \
    ../../../GenesysApplication_if.h \
    ../../../TraitsApp.h \
    ../../../terminal/GenesysShell/GenesysShell.h \
    ../../../terminal/GenesysShell/GenesysShell_if.h \
    ../../../terminal/TraitsTerminalApp.h \
    ../../../terminal/examples/arenaExamples/AirportSecurityExample.h \
    ../../../terminal/examples/arenaExamples/AirportSecurityExampleExtended.h \
    ../../../terminal/examples/arenaExamples/Airport_Extended1.h \
    ../../../terminal/examples/arenaExamples/Assembly_Line.h \
    ../../../terminal/examples/arenaExamples/Banking_Transactions.h \
    ../../../terminal/examples/arenaExamples/Example_BasicOrderShipping.h \
    ../../../terminal/examples/arenaExamples/Example_PortModel.h \
    ../../../terminal/examples/arenaExamples/Example_PublicTransport.h \
    ../../../terminal/examples/arenaSmarts/Smart_AddingResource.h \
    ../../../terminal/examples/arenaSmarts/Smart_AlternatingEntityCreation.h \
    ../../../terminal/examples/arenaSmarts/Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime.h \
    ../../../terminal/examples/arenaSmarts/Smart_ArrivalsEntityTypeVsAttribute.h \
    ../../../terminal/examples/arenaSmarts/Smart_AssignExample.h \
    ../../../terminal/examples/arenaSmarts/Smart_AutomaticStatisticsCollection.h \
    ../../../terminal/examples/arenaSmarts/Smart_BasicModeling.h \
    ../../../terminal/examples/arenaSmarts/Smart_BatchAndSeparate.h \
    ../../../terminal/examples/arenaSmarts/Smart_ContinuousFlowEntities.h \
    ../../../terminal/examples/arenaSmarts/Smart_Create.h \
    ../../../terminal/examples/arenaSmarts/Smart_DecideNWayByChance.h \
    ../../../terminal/examples/arenaSmarts/Smart_DefiningAttributesAsStrings.h \
    ../../../terminal/examples/arenaSmarts/Smart_DefiningControlLogic.h \
    ../../../terminal/examples/arenaSmarts/Smart_DefiningResourceCapacity.h \
    ../../../terminal/examples/arenaSmarts/Smart_DelayBasedOnReplication.h \
    ../../../terminal/examples/arenaSmarts/Smart_EntitiesProcessedByPriority.h \
    ../../../terminal/examples/arenaSmarts/Smart_EvaluatingConditionsBeforeEnteringQueue.h \
    ../../../terminal/examples/arenaSmarts/Smart_Expression.h \
    ../../../terminal/examples/arenaSmarts/Smart_InventoryAndHoldingCosts.h \
    ../../../terminal/examples/arenaSmarts/Smart_MaxArrivalsField.h \
    ../../../terminal/examples/arenaSmarts/Smart_ModelRunUntil1000Parts.h \
    ../../../terminal/examples/arenaSmarts/Smart_ModuleDisplayVariables.h \
    ../../../terminal/examples/arenaSmarts/Smart_OverlappingResources.h \
    ../../../terminal/examples/arenaSmarts/Smart_ParallelProcessingOfEntities.h \
    ../../../terminal/examples/arenaSmarts/Smart_PlacingEntitiesInQueueSets.h \
    ../../../terminal/examples/arenaSmarts/Smart_PriorityExample.h \
    ../../../terminal/examples/arenaSmarts/Smart_ProcessArena.h \
    ../../../terminal/examples/arenaSmarts/Smart_Record_Arena.h \
    ../../../terminal/examples/arenaSmarts/Smart_RemovingAndReorderingEntitiesInAQueue.h \
    ../../../terminal/examples/arenaSmarts/Smart_ResourceCosting.h \
    ../../../terminal/examples/arenaSmarts/Smart_ResourceScheduleCosting.h \
    ../../../terminal/examples/arenaSmarts/Smart_ResourceSets.h \
    ../../../terminal/examples/arenaSmarts/Smart_SeizingMultipleSimultaneosly.h \
    ../../../terminal/examples/arenaSmarts/Smart_SelectingRouteBasedOnProbability.h \
    ../../../terminal/examples/arenaSmarts/Smart_SelectingShorterQueue.h \
    ../../../terminal/examples/arenaSmarts/Smart_SynchronizingParallelEntities.h \
    ../../../terminal/examples/arenaSmarts/Smart_ValueAdded.h \
    ../../../terminal/examples/arenaSmarts/Smart_WaitForSignal.h \
    ../../../terminal/examples/book/Book_Cap02_Example01.h \
    ../../../terminal/examples/smarts/Smart_AssignWriteSeizes.h \
    ../../../terminal/examples/smarts/Smart_BatchSeparate.h \
    ../../../terminal/examples/smarts/Smart_Buffer.h \
    ../../../terminal/examples/smarts/Smart_Clone.h \
    ../../../terminal/examples/smarts/Smart_CppForG.h \
    ../../../terminal/examples/smarts/Smart_Delay.h \
    ../../../terminal/examples/smarts/Smart_Dummy.h \
    ../../../terminal/examples/smarts/Smart_Failures.h \
    ../../../terminal/examples/smarts/Smart_HoldSearchRemove.h \
    ../../../terminal/examples/smarts/Smart_LSODE.h \
    ../../../terminal/examples/smarts/Smart_MarkovChain.h \
    ../../../terminal/examples/smarts/Smart_Modalmodel.h \
    ../../../terminal/examples/smarts/Smart_ModelInfoModelSimulation.h \
    ../../../terminal/examples/smarts/Smart_ODE.h \
    ../../../terminal/examples/smarts/Smart_OnEvent.h \
    ../../../terminal/examples/smarts/Smart_Parser.h \
    ../../../terminal/examples/smarts/Smart_ParserModelFunctions.h \
    ../../../terminal/examples/smarts/Smart_Plugin.h \
    ../../../terminal/examples/smarts/Smart_Process.h \
    ../../../terminal/examples/smarts/Smart_ProcessSet.h \
    ../../../terminal/examples/smarts/Smart_Record.h \
    ../../../terminal/examples/smarts/Smart_RouteStation.h \
    ../../../terminal/examples/smarts/Smart_SeizeDelayRelease.h \
    ../../../terminal/examples/smarts/Smart_SeizeDelayReleaseMany.h \
    ../../../terminal/examples/smarts/Smart_SeizeDelayReleaseNoDataDefs.h \
    ../../../terminal/examples/smarts/Smart_Sequence.h \
    ../../../terminal/examples/smarts/Smart_SimulationControlResponse.h \
    ../../../terminal/examples/smarts/Smart_WaitScanCondition.h \
    ../../../terminal/examples/smarts/Smart_WaitSignal.h \
    ../../../terminal/examples/teaching/AnElectronicAssemblyAndTestSystem.h \
    ../../../terminal/examples/teaching/BufferFIFO.h \
    ../../../terminal/examples/teaching/ContinuousModel.h \
    ../../../terminal/examples/teaching/FullSimulationOfComplexModel.h \
    ../../../terminal/examples/teaching/Half_Adder.h \
    ../../../terminal/examples/teaching/Loja/Loja01.h \
    ../../../terminal/examples/teaching/OperatingSystem02.h \
    ../../../terminal/examples/teaching/OperatingSystem03.h \
    ../../../terminal/examples/teaching/Rectifier.h \
    ../../../web/BaseGenesysWebApplication.h \
    ../../../web/api/ApiRouter.h \
    ../../../web/auth/TokenService.h \
    ../../../web/http/HttpRequest.h \
    ../../../web/http/HttpResponse.h \
    ../../../web/http/SimpleHttpServer.h \
    ../../../web/service/SimulatorSessionService.h \
    ../../../web/session/SessionContext.h \
    ../../../web/session/SessionManager.h \
    ../../../web/worker/WorkerJob.h \
    ../../../web/worker/WorkerJobManager.h \
    codeeditor/CodeEditor.h \
    codeeditor/LineNumberArea.h \
    graphicals/ModelGraphicsScene.h \
    graphicals/ModelGraphicsView.h \
    propertyeditor/ObjectPropertyBrowser.h \
    propertyeditor/PropertyEditor.h \
    propertyeditor/DataComponentProperty.h \
    propertyeditor/DataComponentEditor.h \
    propertyeditor/ComboBoxEnum.h \
    TraitsGUI.h \
    UtilGUI.h \
    controllers/SimulationController.h \
    # Phase-3 GUI refactor controller header for model-inspector responsibilities.
    controllers/ModelInspectorController.h \
    # Phase-4 GUI refactor controller headers for trace and simulation-event responsibilities.
    controllers/TraceConsoleController.h \
    controllers/SimulationEventController.h \
    # Phase-5 GUI refactor controller header for plugin-catalog responsibilities.
    controllers/PluginCatalogController.h \
    # Phase-6 GUI refactor controller header for property-editor responsibilities.
    controllers/PropertyEditorController.h \
    # Phase-7 GUI refactor controller header for model/application lifecycle responsibilities.
    controllers/ModelLifecycleController.h \
    # Phase-8 GUI refactor controller header for simulation-command responsibilities.
    controllers/SimulationCommandController.h \
    # Phase-9 GUI refactor controller header for edit-command responsibilities.
    controllers/EditCommandController.h \
    # Phase-10 GUI refactor controller header for scene/view/drawing responsibilities.
    controllers/SceneToolController.h \
    # Graphical context-menu controller header for canvas popup actions.
    controllers/GraphicalContextMenuController.h \
    # Phase-11 GUI refactor controller header for dialog/utility responsibilities.
    controllers/DialogUtilityController.h \
    # Phase-1 GUI refactor service headers.
    services/ModelLanguageSynchronizer.h \
    services/GraphvizModelExporter.h \
    services/CppModelExporter.h \
    services/GraphicalModelSerializer.h \
    services/GraphicalModelBuilder.h \
    actions/AddUndoCommand.h \
    actions/DeleteUndoCommand.h \
    actions/GroupUndoCommand.h \
    actions/MoveUndoCommand.h \
    actions/PasteUndoCommand.h \
    actions/UngroupUndoCommand.h \
    animations/AnimationCounter.h \
    animations/AnimationPlaceholder.h \
    animations/AnimationQueue.h \
    animations/AnimationTimer.h \
    animations/AnimationTransition.h \
    animations/AnimationVariable.h \
    dialogs/DialogFind.h \
    dialogs/DialogSelectCounter.h \
    dialogs/DialogSelectVariable.h \
    dialogs/DialogTimerConfigure.h \
    dialogs/Dialogmodelinformation.h \
    dialogs/dialogBreakpoint.h \
    dialogs/dialogpluginmanager.h \
    dialogs/dialogsimulationconfigure.h \
    dialogs/dialogsystempreferences.h \
    graphicals/GraphicalAnimateExpression.h \
    graphicals/GraphicalAssociation.h \
    graphicals/GraphicalComponentPort.h \
    graphicals/GraphicalConnection.h \
    graphicals/GraphicalDiagramConnection.h \
    graphicals/GraphicalImageAnimation.h \
    graphicals/GraphicalModelComponent.h \
    GuiCrashDiagnostics.h \
    GuiScopeTrace.h \
    graphicals/GraphicalModelDataDefinition.h \
    mainwindow.h \
    propertyeditor/qtpropertybrowser/QtAbstractEditorFactoryBase \
    propertyeditor/qtpropertybrowser/QtAbstractPropertyBrowser \
    propertyeditor/qtpropertybrowser/QtAbstractPropertyManager \
    propertyeditor/qtpropertybrowser/QtBoolPropertyManager \
    propertyeditor/qtpropertybrowser/QtBrowserItem \
    propertyeditor/qtpropertybrowser/QtButtonPropertyBrowser \
    propertyeditor/qtpropertybrowser/QtCharEditorFactory \
    propertyeditor/qtpropertybrowser/QtCharPropertyManager \
    propertyeditor/qtpropertybrowser/QtCheckBoxFactory \
    propertyeditor/qtpropertybrowser/QtColorEditorFactory \
    propertyeditor/qtpropertybrowser/QtColorPropertyManager \
    propertyeditor/qtpropertybrowser/QtCursorEditorFactory \
    propertyeditor/qtpropertybrowser/QtCursorPropertyManager \
    propertyeditor/qtpropertybrowser/QtDateEditFactory \
    propertyeditor/qtpropertybrowser/QtDatePropertyManager \
    propertyeditor/qtpropertybrowser/QtDateTimeEditFactory \
    propertyeditor/qtpropertybrowser/QtDateTimePropertyManager \
    propertyeditor/qtpropertybrowser/QtDoublePropertyManager \
    propertyeditor/qtpropertybrowser/QtDoubleSpinBoxFactory \
    propertyeditor/qtpropertybrowser/QtEnumEditorFactory \
    propertyeditor/qtpropertybrowser/QtEnumPropertyManager \
    propertyeditor/qtpropertybrowser/QtFlagPropertyManager \
    propertyeditor/qtpropertybrowser/QtFontEditorFactory \
    propertyeditor/qtpropertybrowser/QtFontPropertyManager \
    propertyeditor/qtpropertybrowser/QtGroupBoxPropertyBrowser \
    propertyeditor/qtpropertybrowser/QtGroupPropertyManager \
    propertyeditor/qtpropertybrowser/QtIntPropertyManager \
    propertyeditor/qtpropertybrowser/QtKeySequenceEditorFactory \
    propertyeditor/qtpropertybrowser/QtKeySequencePropertyManager \
    propertyeditor/qtpropertybrowser/QtLineEditFactory \
    propertyeditor/qtpropertybrowser/QtLocalePropertyManager \
    propertyeditor/qtpropertybrowser/QtPointFPropertyManager \
    propertyeditor/qtpropertybrowser/QtPointPropertyManager \
    propertyeditor/qtpropertybrowser/QtProperty \
    propertyeditor/qtpropertybrowser/QtRectFPropertyManager \
    propertyeditor/qtpropertybrowser/QtRectPropertyManager \
    propertyeditor/qtpropertybrowser/QtScrollBarFactory \
    propertyeditor/qtpropertybrowser/QtSizeFPropertyManager \
    propertyeditor/qtpropertybrowser/QtSizePolicyPropertyManager \
    propertyeditor/qtpropertybrowser/QtSizePropertyManager \
    propertyeditor/qtpropertybrowser/QtSliderFactory \
    propertyeditor/qtpropertybrowser/QtSpinBoxFactory \
    propertyeditor/qtpropertybrowser/QtStringPropertyManager \
    propertyeditor/qtpropertybrowser/QtTimeEditFactory \
    propertyeditor/qtpropertybrowser/QtTimePropertyManager \
    propertyeditor/qtpropertybrowser/QtTreePropertyBrowser \
    propertyeditor/qtpropertybrowser/QtVariantEditorFactory \
    propertyeditor/qtpropertybrowser/QtVariantProperty \
    propertyeditor/qtpropertybrowser/QtVariantPropertyManager \
    propertyeditor/qtpropertybrowser/qtbuttonpropertybrowser.h \
    propertyeditor/qtpropertybrowser/qteditorfactory.h \
    propertyeditor/qtpropertybrowser/qtgroupboxpropertybrowser.h \
    propertyeditor/qtpropertybrowser/qtpropertybrowser.h \
    propertyeditor/qtpropertybrowser/qtpropertybrowserutils_p.h \
    propertyeditor/qtpropertybrowser/qtpropertymanager.h \
    propertyeditor/qtpropertybrowser/qttreepropertybrowser.h \
    propertyeditor/qtpropertybrowser/qtvariantproperty.h \
    guithememanager.h \
    systempreferences.h

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
