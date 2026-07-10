/*
 * SimulatorFacade.h
 *
 * High-level non-owning facade over Simulator and its main composed services.
 *
 * This header delegates calls to Simulator, its managers, the current Model and
 * the current Model sub-services without forcing callers to navigate manually
 * through getPluginManager(), getModelManager(), getTraceManager(), etc.
 *
 * All methods tolerate a null _simulator or missing current model:
 *   - pointer returns  -> nullptr
 *   - bool returns     -> false
 *   - numeric returns  -> 0
 *   - string returns   -> ""
 *   - void methods     -> no-op
 *
 * Method prefixes resolve name conflicts between delegates:
 *   (none)            Simulator direct methods
 *   licence*          LicenceManager
 *   plugin*           PluginManager
 *   model*            ModelManager / Model
 *   info*             ModelInfo
 *   sim*              ModelSimulation
 *   data*             ModelDataManager
 *   component*        ModelComponentManager
 *   event*            OnEventManager
 *   trace*, setTrace* TraceManager
 *   parser*           ParserManager
 *   simulationExp*    ExperimentManager
 */

#ifndef SIMULATORFACADE_H
#define SIMULATORFACADE_H

#include "Simulator.h"
#include "LicenceManager.h"
#include "PluginManager.h"
#include "TraceManager.h"
#include "ParserManager.h"
#include "ParserChangesInformation.h"
#include "ExperimentManager.h"
#include "SimulationExperiment.h"
#include "OnEventManager.h"
#include "model/ModelManager.h"
#include "model/Model.h"
#include "model/ModelInfo.h"
#include "model/ModelSimulation.h"
#include "model/ModelDataManager.h"
#include "model/ModelComponentManager.h"
#include "model/ModelComponent.h"
#include "model/ModelDataDefinition.h"

/*!
 * \brief High-level non-owning facade over Simulator and its main composed services.
 *
 * Construct with a non-owning pointer to an existing Simulator instance.
 * All public methods delegate through the private helpers; none of those
 * helpers are exposed publicly.
 */
class SimulatorFacade {
public:
    explicit SimulatorFacade(Simulator* simulator) : _simulator(simulator) {}
    virtual ~SimulatorFacade() = default;

    // -------------------------------------------------------------------------
    // Simulator — direct methods
    // -------------------------------------------------------------------------

    std::string getVersion() const {
        return _simulator != nullptr ? _simulator->getVersion() : "";
    }

    unsigned int getVersionNumber() const {
        return _simulator != nullptr ? _simulator->getVersionNumber() : 0u;
    }

    std::string getName() const {
        return _simulator != nullptr ? _simulator->getName() : "";
    }

    // -------------------------------------------------------------------------
    // LicenceManager
    // -------------------------------------------------------------------------

    const std::string licenceShow() const {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->showLicence() : "";
    }

    const std::string licenceShowLimits() const {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->showLimits() : "";
    }

    const std::string licenceShowActivationCode() const {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->showActivationCode() : "";
    }

    bool licenceLookforActivationCode() {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->lookforActivationCode() : false;
    }

    bool licenceInsertActivationCode() {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->insertActivationCode() : false;
    }

    void licenceRemoveActivationCode() {
        LicenceManager* m = licenceManager_();
        if (m != nullptr) m->removeActivationCode();
    }

    unsigned int licenceGetModelComponentsLimit() {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->getModelComponentsLimit() : 0u;
    }

    unsigned int licenceGetModelDatasLimit() {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->getModelDatasLimit() : 0u;
    }

    unsigned int licenceGetEntityLimit() {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->getEntityLimit() : 0u;
    }

    unsigned int licenceGetHostsLimit() {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->getHostsLimit() : 0u;
    }

    unsigned int licenceGetThreadsLimit() {
        LicenceManager* m = licenceManager_();
        return m != nullptr ? m->getThreadsLimit() : 0u;
    }

    // -------------------------------------------------------------------------
    // PluginManager
    // -------------------------------------------------------------------------

    std::string showPlugins() {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->show() : "";
    }

    List<Plugin*>* completePluginsFieldsAndTemplates() {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->completePluginsFieldsAndTemplates() : nullptr;
    }

