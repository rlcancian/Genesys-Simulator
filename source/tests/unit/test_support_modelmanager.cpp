#include <gtest/gtest.h>

#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"

// Provide an out-of-line virtual destructor definition so RTTI symbols are available when linking support-only tests.
Simulator::~Simulator() = default;

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
    EXPECT_EQ(manager.size(), 1u);
    EXPECT_TRUE(manager.hasModel(created));
    EXPECT_EQ(manager.currentIndex(), 0);
    EXPECT_EQ(manager.modelAt(0), created);
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

TEST(SupportModelManagerClassTest, NewModelKeepsPreviousModelOpen) {
    // A new model is now an opened model, not a detached replacement of the previous current model.
    g_model_constructions = 0;
    g_model_destructions = 0;
    {
        ModelManager manager(nullptr);

        Model* first = manager.newModel();
        Model* second = manager.newModel();

        ASSERT_NE(first, nullptr);
        ASSERT_NE(second, nullptr);
        EXPECT_EQ(manager.current(), second);
        EXPECT_EQ(manager.size(), 2u);
        EXPECT_EQ(manager.modelAt(0), first);
        EXPECT_EQ(manager.modelAt(1), second);
        ASSERT_EQ(manager.models().size(), 2u);
        EXPECT_EQ(manager.models().at(0), first);
        EXPECT_EQ(manager.models().at(1), second);
        EXPECT_EQ(g_model_constructions, 2);
        EXPECT_EQ(g_model_destructions, 0);
    }

    EXPECT_EQ(g_model_destructions, 2);
}

TEST(SupportModelManagerClassTest, DestructorReleasesOpenModels) {
    // Ensures manager teardown releases every model owned by the open-model list.
    g_model_constructions = 0;
    g_model_destructions = 0;
    {
        ModelManager manager(nullptr);
        ASSERT_NE(manager.newModel(), nullptr);
        ASSERT_NE(manager.newModel(), nullptr);
        EXPECT_EQ(manager.size(), 2u);
    }

    EXPECT_EQ(g_model_constructions, 2);
    EXPECT_EQ(g_model_destructions, 2);
}

TEST(SupportModelManagerClassTest, NavigationUpdatesCurrentModel) {
    ModelManager manager(nullptr);

    Model* first = manager.newModel();
    Model* second = manager.newModel();
    Model* third = manager.newModel();

    EXPECT_EQ(manager.current(), third);
    EXPECT_EQ(manager.currentIndex(), 2);
    EXPECT_FALSE(manager.canGoNext());
    EXPECT_TRUE(manager.canGoPrevious());

    EXPECT_EQ(manager.previous(), second);
    EXPECT_EQ(manager.current(), second);
    EXPECT_TRUE(manager.canGoNext());
    EXPECT_TRUE(manager.canGoPrevious());

    EXPECT_EQ(manager.previous(), first);
    EXPECT_EQ(manager.currentIndex(), 0);
    EXPECT_FALSE(manager.canGoPrevious());

    EXPECT_EQ(manager.next(), second);
    EXPECT_EQ(manager.next(), third);
    EXPECT_EQ(manager.next(), nullptr);
    EXPECT_EQ(manager.current(), third);
}

TEST(SupportModelManagerClassTest, SetCurrentAcceptsOnlyOpenModels) {
    ModelManager manager(nullptr);

    Model* opened = manager.newModel();
    Model* unmanaged = new Model(nullptr);

    EXPECT_FALSE(manager.setCurrent(unmanaged));
    EXPECT_EQ(manager.current(), opened);

    EXPECT_TRUE(manager.setCurrent(nullptr));
    EXPECT_EQ(manager.current(), nullptr);

    EXPECT_TRUE(manager.setCurrent(opened));
    EXPECT_EQ(manager.current(), opened);

    delete unmanaged;
}

TEST(SupportModelManagerClassTest, RemoveCurrentSelectsNearestRemainingModel) {
    g_model_destructions = 0;
    ModelManager manager(nullptr);

    Model* first = manager.newModel();
    Model* second = manager.newModel();
    Model* third = manager.newModel();

    ASSERT_TRUE(manager.setCurrent(second));
    manager.remove(second);

    EXPECT_EQ(manager.size(), 2u);
    EXPECT_EQ(manager.current(), third);
    EXPECT_EQ(manager.currentIndex(), 1);

    manager.remove(third);

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_EQ(manager.current(), first);
    EXPECT_EQ(manager.currentIndex(), 0);

    manager.remove(first);

    EXPECT_EQ(manager.size(), 0u);
    EXPECT_EQ(manager.current(), nullptr);
    EXPECT_EQ(g_model_destructions, 3);
}
