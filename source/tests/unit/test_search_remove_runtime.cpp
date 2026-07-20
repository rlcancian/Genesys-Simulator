#include <gtest/gtest.h>

#include "kernel/simulator/Event.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/essentialPlugins/Attribute.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "plugins/components/Decisions/Remove.h"
#include "plugins/components/Decisions/Search.h"
#include "plugins/data/DiscreteProcessing/Queue.h"

#include <vector>

namespace {

class CollectorSinkComponentProbe final : public ModelComponent {
public:
    CollectorSinkComponentProbe(Model* model, const std::string& name)
        : ModelComponent(model, "CollectorSinkComponentProbe", name) {}

    const std::vector<Entity*>& receivedEntities() const {
        return _receivedEntities;
    }

protected:
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
        (void)inputPortNumber;
        _receivedEntities.push_back(entity);
    }

    bool _loadInstance(PersistenceRecord* fields) override {
        return ModelComponent::_loadInstance(fields);
    }

    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override {
        ModelComponent::_saveInstance(fields, saveDefaultValues);
    }

private:
    std::vector<Entity*> _receivedEntities;
};

class SearchProbe final : public Search {
public:
    SearchProbe(Model* model, const std::string& name)
        : Search(model, name) {}

    void dispatch(Entity* entity) {
        _onDispatchEvent(entity, 0u);
    }
};

class RemoveProbe final : public Remove {
public:
    RemoveProbe(Model* model, const std::string& name)
        : Remove(model, name) {}

    void dispatch(Entity* entity) {
        _onDispatchEvent(entity, 0u);
    }
};

void drainFutureEvents(Model* model) {
    ASSERT_NE(model, nullptr);
    while (!model->getFutureEvents()->empty()) {
        Event* event = model->getFutureEvents()->front();
        model->getFutureEvents()->pop_front();
        ModelComponent::DispatchEvent(event);
        delete event;
    }
}

} // namespace

TEST(SearchRemoveRuntimeTest, SearchQueueFindsEntityInRangeSavesRankAndRoutesToFoundPort) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SearchProbe search(model, "SearchFind");
    Queue queue(model, "SearchFindQueue");
    CollectorSinkComponentProbe notFoundSink(model, "SearchFindNotFound");
    CollectorSinkComponentProbe foundSink(model, "SearchFindFound");
    search.getConnectionManager()->insert(&notFoundSink);
    search.getConnectionManager()->insert(&foundSink);
    search.setSearchInType(Search::SearchInType::QUEUE);
    search.setSearchIn(&queue);
    search.setStartRank("1");
    search.setEndRank("3");
    search.setSearchCondition("1");
    search.setSaveFounRankAttribute("SearchFoundRankAttr");
    Attribute searchFoundRankAttribute(model, "SearchFoundRankAttr");

    CollectorSinkComponentProbe producer(model, "SearchFindProducer");
    queue.insertElement(new Waiting(model->createEntity("SearchFindQueueE0", true), 0.0, &producer));
    queue.insertElement(new Waiting(model->createEntity("SearchFindQueueE1", true), 0.0, &producer));
    queue.insertElement(new Waiting(model->createEntity("SearchFindQueueE2", true), 0.0, &producer));

    Entity* trigger = model->createEntity("SearchFindTrigger", true);
    search.dispatch(trigger);
    drainFutureEvents(model);

    EXPECT_DOUBLE_EQ(trigger->getAttributeValue("SearchFoundRankAttr"), 1.0);
    EXPECT_TRUE(notFoundSink.receivedEntities().empty());
    ASSERT_EQ(foundSink.receivedEntities().size(), 1u);
    EXPECT_EQ(foundSink.receivedEntities().front(), trigger);
}

TEST(SearchRemoveRuntimeTest, SearchQueueNotFoundRoutesToPortZeroAndSavesZeroRank) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SearchProbe search(model, "SearchNotFound");
    Queue queue(model, "SearchNotFoundQueue");
    CollectorSinkComponentProbe notFoundSink(model, "SearchNotFoundOut0");
    CollectorSinkComponentProbe foundSink(model, "SearchNotFoundOut1");
    search.getConnectionManager()->insert(&notFoundSink);
    search.getConnectionManager()->insert(&foundSink);
    search.setSearchInType(Search::SearchInType::QUEUE);
    search.setSearchIn(&queue);
    search.setStartRank("0");
    search.setEndRank("2");
    search.setSearchCondition("0");
    search.setSaveFounRankAttribute("SearchNotFoundRankAttr");
    Attribute searchNotFoundRankAttribute(model, "SearchNotFoundRankAttr");

    CollectorSinkComponentProbe producer(model, "SearchNotFoundProducer");
    queue.insertElement(new Waiting(model->createEntity("SearchNotFoundQueueE0", true), 0.0, &producer));
    queue.insertElement(new Waiting(model->createEntity("SearchNotFoundQueueE1", true), 0.0, &producer));

    Entity* trigger = model->createEntity("SearchNotFoundTrigger", true);
    search.dispatch(trigger);
    drainFutureEvents(model);

    EXPECT_DOUBLE_EQ(trigger->getAttributeValue("SearchNotFoundRankAttr"), 0.0);
    ASSERT_EQ(notFoundSink.receivedEntities().size(), 1u);
    EXPECT_EQ(notFoundSink.receivedEntities().front(), trigger);
    EXPECT_TRUE(foundSink.receivedEntities().empty());
}

