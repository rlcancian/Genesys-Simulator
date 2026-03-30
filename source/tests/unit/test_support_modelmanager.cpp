#include <gtest/gtest.h>

#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"

TraceManager* Simulator::getTraceManager() const {
    return nullptr;
}

namespace {
int g_model_constructions = 0;
int g_model_saves = 0;
}

Model::Model(Simulator* simulator, unsigned int level) {
    (void)simulator;
    (void)level;
    ++g_model_constructions;
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
    ModelManager manager(nullptr);

    Model* created = manager.newModel();

    ASSERT_NE(created, nullptr);
    EXPECT_EQ(manager.current(), created);
    EXPECT_EQ(g_model_constructions, 1);
}

TEST(SupportModelManagerClassTest, SaveModelDelegatesToCurrentModel) {
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
