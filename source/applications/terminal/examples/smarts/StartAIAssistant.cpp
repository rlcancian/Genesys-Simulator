#include "StartAIAssistant.h"

#include <cstdlib>
#include <iostream>
#include <string>

#include "../../../TraitsApp.h"

#include "kernel/simulator/Simulator.h"

#include "plugins/components/AI/AIAssistant.h"
#include "plugins/components/DiscreteProcessing/Delay.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/Logic/Assign.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/data/AI/AISupport.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

namespace {
std::string envOr(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value != nullptr && *value != '\0') ? std::string(value) : fallback;
}
}

StartAIAssistant::StartAIAssistant() {
}

int StartAIAssistant::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys->getTraceManager());

    PluginManager* plugins = genesys->getPluginManager();
    plugins->autoInsertPlugins("autoloadplugins.txt");

    Model* model = genesys->getModelManager()->newModel();

    Create* create = plugins->newInstance<Create>(model, "CreateOrders");
    create->setEntityTypeName("Order");
    create->setTimeBetweenCreationsExpression("unif(1,3)", Util::TimeUnit::second);
    create->setFirstCreation(0.0);
    create->setEntitiesPerCreation(1);
    create->setMaxCreations(6);

    Assign* assign = plugins->newInstance<Assign>(model, "PrepareOrder");
    assign->getAssignments()->insert(new Assignment("tempo_processamento", "unif(4,12)"));
    assign->getAssignments()->insert(new Assignment("prioridade", "unif(0,1)"));

    Queue* queue = plugins->newInstance<Queue>(model, "Queue_1");
    queue->setOrderRule(Queue::OrderRule::FIFO);

    Resource* resource = plugins->newInstance<Resource>(model, "Resource_1");
    resource->setCapacity(1);

    Seize* seize = plugins->newInstance<Seize>(model, "HoldResource");
    seize->setQueueableItem(new QueueableItem(queue));
    seize->addRequest(new SeizableItem(resource, "1"));
    seize->setAllocationType(Util::AllocationType::Transfer);
    seize->setPriorityExpression("prioridade");

    Delay* preAiDelay = plugins->newInstance<Delay>(model, "PreAIHold");
    preAiDelay->setDelayExpression("tempo_processamento / 2");
    preAiDelay->setDelayTimeUnit(Util::TimeUnit::second);

    AISupport* support = plugins->newInstance<AISupport>(model, "AISupport_1");
    support->setProvider(envOr("GENESYS_AI_PROVIDER", "Local"));
    support->setModel(envOr("GENESYS_AI_MODEL", "llama3.2"));
    support->setBaseUrl(envOr("GENESYS_AI_BASE_URL", ""));
    support->setApiKeyEnvironmentVariable(envOr("GENESYS_AI_API_KEY_ENV", ""));
    support->setSystemInstructions(envOr(
        "GENESYS_AI_SYSTEM_PROMPT",
        "You are a decision service inside a GenESyS simulation. Return only valid JSON with action, confidence, explanation, and numeric values."));
    support->setConversationEnabled(true);
    support->setMaxHistoryMessages(10);
    support->setClearHistoryBetweenReplications(true);

    AIAssistant* assistant = plugins->newInstance<AIAssistant>(model, "RouteWithAI");
    assistant->setAISupport(support);
    assistant->setConversationScope("Expression");
    assistant->setConversationKeyExpression("Entity.Id");
    assistant->setPromptTemplate(
        "Entity {Entity.Id} has tempo_processamento={Entity.tempo_processamento}, "
        "prioridade={Entity.prioridade}, queue={NQ(Queue_1)}, "
        "resource_busy={MR(Resource_1)}, time_now={TNOW}. "
        "If the queue is long and the resource is busy, choose action \"deposit\". "
        "Otherwise choose action \"factory\". "
        "Return JSON with action, confidence, explanation, and numeric values.");
    assistant->setActionMappings("factory:0,deposit:1");
    assistant->setDelayExpression("0.25");
    assistant->setDefaultOutputPort(0);
    assistant->setErrorOutputPort(1);
    assistant->setMinimumConfidence(0.0);
    assistant->setSaveConfidenceAttribute("ai_confidence");
    assistant->setSaveSuccessAttribute("ai_success");

    Delay* factoryDelay = plugins->newInstance<Delay>(model, "FactoryDelay");
    factoryDelay->setDelayExpression("tempo_processamento");
    factoryDelay->setDelayTimeUnit(Util::TimeUnit::second);
    Release* factoryRelease = plugins->newInstance<Release>(model, "FactoryRelease");
    factoryRelease->addReleaseRequests(new SeizableItem(resource, "1"));
    Dispose* factoryDispose = plugins->newInstance<Dispose>(model, "FactoryDispose");

    Delay* depotDelay = plugins->newInstance<Delay>(model, "DepotDelay");
    depotDelay->setDelayExpression("tempo_processamento / 3");
    depotDelay->setDelayTimeUnit(Util::TimeUnit::second);
    Release* depotRelease = plugins->newInstance<Release>(model, "DepotRelease");
    depotRelease->addReleaseRequests(new SeizableItem(resource, "1"));
    Dispose* depotDispose = plugins->newInstance<Dispose>(model, "DepotDispose");

    create->connectTo(assign);
    assign->connectTo(seize);
    seize->connectTo(preAiDelay);
    preAiDelay->connectTo(assistant);
    assistant->getConnectionManager()->insert(factoryDelay);
    assistant->getConnectionManager()->insert(depotDelay);
    factoryDelay->connectTo(factoryRelease);
    factoryRelease->connectTo(factoryDispose);
    depotDelay->connectTo(depotRelease);
    depotRelease->connectTo(depotDispose);

    std::cout << "StartAIAssistant configured with provider=" << support->getProvider()
              << ", model=" << support->getModel()
              << ", baseUrl=" << (support->getBaseUrl().empty() ? "<default>" : support->getBaseUrl())
              << std::endl;

    model->getSimulation()->setReplicationLength(30, Util::TimeUnit::second);
    model->getSimulation()->setNumberOfReplications(1);
    model->save("./models/StartAIAssistant.gen");
    model->getSimulation()->start();

    delete genesys;
    return 0;
}