TEST(SearchRemoveRuntimeTest, RemoveEqualStartAndEndRankRemovesExactlyOneAndRoutesCorrectly) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    RemoveProbe remove(model, "RemoveSingleRank");
    Queue queue(model, "RemoveSingleRankQueue");
    CollectorSinkComponentProbe mainSink(model, "RemoveSingleRankMain");
    CollectorSinkComponentProbe removedSink(model, "RemoveSingleRankRemoved");
    remove.getConnectionManager()->insert(&mainSink);
    remove.getConnectionManager()->insert(&removedSink);
    remove.setRemoveFromType(Remove::RemoveFromType::QUEUE);
    remove.setRemoveFrom(&queue);
    remove.setRemoveStartRank("1");
    remove.setRemoveEndRank("1");

    CollectorSinkComponentProbe producer(model, "RemoveSingleRankProducer");
    Entity* q0 = model->createEntity("RemoveSingleRankQ0", true);
    Entity* q1 = model->createEntity("RemoveSingleRankQ1", true);
    Entity* q2 = model->createEntity("RemoveSingleRankQ2", true);
    queue.insertElement(new Waiting(q0, 0.0, &producer));
    queue.insertElement(new Waiting(q1, 0.0, &producer));
    queue.insertElement(new Waiting(q2, 0.0, &producer));

    Entity* trigger = model->createEntity("RemoveSingleRankTrigger", true);
    remove.dispatch(trigger);
    drainFutureEvents(model);

    ASSERT_EQ(removedSink.receivedEntities().size(), 1u);
    EXPECT_EQ(removedSink.receivedEntities().front(), q1);
    EXPECT_EQ(queue.size(), 2u);
    ASSERT_EQ(mainSink.receivedEntities().size(), 1u);
    EXPECT_EQ(mainSink.receivedEntities().front(), trigger);
}

TEST(SearchRemoveRuntimeTest, RemoveRangeRemovesOnlyEntitiesInsideConfiguredInterval) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    RemoveProbe remove(model, "RemoveRange");
    Queue queue(model, "RemoveRangeQueue");
    CollectorSinkComponentProbe mainSink(model, "RemoveRangeMain");
    CollectorSinkComponentProbe removedSink(model, "RemoveRangeRemoved");
    remove.getConnectionManager()->insert(&mainSink);
    remove.getConnectionManager()->insert(&removedSink);
    remove.setRemoveFromType(Remove::RemoveFromType::QUEUE);
    remove.setRemoveFrom(&queue);
    remove.setRemoveStartRank("1");
    remove.setRemoveEndRank("2");

    CollectorSinkComponentProbe producer(model, "RemoveRangeProducer");
    Entity* q0 = model->createEntity("RemoveRangeQ0", true);
    Entity* q1 = model->createEntity("RemoveRangeQ1", true);
    Entity* q2 = model->createEntity("RemoveRangeQ2", true);
    Entity* q3 = model->createEntity("RemoveRangeQ3", true);
    queue.insertElement(new Waiting(q0, 0.0, &producer));
    queue.insertElement(new Waiting(q1, 0.0, &producer));
    queue.insertElement(new Waiting(q2, 0.0, &producer));
    queue.insertElement(new Waiting(q3, 0.0, &producer));

    Entity* trigger = model->createEntity("RemoveRangeTrigger", true);
    remove.dispatch(trigger);
    drainFutureEvents(model);

    ASSERT_EQ(removedSink.receivedEntities().size(), 2u);
    EXPECT_EQ(removedSink.receivedEntities().at(0), q1);
    EXPECT_EQ(removedSink.receivedEntities().at(1), q2);
    EXPECT_EQ(queue.size(), 2u);
    EXPECT_EQ(queue.getAtRank(0)->getEntity(), q0);
    EXPECT_EQ(queue.getAtRank(1)->getEntity(), q3);
    ASSERT_EQ(mainSink.receivedEntities().size(), 1u);
    EXPECT_EQ(mainSink.receivedEntities().front(), trigger);
}
