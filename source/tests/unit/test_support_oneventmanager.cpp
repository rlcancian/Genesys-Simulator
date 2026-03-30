#include <gtest/gtest.h>

#include "kernel/simulator/OnEventManager.h"

namespace {
int g_replication_start_calls = 0;
int g_model_check_success_calls = 0;

void CountReplicationStart(SimulationEvent*) {
    ++g_replication_start_calls;
}

void CountModelCheckSuccess(ModelEvent*) {
    ++g_model_check_success_calls;
}

struct ModelEventObserver {
    int calls = 0;

    void OnModelCheckSuccess(ModelEvent*) {
        ++calls;
    }
};

struct SimulationEventObserver {
    int calls = 0;

    void OnReplicationStart(SimulationEvent*) {
        ++calls;
    }
};
}

TEST(SupportOnEventManagerClassTest, DeduplicatesFunctionHandlers) {
    g_replication_start_calls = 0;

    OnEventManager manager;
    manager.addOnReplicationStartHandler(&CountReplicationStart);
    manager.addOnReplicationStartHandler(&CountReplicationStart);

    manager.NotifyReplicationStartHandlers(nullptr);

    EXPECT_EQ(g_replication_start_calls, 1);
}

TEST(SupportOnEventManagerClassTest, InvokesMethodHandlers) {
    OnEventManager manager;
    SimulationEventObserver observer;

    manager.addOnReplicationStartHandler(&observer, &SimulationEventObserver::OnReplicationStart);
    manager.NotifyReplicationStartHandlers(nullptr);

    EXPECT_EQ(observer.calls, 1);
}

TEST(SupportOnEventManagerClassTest, DeduplicatesModelFunctionHandlers) {
    g_model_check_success_calls = 0;

    OnEventManager manager;
    manager.addOnModelCheckSucessHandler(&CountModelCheckSuccess);
    manager.addOnModelCheckSucessHandler(&CountModelCheckSuccess);

    manager.NotifyModelCheckSuccessHandlers(nullptr);

    EXPECT_EQ(g_model_check_success_calls, 1);
}

TEST(SupportOnEventManagerClassTest, InvokesModelMethodHandlers) {
    OnEventManager manager;
    ModelEventObserver observer;

    manager.addOnModelCheckSuccessHandler(&observer, &ModelEventObserver::OnModelCheckSuccess);
    manager.NotifyModelCheckSuccessHandlers(nullptr);

    EXPECT_EQ(observer.calls, 1);
}
