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

// Tracks destruction events so tests can validate detached current-model lifecycle handling in ModelManager.
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
    // Resets test counters to keep this scenario isolated from previous test side effects.
    g_model_constructions = 0;
    g_model_destructions = 0;
    g_model_saves = 0;
    ModelManager manager(nullptr);

    Model* created = manager.newModel();

    ASSERT_NE(created, nullptr);
    EXPECT_EQ(manager.current(), created);
    EXPECT_EQ(g_model_constructions, 1);
}

TEST(SupportModelManagerClassTest, SaveModelDelegatesToCurrentModel) {
    // Resets test counters to keep this scenario isolated from previous test side effects.
    g_model_constructions = 0;
    g_model_destructions = 0;
    g_model_saves = 0;
    ModelManager manager(nullptr);
    manager.newModel();

    const bool saved = manager.saveModel("dummy.gen");

    EXPECT_TRUE(saved);
    EXPECT_EQ(g_model_saves, 1);
}

TEST(SupportModelManagerClassTest, SaveModelWithoutCurrentModelReturnsFalse) {
    // Resets test counters to keep this scenario isolated from previous test side effects.
    g_model_constructions = 0;
    g_model_destructions = 0;
    g_model_saves = 0;
    ModelManager manager(nullptr);

    EXPECT_FALSE(manager.saveModel("dummy.gen"));
}

TEST(SupportModelManagerClassTest, NewModelReleasesPreviousDetachedCurrentModel) {
    // Resets lifecycle counters and validates replacing detached current model deletes the previous instance.
    g_model_constructions = 0;
    g_model_destructions = 0;
    g_model_saves = 0;

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
    // Ensures manager teardown deletes a detached current model that was never inserted into the internal list.
    g_model_constructions = 0;
    g_model_destructions = 0;
    g_model_saves = 0;

    {
        ModelManager manager(nullptr);
        Model* current = manager.newModel();
        ASSERT_NE(current, nullptr);
        EXPECT_EQ(g_model_constructions, 1);
        EXPECT_EQ(g_model_destructions, 0);
    }

    EXPECT_EQ(g_model_destructions, 1);
}