    bool pluginCheck(const std::string& dynamicLibraryFilename) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->check(dynamicLibraryFilename) : false;
    }

    SystemDependencyCheckResult pluginCheckSystemDependencies(const std::string& dynamicLibraryFilename) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->checkSystemDependencies(dynamicLibraryFilename) : SystemDependencyCheckResult{};
    }

    List<std::string>* discoverPluginFilenames() const {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->discoverPluginFilenames() : nullptr;
    }

    List<PluginLoadIssue>* getPluginLoadIssues() const {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->getPluginLoadIssues() : nullptr;
    }

    void clearPluginLoadIssues() {
        PluginManager* m = pluginManager_();
        if (m != nullptr) m->clearPluginLoadIssues();
    }

    void clearPluginLoadIssue(const std::string& dynamicLibraryFilename) {
        PluginManager* m = pluginManager_();
        if (m != nullptr) m->clearPluginLoadIssue(dynamicLibraryFilename);
    }

    Plugin* insertPlugin(const std::string& dynamicLibraryFilename) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->insert(dynamicLibraryFilename) : nullptr;
    }

    Plugin* insertPlugin(const std::string& dynamicLibraryFilename, const PluginInsertionOptions& options) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->insert(dynamicLibraryFilename, options) : nullptr;
    }

    bool removePlugin(const std::string& dynamicLibraryFilename) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->remove(dynamicLibraryFilename) : false;
    }

    bool removePlugin(Plugin* plugin) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->remove(plugin) : false;
    }

    Plugin* findPlugin(const std::string& pluginTypeName) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->find(pluginTypeName) : nullptr;
    }

    std::vector<std::string> getDataDefinitionPluginTypenames() const {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->getDataDefinitionPluginTypenames() : std::vector<std::string>{};
    }

    std::string pluginSourceIncludePathFor(const std::string& pluginTypeName) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->sourceIncludePathFor(pluginTypeName) : "";
    }

    List<Plugin*>* autoInsertPlugins(const std::string& pluginsListFilename, const bool lookForPluginsIfFilenameNotFound = true) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->autoInsertPlugins(pluginsListFilename, lookForPluginsIfFilenameNotFound) : nullptr;
    }

    List<Plugin*>* autoInsertPlugins(const std::string& pluginsListFilename, const bool lookForPluginsIfFilenameNotFound, const PluginInsertionOptions& options) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->autoInsertPlugins(pluginsListFilename, lookForPluginsIfFilenameNotFound, options) : nullptr;
    }

    List<Plugin*>* autoInsertPlugins() {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->autoInsertPlugins() : nullptr;
    }

    List<Plugin*>* autoInsertPlugins(const PluginInsertionOptions& options) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->autoInsertPlugins(options) : nullptr;
    }

    Plugin* firstPlugin() {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->front() : nullptr;
    }

    Plugin* nextPlugin() {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->next() : nullptr;
    }

    Plugin* lastPlugin() {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->last() : nullptr;
    }

    unsigned int pluginCount() {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->size() : 0u;
    }

    Plugin* getPluginAtRank(unsigned int rank) {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->getAtRank(rank) : nullptr;
    }

    ModelDataDefinition* newInstance(const std::string& pluginTypename, Model* model, std::string name = "") {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->newInstance(pluginTypename, model, name) : nullptr;
    }

    template <typename T>
    T* newInstance(Model* model, std::string name = "") {
        PluginManager* m = pluginManager_();
        return m != nullptr ? m->newInstance<T>(model, name) : nullptr;
    }

    // -------------------------------------------------------------------------
    // ModelManager
    // -------------------------------------------------------------------------

    Model* newModel() {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->newModel() : nullptr;
    }

    void insertModel(Model* model) {
        ModelManager* m = modelManager_();
        if (m != nullptr) m->insert(model);
    }

    void removeModel(Model* model) {
        ModelManager* m = modelManager_();
        if (m != nullptr) m->remove(model);
    }

    bool setCurrentModel(Model* model) {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->setCurrent(model) : false;
    }

    bool saveModel(std::string filename) {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->saveModel(filename) : false;
    }

    Model* loadModel(std::string filename) {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->loadModel(filename) : nullptr;
    }

    Model* createModelFromLanguage(std::string modelSpecification) {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->createFromLanguage(modelSpecification) : nullptr;
    }

    unsigned int modelCount() {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->size() : 0u;
    }

    std::vector<Model*> models() const {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->models() : std::vector<Model*>{};
    }

    bool hasModel(Model* model) const {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->hasModel(model) : false;
    }

    int indexOfModel(Model* model) const {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->indexOf(model) : -1;
    }

    Model* modelAt(unsigned int index) const {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->modelAt(index) : nullptr;
    }

    int currentModelIndex() const {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->currentIndex() : -1;
    }

    Model* firstModel() {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->front() : nullptr;
    }

    Model* lastModel() {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->last() : nullptr;
    }

    Model* currentModel() {
        return currentModel_();
    }

    Model* nextModel() {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->next() : nullptr;
    }

    Model* previousModel() {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->previous() : nullptr;
    }

    bool canGoNextModel() const {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->canGoNext() : false;
    }

    bool canGoPreviousModel() const {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->canGoPrevious() : false;
    }

    // -------------------------------------------------------------------------
    // TraceManager
    // -------------------------------------------------------------------------

    static std::string traceConvertEnumToStr(TraceManager::Level level) {
        return TraceManager::convertEnumToStr(level);
    }

    void addTraceHandler(traceListener traceListener) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceHandler(traceListener);
    }

    void addTraceReportHandler(traceListener traceReportListener) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceReportHandler(traceReportListener);
    }

    void addTraceSimulationHandler(traceSimulationListener traceSimulationListener) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceSimulationHandler(traceSimulationListener);
    }

    void addTraceErrorHandler(traceErrorListener traceErrorListener) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceErrorHandler(traceErrorListener);
    }

    template<typename Class>
    void addTraceHandler(Class* object, void (Class::*function)(TraceEvent)) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceHandler(object, function);
    }

    template<typename Class>
    void addTraceErrorHandler(Class* object, void (Class::*function)(TraceErrorEvent)) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceErrorHandler(object, function);
    }

    template<typename Class>
    void addTraceReportHandler(Class* object, void (Class::*function)(TraceEvent)) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceReportHandler(object, function);
    }

    template<typename Class>
    void addTraceSimulationHandler(Class* object, void (Class::*function)(TraceSimulationEvent)) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceSimulationHandler(object, function);
    }

    void addTraceSimulationExceptionRuleModelData(void* thisobject) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->addTraceSimulationExceptionRuleModelData(thisobject);
    }

    void beginTraceShutdown() {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->beginShutdown();
    }

    void trace(const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->trace(text, level);
    }

    void traceError(const std::string& text, const std::exception& e) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->traceError(text, e);
    }

    void traceError(const std::string& text, TraceManager::Level level = TraceManager::Level::L1_errorFatal) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->traceError(text, level);
    }

    void traceReport(const std::string& text, TraceManager::Level level = TraceManager::Level::L2_results) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->traceReport(text, level);
    }

    void traceSimulation(void* thisobject, double time, Entity* entity, ModelComponent* component, const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed, bool showAnyway = false) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->traceSimulation(thisobject, time, entity, component, text, level, showAnyway);
    }

    void traceSimulation(void* thisobject, const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed, bool showAnyway = false) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->traceSimulation(thisobject, text, level, showAnyway);
    }

    List<std::string>* traceGetErrorMessages() const {
        TraceManager* m = traceManager_();
        return m != nullptr ? m->errorMessages() : nullptr;
    }

    void setTraceLevel(TraceManager::Level traceLevel) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->setTraceLevel(traceLevel);
    }

    TraceManager::Level getTraceLevel() const {
        TraceManager* m = traceManager_();
        return m != nullptr ? m->getTraceLevel() : TraceManager::Level::L0_noTraces;
    }

    void setTraceSimulationRuleAllAllowed(bool traceSimulationRuleAllAllowed) {
        TraceManager* m = traceManager_();
        if (m != nullptr) m->setTraceSimulationRuleAllAllowed(traceSimulationRuleAllAllowed);
    }

    bool isTraceSimulationRuleAllAllowed() const {
        TraceManager* m = traceManager_();
        return m != nullptr ? m->isTraceSimulationRuleAllAllowed() : false;
    }

    // -------------------------------------------------------------------------
    // ParserManager
    // -------------------------------------------------------------------------

    ParserManager::GenerateNewParserResult parserGenerateNewParser(ParserChangesInformation* changes) {
        ParserManager* m = parserManager_();
        return m != nullptr ? m->generateNewParser(changes) : ParserManager::GenerateNewParserResult{};
    }

    bool parserConnectNewParser(ParserManager::NewParser newParser) {
        ParserManager* m = parserManager_();
        return m != nullptr ? m->connectNewParser(newParser) : false;
    }

    // -------------------------------------------------------------------------
    // ExperimentManager
    // -------------------------------------------------------------------------

    SimulationExperiment* newSimulationExperiment() {
        ExperimentManager* m = experimentManager_();
        return m != nullptr ? m->newSimulationExperiment() : nullptr;
    }

    void insertSimulationExperiment(SimulationExperiment* experiment) {
        ExperimentManager* m = experimentManager_();
        if (m != nullptr) m->insert(experiment);
    }

    void removeSimulationExperiment(SimulationExperiment* experiment) {
        ExperimentManager* m = experimentManager_();
        if (m != nullptr) m->remove(experiment);
    }

    void setCurrentSimulationExperiment(SimulationExperiment* experiment) {
        ExperimentManager* m = experimentManager_();
        if (m != nullptr) m->setCurrent(experiment);
    }

    bool saveSimulationExperiment(std::string filename) {
        ExperimentManager* m = experimentManager_();
        return m != nullptr ? m->saveSimulationExperiment(filename) : false;
    }

    bool loadSimulationExperiment(std::string filename) {
        ExperimentManager* m = experimentManager_();
        return m != nullptr ? m->loadSimulationExperiment(filename) : false;
    }

    unsigned int simulationExperimentCount() {
        ExperimentManager* m = experimentManager_();
        return m != nullptr ? m->size() : 0u;
    }

    SimulationExperiment* firstSimulationExperiment() {
        ExperimentManager* m = experimentManager_();
        return m != nullptr ? m->front() : nullptr;
    }

    SimulationExperiment* currentSimulationExperiment() {
        ExperimentManager* m = experimentManager_();
        return m != nullptr ? m->current() : nullptr;
    }

    SimulationExperiment* nextSimulationExperiment() {
        ExperimentManager* m = experimentManager_();
        return m != nullptr ? m->next() : nullptr;
    }

    List<SimulationExperiment*>* getSimulationExperiments() const {
        ExperimentManager* m = experimentManager_();
        return m != nullptr ? m->getExperiments() : nullptr;
    }

    // -------------------------------------------------------------------------
    // Model — operations on current model
    // -------------------------------------------------------------------------

    bool modelSave(std::string filename) {
        Model* model = currentModel_();
        return model != nullptr ? model->save(filename) : false;
    }

    bool modelLoad(std::string filename) {
        Model* model = currentModel_();
        return model != nullptr ? model->load(filename) : false;
    }

    bool modelCheck() {
        Model* model = currentModel_();
        return model != nullptr ? model->check() : false;
    }

    void modelClear() {
        Model* model = currentModel_();
        if (model != nullptr) model->clear();
    }

    std::string modelShowLanguage() {
        Model* model = currentModel_();
        return model != nullptr ? model->showLanguage() : "";
    }

    void modelShow() {
        Model* model = currentModel_();
        if (model != nullptr) model->show();
    }

    bool modelInsert(ModelDataDefinition* elemOrComp) {
        Model* model = currentModel_();
        return model != nullptr ? model->insert(elemOrComp) : false;
    }

    void modelRemove(ModelDataDefinition* elemOrComp) {
        Model* model = currentModel_();
        if (model != nullptr) model->remove(elemOrComp);
    }

    std::list<ModelDataDefinition*> modelCollectDataDefinitionsRemovedWith(const std::list<ModelDataDefinition*>& roots) const {
        Model* model = currentModel_();
        return model != nullptr ? model->collectDataDefinitionsRemovedWith(roots) : std::list<ModelDataDefinition*>{};
    }

    Entity* modelCreateEntity(std::string name, bool insertIntoModel = true) {
        Model* model = currentModel_();
        return model != nullptr ? model->createEntity(name, insertIntoModel) : nullptr;
    }

    void modelRemoveEntity(Entity* entity) {
        Model* model = currentModel_();
        if (model != nullptr) model->removeEntity(entity);
    }

    void modelSendEntityToComponent(Entity* entity, Connection* connection, double timeDelay = 0.0) {
        Model* model = currentModel_();
        if (model != nullptr) model->sendEntityToComponent(entity, connection, timeDelay);
    }

    void modelSendEntityToComponent(Entity* entity, ModelComponent* component, double timeDelay = 0.0, unsigned int componentinputPortNumber = 0) {
        Model* model = currentModel_();
        if (model != nullptr) model->sendEntityToComponent(entity, component, timeDelay, componentinputPortNumber);
    }

    double modelParseExpression(const std::string expression) {
        Model* model = currentModel_();
        return model != nullptr ? model->parseExpression(expression) : 0.0;
    }

    double modelParseExpression(const std::string expression, bool& success, std::string& errorMessage) {
        Model* model = currentModel_();
        if (model != nullptr) return model->parseExpression(expression, success, errorMessage);
        success = false; errorMessage = ""; return 0.0;
    }

    bool modelCheckExpression(const std::string expression, const std::string expressionName, std::string& errorMessage) {
        Model* model = currentModel_();
        return model != nullptr ? model->checkExpression(expression, expressionName, errorMessage) : false;
    }

    void modelCheckReferencesToDataDefinitions(std::string expression, std::map<std::string, std::list<std::string>*>* referencedDataDefinitions) {
        Model* model = currentModel_();
        if (model != nullptr) model->checkReferencesToDataDefinitions(expression, referencedDataDefinitions);
    }

    Util::identification modelGetId() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getId() : static_cast<Util::identification>(0);
    }

    bool modelHasChanged() const {
        Model* model = currentModel_();
        return model != nullptr ? model->hasChanged() : false;
    }

    void modelSetHasChanged(bool hasChanged) {
        Model* model = currentModel_();
        if (model != nullptr) model->setHasChanged(hasChanged);
    }

    List<Event*>* modelGetFutureEvents() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getFutureEvents() : nullptr;
    }

    List<SimulationControl*>* modelGetControls() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getControls() : nullptr;
    }

    List<SimulationResponse*>* modelGetResponses() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getResponses() : nullptr;
    }

    Persistence_if* modelGetPersistence() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getPersistence() : nullptr;
    }

    unsigned int modelGetLevel() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getLevel() : 0u;
    }

    // -------------------------------------------------------------------------
    // ModelInfo — metadata of current model
    // -------------------------------------------------------------------------

    std::string infoShow() {
        ModelInfo* info = modelInfo_();
        return info != nullptr ? info->show() : "";
    }

    void infoSetName(std::string name) {
        ModelInfo* info = modelInfo_();
        if (info != nullptr) info->setName(name);
    }

    std::string infoGetName() const {
        ModelInfo* info = modelInfo_();
        return info != nullptr ? info->getName() : "";
    }

    void infoSetAnalystName(std::string analystName) {
        ModelInfo* info = modelInfo_();
        if (info != nullptr) info->setAnalystName(analystName);
    }

    std::string infoGetAnalystName() const {
        ModelInfo* info = modelInfo_();
        return info != nullptr ? info->getAnalystName() : "";
    }

    void infoSetDescription(std::string description) {
        ModelInfo* info = modelInfo_();
        if (info != nullptr) info->setDescription(description);
    }

    std::string infoGetDescription() const {
        ModelInfo* info = modelInfo_();
        return info != nullptr ? info->getDescription() : "";
    }

    void infoSetProjectTitle(std::string projectTitle) {
        ModelInfo* info = modelInfo_();
        if (info != nullptr) info->setProjectTitle(projectTitle);
    }

    std::string infoGetProjectTitle() const {
        ModelInfo* info = modelInfo_();
        return info != nullptr ? info->getProjectTitle() : "";
    }

    void infoSetVersion(std::string version) {
        ModelInfo* info = modelInfo_();
        if (info != nullptr) info->setVersion(version);
    }

    std::string infoGetVersion() const {
        ModelInfo* info = modelInfo_();
        return info != nullptr ? info->getVersion() : "";
    }

    void infoLoadInstance(PersistenceRecord* fields) {
        ModelInfo* info = modelInfo_();
        if (info != nullptr) info->loadInstance(fields);
    }

    void infoSaveInstance(PersistenceRecord* fields) {
        ModelInfo* info = modelInfo_();
        if (info != nullptr) info->saveInstance(fields);
    }

    bool infoHasChanged() const {
        ModelInfo* info = modelInfo_();
        return info != nullptr ? info->hasChanged() : false;
    }

    void infoSetHasChanged(bool hasChanged) {
        ModelInfo* info = modelInfo_();
        if (info != nullptr) info->setHasChanged(hasChanged);
    }

    // -------------------------------------------------------------------------
    // ModelSimulation — simulation control on current model
    // -------------------------------------------------------------------------

    std::string simShow() {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->show() : "";
    }

    void simStart() {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->start();
    }

    void simPause() {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->pause();
    }

    void simStep() {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->step();
    }

    void simStop() {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->stop();
    }

    void simSetNumberOfReplications(unsigned int numberOfReplications) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setNumberOfReplications(numberOfReplications);
    }

    unsigned int simGetNumberOfReplications() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getNumberOfReplications() : 0u;
    }

    void simSetReplicationLength(double replicationLength, Util::TimeUnit replicationLengthTimeUnit = Util::TimeUnit::unknown) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setReplicationLength(replicationLength, replicationLengthTimeUnit);
    }

    double simGetReplicationLength() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getReplicationLength() : 0.0;
    }

    void simSetReplicationLengthTimeUnit(Util::TimeUnit replicationLengthTimeUnit) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setReplicationLengthTimeUnit(replicationLengthTimeUnit);
    }

    Util::TimeUnit simGetReplicationLengthTimeUnit() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getReplicationLengthTimeUnit() : Util::TimeUnit::unknown;
    }

    void simSetReplicationReportBaseTimeUnit(Util::TimeUnit replicationReportBaseTimeUnit) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setReplicationReportBaseTimeUnit(replicationReportBaseTimeUnit);
    }

    Util::TimeUnit simGetReplicationBaseTimeUnit() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getReplicationBaseTimeUnit() : Util::TimeUnit::unknown;
    }

    void simSetWarmUpPeriod(double warmUpPeriod, Util::TimeUnit warmUpPeriodTimeUnit = Util::TimeUnit::unknown) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setWarmUpPeriod(warmUpPeriod, warmUpPeriodTimeUnit);
    }

    double simGetWarmUpPeriod() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getWarmUpPeriod() : 0.0;
    }

    void simSetWarmUpPeriodTimeUnit(Util::TimeUnit warmUpPeriodTimeUnit) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setWarmUpPeriodTimeUnit(warmUpPeriodTimeUnit);
    }

    Util::TimeUnit simGetWarmUpPeriodTimeUnit() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getWarmUpPeriodTimeUnit() : Util::TimeUnit::unknown;
    }

    void simSetTerminatingCondition(std::string terminatingCondition) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setTerminatingCondition(terminatingCondition);
    }

    std::string simGetTerminatingCondition() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getTerminatingCondition() : "";
    }

    void simSetPauseOnEvent(bool pauseOnEvent) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setPauseOnEvent(pauseOnEvent);
    }

    bool simIsPauseOnEvent() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isPauseOnEvent() : false;
    }

    void simSetStepByStep(bool stepByStep) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setStepByStep(stepByStep);
    }

    bool simIsStepByStep() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isStepByStep() : false;
    }

    void simSetInitializeStatistics(bool initializeStatistics) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setInitializeStatistics(initializeStatistics);
    }

    bool simIsInitializeStatistics() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isInitializeStatistics() : false;
    }

    void simSetInitializeSystem(bool initializeSystem) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setInitializeSystem(initializeSystem);
    }

    bool simIsInitializeSystem() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isInitializeSystem() : false;
    }

    void simSetPauseOnReplication(bool pauseBetweenReplications) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setPauseOnReplication(pauseBetweenReplications);
    }

    bool simIsPauseOnReplication() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isPauseOnReplication() : false;
    }

    void simSetReporter(SimulationReporter_if* simulationReporter) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setReporter(simulationReporter);
    }

    SimulationReporter_if* simGetReporter() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getReporter() : nullptr;
    }

    double simGetSimulatedTime() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getSimulatedTime() : 0.0;
    }

    bool simIsRunning() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isRunning() : false;
    }

    bool simIsPaused() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isPaused() : false;
    }

    unsigned int simGetCurrentReplicationNumber() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getCurrentReplicationNumber() : 0u;
    }

    void simSetShowReportsAfterReplication(bool showReportsAfterReplication) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setShowReportsAfterReplication(showReportsAfterReplication);
    }

    bool simIsShowReportsAfterReplication() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isShowReportsAfterReplication() : false;
    }

    void simSetShowReportsAfterSimulation(bool showReportsAfterSimulation) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setShowReportsAfterSimulation(showReportsAfterSimulation);
    }

    bool simIsShowReportsAfterSimulation() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isShowReportsAfterSimulation() : false;
    }

    List<double>* simGetBreakpointsOnTime() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getBreakpointsOnTime() : nullptr;
    }

    List<Entity*>* simGetBreakpointsOnEntity() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getBreakpointsOnEntity() : nullptr;
    }

    List<ModelComponent*>* simGetBreakpointsOnComponent() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getBreakpointsOnComponent() : nullptr;
    }

    const List<ModelDataDefinition*>* simGetSimulationStatisticsAggregates() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getSimulationStatisticsAggregates() : nullptr;
    }

    void simLoadInstance(PersistenceRecord* fields) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->loadInstance(fields);
    }

    void simSaveInstance(PersistenceRecord* fields, bool saveDefaults) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->saveInstance(fields, saveDefaults);
    }

    Event* simGetCurrentEvent() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->getCurrentEvent() : nullptr;
    }

    void simSetShowSimulationResposesInReport(bool showSimulationResposesInReport) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setShowSimulationResposesInReport(showSimulationResposesInReport);
    }

    bool simIsShowSimulationResposesInReport() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isShowSimulationResposesInReport() : false;
    }

    void simSetShowSimulationControlsInReport(bool showSimulationControlsInReport) {
        ModelSimulation* s = modelSimulation_();
        if (s != nullptr) s->setShowSimulationControlsInReport(showSimulationControlsInReport);
    }

    bool simIsShowSimulationControlsInReport() const {
        ModelSimulation* s = modelSimulation_();
        return s != nullptr ? s->isShowSimulationControlsInReport() : false;
    }

    // -------------------------------------------------------------------------
    // ModelDataManager — data definitions in current model
    // -------------------------------------------------------------------------

    bool dataInsert(ModelDataDefinition* anElement) {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->insert(anElement) : false;
    }

    void dataRemove(ModelDataDefinition* anElement) {
        ModelDataManager* m = modelDataManager_();
        if (m != nullptr) m->remove(anElement);
    }

    bool dataInsert(std::string datadefinitionTypename, ModelDataDefinition* anElement) {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->insert(datadefinitionTypename, anElement) : false;
    }

    void dataRemove(std::string datadefinitionTypename, ModelDataDefinition* anElement) {
        ModelDataManager* m = modelDataManager_();
        if (m != nullptr) m->remove(datadefinitionTypename, anElement);
    }

    bool dataCheck(std::string datadefinitionTypename, ModelDataDefinition* anElement, std::string expressionName, std::string& errorMessage) {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->check(datadefinitionTypename, anElement, expressionName, errorMessage) : false;
    }

    bool dataCheck(std::string datadefinitionTypename, std::string elementName, std::string expressionName, bool mandatory, std::string& errorMessage) {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->check(datadefinitionTypename, elementName, expressionName, mandatory, errorMessage) : false;
    }

    void dataClear() {
        ModelDataManager* m = modelDataManager_();
        if (m != nullptr) m->clear();
    }

    ModelDataDefinition* dataGetDataDefinition(std::string datadefinitionTypename, Util::identification id) {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->getDataDefinition(datadefinitionTypename, id) : nullptr;
    }

    ModelDataDefinition* dataGetDataDefinition(std::string datadefinitionTypename, std::string name) {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->getDataDefinition(datadefinitionTypename, name) : nullptr;
    }

    unsigned int dataGetNumberOfDataDefinitions(std::string datadefinitionTypename) {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->getNumberOfDataDefinitions(datadefinitionTypename) : 0u;
    }

    unsigned int dataGetNumberOfDataDefinitions() {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->getNumberOfDataDefinitions() : 0u;
    }

    int dataGetRankOf(std::string datadefinitionTypename, std::string name) {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->getRankOf(datadefinitionTypename, name) : -1;
    }

    std::list<std::string> dataGetDataDefinitionClassnames() const {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->getDataDefinitionClassnames() : std::list<std::string>{};
    }

    List<ModelDataDefinition*>* dataGetDataDefinitionList(std::string datadefinitionTypename) const {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->getDataDefinitionList(datadefinitionTypename) : nullptr;
    }

    void dataShow() {
        ModelDataManager* m = modelDataManager_();
        if (m != nullptr) m->show();
    }

    bool dataHasChanged() const {
        ModelDataManager* m = modelDataManager_();
        return m != nullptr ? m->hasChanged() : false;
    }

    void dataSetHasChanged(bool hasChanged) {
        ModelDataManager* m = modelDataManager_();
        if (m != nullptr) m->setHasChanged(hasChanged);
    }

    // -------------------------------------------------------------------------
    // ModelComponentManager — components in current model
    // -------------------------------------------------------------------------

    bool componentInsert(ModelComponent* comp) {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->insert(comp) : false;
    }

    void componentRemove(ModelComponent* comp) {
        ModelComponentManager* m = modelComponentManager_();
        if (m != nullptr) m->remove(comp);
    }

    ModelComponent* componentFind(std::string name) {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->find(name) : nullptr;
    }

    ModelComponent* componentFind(Util::identification id) {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->find(id) : nullptr;
    }

    void componentClear() {
        ModelComponentManager* m = modelComponentManager_();
        if (m != nullptr) m->clear();
    }

    unsigned int componentGetNumberOfComponents() {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->getNumberOfComponents() : 0u;
    }

    std::list<ModelComponent*>::iterator componentBegin() {
        ModelComponentManager* m = modelComponentManager_();
        static std::list<ModelComponent*> _empty;
        return m != nullptr ? m->begin() : _empty.begin();
    }

    std::list<ModelComponent*>::iterator componentEnd() {
        ModelComponentManager* m = modelComponentManager_();
        static std::list<ModelComponent*> _empty;
        return m != nullptr ? m->end() : _empty.end();
    }

    ModelComponent* componentFront() {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->front() : nullptr;
    }

    ModelComponent* componentNext() {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->next() : nullptr;
    }

    bool componentHasChanged() const {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->hasChanged() : false;
    }

    void componentSetHasChanged(bool hasChanged) {
        ModelComponentManager* m = modelComponentManager_();
        if (m != nullptr) m->setHasChanged(hasChanged);
    }

    std::list<SourceModelComponent*>* componentGetSourceComponents() {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->getSourceComponents() : nullptr;
    }

    std::list<ModelComponent*>* componentGetTransferInComponents() {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->getTransferInComponents() : nullptr;
    }

    std::list<ModelComponent*>* componentGetAllComponents() const {
        ModelComponentManager* m = modelComponentManager_();
        return m != nullptr ? m->getAllComponents() : nullptr;
    }

    // -------------------------------------------------------------------------
    // OnEventManager — event handlers on current model
    // -------------------------------------------------------------------------

    void eventAddOnModelCheckSucessHandler(modelEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnModelCheckSucessHandler(EventHandler);
    }

    void eventAddOnModelLoadHandler(modelEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnModelLoadHandler(EventHandler);
    }

    void eventAddOnModelSaveHandler(modelEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnModelSaveHandler(EventHandler);
    }

    void eventAddOnReplicationStartHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnReplicationStartHandler(EventHandler);
    }

    void eventAddOnReplicationStepHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnReplicationStepHandler(EventHandler);
    }

    void eventAddOnReplicationEndHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnReplicationEndHandler(EventHandler);
    }

    void eventAddOnProcessEventHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnProcessEventHandler(EventHandler);
    }

    void eventAddOnAfterProcessEventHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnAfterProcessEventHandler(EventHandler);
    }

    void eventAddOnEntityCreateHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnEntityCreateHandler(EventHandler);
    }

    void eventAddOnEntityMoveHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnEntityMoveHandler(EventHandler);
    }

    void eventAddOnEntityRemoveHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnEntityRemoveHandler(EventHandler);
    }

    void eventAddOnSimulationStartHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnSimulationStartHandler(EventHandler);
    }

    void eventAddOnSimulationPausedHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnSimulationPausedHandler(EventHandler);
    }

    void eventAddOnSimulationResumeHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnSimulationResumeHandler(EventHandler);
    }

    void eventAddOnSimulationEndHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnSimulationEndHandler(EventHandler);
    }

    void eventAddOnBreakpointHandler(simulationEventHandler EventHandler) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnBreakpointHandler(EventHandler);
    }

    template<typename Class>
    void eventAddOnModelCheckSuccessHandler(Class* object, void (Class::*function)(ModelEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnModelCheckSuccessHandler(object, function);
    }

    template<typename Class>
    void eventAddOnModelLoadHandler(Class* object, void (Class::*function)(ModelEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnModelLoadHandler(object, function);
    }

    template<typename Class>
    void eventAddOnModelSaveHandler(Class* object, void (Class::*function)(ModelEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnModelSaveHandler(object, function);
    }

    template<typename Class>
    void eventAddOnReplicationStartHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnReplicationStartHandler(object, function);
    }

    template<typename Class>
    void eventAddOnReplicationStepHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnReplicationStepHandler(object, function);
    }

    template<typename Class>
    void eventAddOnReplicationEndHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnReplicationEndHandler(object, function);
    }

    template<typename Class>
    void eventAddOnProcessEventHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnProcessEventHandler(object, function);
    }

    template<typename Class>
    void eventAddOnAfterProcessEventHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnAfterProcessEventHandler(object, function);
    }

    template<typename Class>
    void eventAddOnEntityCreateHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnEntityCreateHandler(object, function);
    }

    template<typename Class>
    void eventAddOnEntityMoveHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnEntityMoveHandler(object, function);
    }

    template<typename Class>
    void eventAddOnEntityRemoveHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnEntityRemoveHandler(object, function);
    }

    template<typename Class>
    void eventAddOnSimulationStartHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnSimulationStartHandler(object, function);
    }

    template<typename Class>
    void eventAddOnSimulationPausedHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnSimulationPausedHandler(object, function);
    }

    template<typename Class>
    void eventAddOnSimulationResumeHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnSimulationResumeHandler(object, function);
    }

    template<typename Class>
    void eventAddOnSimulationEndHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnSimulationEndHandler(object, function);
    }

    template<typename Class>
    void eventAddOnBreakpointHandler(Class* object, void (Class::*function)(SimulationEvent*)) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->addOnBreakpointHandler(object, function);
    }

    void eventNotifyModelCheckSuccessHandlers(ModelEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyModelCheckSuccessHandlers(se);
    }

    void eventNotifyModelLoadHandlers(ModelEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyModelLoadHandlers(se);
    }

    void eventNotifyModelSaveHandlers(ModelEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyModelSaveHandlers(se);
    }

    void eventNotifyReplicationStartHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyReplicationStartHandlers(se);
    }

    void eventNotifyReplicationStepHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyReplicationStepHandlers(se);
    }

    void eventNotifyReplicationEndHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyReplicationEndHandlers(se);
    }

    void eventNotifyProcessEventHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyProcessEventHandlers(se);
    }

    void eventNotifyAfterProcessEventHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyAfterProcessEventHandlers(se);
    }

    void eventNotifyEntityCreateHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyEntityCreateHandlers(se);
    }

    void eventNotifyEntityMoveHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyEntityMoveHandlers(se);
    }

    void eventNotifyEntityRemoveHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyEntityRemoveHandlers(se);
    }

    void eventNotifySimulationStartHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifySimulationStartHandlers(se);
    }

    void eventNotifySimulationPausedHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifySimulationPausedHandlers(se);
    }

    void eventNotifySimulationResumeHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifySimulationResumeHandlers(se);
    }

    void eventNotifySimulationEndHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifySimulationEndHandlers(se);
    }

    void eventNotifyBreakpointHandlers(SimulationEvent* se) {
        OnEventManager* m = onEventManager_();
        if (m != nullptr) m->NotifyBreakpointHandlers(se);
    }

