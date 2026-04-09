#include <gtest/gtest.h>

#include "kernel/simulator/ConnectionManager.h"

TEST(SupportConnectionManagerClassTest, StartsEmpty) {
    ConnectionManager manager;

    EXPECT_EQ(manager.size(), 0u);
    EXPECT_EQ(manager.getFrontConnection(), nullptr);
    EXPECT_EQ(manager.getConnectionAtPort(0), nullptr);
}

TEST(SupportConnectionManagerClassTest, InsertCreatesConnectionAtPortZero) {
    ConnectionManager manager;

    manager.insert(static_cast<ModelComponent*>(nullptr));

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_NE(manager.getFrontConnection(), nullptr);
    EXPECT_EQ(manager.getConnectionAtPort(0), manager.getFrontConnection());
}

TEST(SupportConnectionManagerClassTest, RemoveAtPortClearsInsertedConnection) {
    ConnectionManager manager;
    manager.insert(static_cast<ModelComponent*>(nullptr));

    manager.removeAtPort(0);

    EXPECT_EQ(manager.size(), 0u);
    EXPECT_EQ(manager.getConnectionAtPort(0), nullptr);
}

TEST(SupportConnectionManagerClassTest, InsertAtPortReplacesExistingConnectionWithoutGrowingSize) {
    ConnectionManager manager;
    // Build two raw connections to validate replacement and ownership in the same port.
    Connection* first = new Connection{nullptr, {0, ""}};
    Connection* second = new Connection{nullptr, {0, ""}};

    manager.insertAtPort(0, first);
    manager.insertAtPort(0, second);

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_EQ(manager.getConnectionAtPort(0), second);
}
