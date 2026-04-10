#include <gtest/gtest.h>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/Queue.h"

namespace {

static unsigned int g_waitingProbeDestructions = 0;

class WaitingProbe : public Waiting {
public:
    WaitingProbe(Entity* entity, double timeStartedWaiting, ModelComponent* thisComponent, unsigned int thisComponentOutputPort = 0)
        : Waiting(entity, timeStartedWaiting, thisComponent, thisComponentOutputPort) {}

    ~WaitingProbe() override {
        ++g_waitingProbeDestructions;
    }

    std::string show() override {
        return "WaitingProbe";
    }
};

class QueueLifecycleProbe : public Queue {
public:
    QueueLifecycleProbe(Model* model, const std::string& name)
        : Queue(model, name) {}

    void InitBetweenReplicationsPublic() {
        _initBetweenReplications();
    }
};

} // namespace

TEST(QueueLifecycleTest, FirstOnEmptyQueueReturnsNullptr) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* queue = new QueueLifecycleProbe(model, "QueueProbe");

    EXPECT_EQ(queue->size(), 0u);
    EXPECT_EQ(queue->first(), nullptr);
    EXPECT_EQ(queue->takeFirst(), nullptr);
}

TEST(QueueLifecycleTest, RemoveElementDeletesOwnedWaiting) {
    g_waitingProbeDestructions = 0;

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Entity* entity = model->createEntity("E1", true);
    ASSERT_NE(entity, nullptr);

    auto* queue = new QueueLifecycleProbe(model, "QueueProbe");
    queue->setReportStatistics(false);
    auto* waiting = new WaitingProbe(entity, 0.0, nullptr);
    queue->insertElement(waiting);

    EXPECT_EQ(queue->size(), 1u);
    queue->removeElement(waiting);

    EXPECT_EQ(queue->size(), 0u);
    EXPECT_EQ(g_waitingProbeDestructions, 1u);
}

TEST(QueueLifecycleTest, InitBetweenReplicationsClearsAndDeletesOwnedWaitings) {
    g_waitingProbeDestructions = 0;

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Entity* entity = model->createEntity("E2", true);
    ASSERT_NE(entity, nullptr);

    auto* queue = new QueueLifecycleProbe(model, "QueueProbe");
    queue->setReportStatistics(false);
    queue->insertElement(new WaitingProbe(entity, 1.0, nullptr));
    queue->insertElement(new WaitingProbe(entity, 2.0, nullptr));

    ASSERT_EQ(queue->size(), 2u);
    queue->InitBetweenReplicationsPublic();

    EXPECT_EQ(queue->size(), 0u);
    EXPECT_EQ(g_waitingProbeDestructions, 2u);
}

TEST(QueueLifecycleTest, QueueDestructorDeletesRemainingOwnedWaitings) {
    g_waitingProbeDestructions = 0;

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Entity* entity = model->createEntity("E3", true);
    ASSERT_NE(entity, nullptr);

    auto* queue = new QueueLifecycleProbe(model, "QueueProbe");
    queue->setReportStatistics(false);
    queue->insertElement(new WaitingProbe(entity, 1.0, nullptr));
    queue->insertElement(new WaitingProbe(entity, 2.0, nullptr));
    EXPECT_EQ(queue->size(), 2u);

    model->clear();

    EXPECT_EQ(g_waitingProbeDestructions, 2u);
}