private:
    Simulator* _simulator;

    LicenceManager* licenceManager_() const {
        return _simulator != nullptr ? _simulator->getLicenceManager() : nullptr;
    }

    PluginManager* pluginManager_() const {
        return _simulator != nullptr ? _simulator->getPluginManager() : nullptr;
    }

    ModelManager* modelManager_() const {
        return _simulator != nullptr ? _simulator->getModelManager() : nullptr;
    }

    TraceManager* traceManager_() const {
        return _simulator != nullptr ? _simulator->getTraceManager() : nullptr;
    }

    ParserManager* parserManager_() const {
        return _simulator != nullptr ? _simulator->getParserManager() : nullptr;
    }

    ExperimentManager* experimentManager_() const {
        return _simulator != nullptr ? _simulator->getExperimentManager() : nullptr;
    }

    Model* currentModel_() const {
        ModelManager* m = modelManager_();
        return m != nullptr ? m->current() : nullptr;
    }

    ModelInfo* modelInfo_() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getInfos() : nullptr;
    }

    ModelSimulation* modelSimulation_() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getSimulation() : nullptr;
    }

    ModelDataManager* modelDataManager_() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getDataManager() : nullptr;
    }

    ModelComponentManager* modelComponentManager_() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getComponentManager() : nullptr;
    }

    OnEventManager* onEventManager_() const {
        Model* model = currentModel_();
        return model != nullptr ? model->getOnEventManager() : nullptr;
    }
};

#endif /* SIMULATORFACADE_H */
