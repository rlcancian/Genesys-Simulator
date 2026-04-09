#include <gtest/gtest.h>

#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"

TraceManager* Simulator::getTraceManager() const {
    return nullptr;
}

namespace {
int g_model_constructions = 0;
int g_model_destructions = 0;
int g_model_saves = 0;
}

Model::Model(Simulator* simulator, unsigned int level) {
    (void)simulator;
    (void)level;
    ++g_model_constructions;
}

// Tracks model destruction so tests can validate detached current-model lifecycle behavior in ModelManager.
Model::~Model() {
    ++g_model_destructions;
}

bool Model::save(std::string filename) {
    (void)filename;
    ++g_model_saves;
    return true;
}

bool Model::load(std::string filename) {
    (void)filename;
    return true;
}

TEST(SupportModelManagerClassTest, NewModelSetsCurrentModel) {
    // Resets lifecycle counters to keep this test independent from others.
    g_model_constructions = 0;
    g_model_destructions = 0;
    ModelManager manager(nullptr);

    Model* created = manager.newModel();

    ASSERT_NE(created, nullptr);
    EXPECT_EQ(manager.current(), created);
    EXPECT_EQ(g_model_constructions, 1);
}

TEST(SupportModelManagerClassTest, SaveModelDelegatesToCurrentModel) {
    // Resets construction/save counters before exercising delegation.
    g_model_constructions = 0;
    g_model_saves = 0;
    ModelManager manager(nullptr);
    manager.newModel();

    const bool saved = manager.saveModel("dummy.gen");

    EXPECT_TRUE(saved);
    EXPECT_EQ(g_model_saves, 1);
}

TEST(SupportModelManagerClassTest, SaveModelWithoutCurrentModelReturnsFalse) {
    ModelManager manager(nullptr);

    EXPECT_FALSE(manager.saveModel("dummy.gen"));
}

TEST(SupportModelManagerClassTest, NewModelReleasesPreviousDetachedCurrentModel) {
    // Resets counters and verifies replacing a detached current model releases the previous allocation.
    g_model_constructions = 0;
    g_model_destructions = 0;
    ModelManager manager(nullptr);

    Model* first = manager.newModel();
    Model* second = manager.newModel();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(manager.current(), second);
    EXPECT_EQ(g_model_constructions, 2);
    EXPECT_EQ(g_model_destructions, 1);
}

TEST(SupportModelManagerClassTest, DestructorReleasesDetachedCurrentModel) {
    // Ensures manager teardown releases a detached current model even when it was never inserted into _models.
    g_model_constructions = 0;
    g_model_destructions = 0;
    {
        ModelManager manager(nullptr);
        Model* created = manager.newModel();
        ASSERT_NE(created, nullptr);
    }

    EXPECT_EQ(g_model_constructions, 1);
    EXPECT_EQ(g_model_destructions, 1);
}
