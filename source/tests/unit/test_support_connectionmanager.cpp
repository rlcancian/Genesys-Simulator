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

TEST(SupportConnectionManagerClassTest, RemovingUnknownConnectionKeepsStateUnchanged) {
    ConnectionManager manager;
    Connection* inserted = new Connection{nullptr, {0, ""}};
    Connection* neverInserted = new Connection{nullptr, {1, ""}};

    // Inserts one connection and validates that removing an unknown pointer keeps the existing mapping intact.
    manager.insertAtPort(0, inserted);
    manager.remove(neverInserted);

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_EQ(manager.getConnectionAtPort(0), inserted);

    delete neverInserted;
}

TEST(SupportConnectionManagerClassTest, ReplaceSamePointerDoesNotChangeSize) {
    ConnectionManager manager;
    Connection* reused = new Connection{nullptr, {0, ""}};

    // Reinserts the same pointer in the same port to validate the defensive no-growth replace path.
    manager.insertAtPort(0, reused);
    manager.insertAtPort(0, reused);

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_EQ(manager.getConnectionAtPort(0), reused);
}
