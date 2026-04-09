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
    // Allocates distinct raw connections to validate remove(Connection*) leaves state intact when pointer is unknown.
    Connection* inserted = new Connection{nullptr, {0, ""}};
    Connection* unknown = new Connection{nullptr, {0, ""}};

    manager.insertAtPort(0, inserted);
    manager.remove(unknown);

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_EQ(manager.getConnectionAtPort(0), inserted);

    // Releases the pointer never handed over to manager ownership to avoid leaking test memory.
    delete unknown;
}

TEST(SupportConnectionManagerClassTest, ReplaceSamePointerDoesNotChangeSize) {
    ConnectionManager manager;
    // Reuses the same pointer at the same port to assert defensive replacement logic keeps a single owned entry.
    Connection* shared = new Connection{nullptr, {0, ""}};

    manager.insertAtPort(0, shared);
    manager.insertAtPort(0, shared);

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_EQ(manager.getConnectionAtPort(0), shared);
}
