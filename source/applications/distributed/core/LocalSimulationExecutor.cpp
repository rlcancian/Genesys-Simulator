#include "LocalSimulationExecutor.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Parser_if.h"
#include "kernel/simulator/essentialPlugins/Counter.h"
#include "kernel/simulator/essentialPlugins/StatisticsCollector.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelManager.h"
#include "kernel/simulator/model/ModelSimulation.h"
#include "kernel/statistics/Sampler_if.h"
#include "kernel/statistics/SamplerDefaultImpl1.h"
#include "kernel/statistics/Statistics_if.h"
#include "kernel/util/Util.h"

#include <exception>
#include <set>

namespace genesys::distributed {

namespace {

BatchResult failure(const std::string& error) {
    BatchResult result;
    result.success = false;
    result.error = error;
    return result;
}

// Applies the batch seed by mutating the sampler's own parameters (it owns them) and resetting.
void applySeed(Model* model, std::uint32_t seed) {
    if (model == nullptr) {
        return;
    }
    Parser_if* parser = model->getParser();
    if (parser == nullptr) {
        return;
    }
    auto* sampler = dynamic_cast<SamplerDefaultImpl1*>(parser->getSampler());
    if (sampler == nullptr) {
        return;
    }
    auto* params = dynamic_cast<SamplerDefaultImpl1::DefaultImpl1RNG_Parameters*>(sampler->getRNGparameters());
    if (params == nullptr) {
        return;
    }
    params->seed = seed;
    sampler->reset();
}

// Captures the simulation-level aggregates into the BatchResult, mirroring the worker capture.
void captureInto(Model* model, ModelSimulation* simulation, BatchResult& result) {
    std::set<std::string> counterNames;
    if (ModelDataManager* dataManager = model->getDataManager(); dataManager != nullptr) {
        if (List<ModelDataDefinition*>* counters = dataManager->getDataDefinitionList(Util::TypeOf<Counter>());
            counters != nullptr) {
            for (ModelDataDefinition* counterData : *counters->list()) {
                if (counterData != nullptr) {
                    counterNames.insert(counterData->getName());
                }
            }
        }
    }

    const List<ModelDataDefinition*>* aggregates = simulation->getSimulationStatisticsAggregates();
    if (aggregates == nullptr) {
        return;
    }
    for (ModelDataDefinition* data : *aggregates->list()) {
        StatisticsCollector* collector = dynamic_cast<StatisticsCollector*>(data);
        if (collector == nullptr) {
            continue;
        }
        Statistics_if* statistics = collector->getStatistics();
        if (statistics == nullptr) {
            continue;
        }

        const unsigned int numReplications = statistics->numElements();
        const double average = numReplications > 0 ? statistics->average() : 0.0;

        if (counterNames.count(collector->getName()) > 0) {
            CounterStat counter;
            counter.name = collector->getName();
            counter.total = average * static_cast<double>(numReplications);
            result.counters.push_back(counter);
        } else {
            CollectorStat stat;
            stat.name = collector->getName();
            stat.numReplications = numReplications;
            stat.average = average;
            stat.variance = numReplications >= 2 ? statistics->variance() : 0.0;
            stat.min = numReplications > 0 ? statistics->min() : 0.0;
            stat.max = numReplications > 0 ? statistics->max() : 0.0;
            stat.numObservations = numReplications;
            result.statistics.push_back(stat);
        }
    }
}

} // namespace

BatchResult LocalSimulationExecutor::execute(const DistributedSimulationJob& job) {
    Simulator simulator;
    ModelManager* modelManager = simulator.getModelManager();
    if (modelManager == nullptr) {
        return failure("local: unable to access model manager");
    }

    Model* model = modelManager->createFromLanguage(job.modelText);
    if (model == nullptr) {
        return failure("local: model specification could not be parsed");
    }

    ModelSimulation* simulation = model->getSimulation();
    if (simulation == nullptr) {
        return failure("local: unable to access model simulation");
    }

    simulation->setNumberOfReplications(job.batch.numberOfReplications);
    applySeed(model, job.batch.seed);

    try {
        simulation->start();
    } catch (const std::exception& exception) {
        return failure(std::string("local: simulation failed: ") + exception.what());
    } catch (...) {
        return failure("local: unexpected simulation failure");
    }

    BatchResult result;
    result.success = true;
    result.numberOfReplications = simulation->getNumberOfReplications();
    captureInto(model, simulation, result);
    return result;
}

} // namespace genesys::distributed
