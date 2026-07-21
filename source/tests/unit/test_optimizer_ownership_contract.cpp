#include <gtest/gtest.h>

#include "tools/Optimization/OptimizerDefaultImpl1.h"

#include <type_traits>

static_assert(!std::is_copy_constructible_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 must not shallow-copy its seven owned List objects");
static_assert(!std::is_copy_assignable_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 must not shallow-copy-assign its seven owned List objects");
static_assert(!std::is_move_constructible_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 move construction is unsupported until ownership is migrated");
static_assert(!std::is_move_assignable_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 move assignment is unsupported until ownership is migrated");

TEST(OptimizerOwnershipContractTest, DefaultConstructionAndDestructionRemainValid) {
    OptimizerDefaultImpl1 optimizer;

    EXPECT_EQ(optimizer.getModel(), nullptr);
    ASSERT_NE(optimizer.getAvailableControls(), nullptr);
    ASSERT_NE(optimizer.getAvailableResponses(), nullptr);
    ASSERT_NE(optimizer.getSelectedControls(), nullptr);
    ASSERT_NE(optimizer.getSelectedResponses(), nullptr);
    ASSERT_NE(optimizer.getObjectives(), nullptr);
    ASSERT_NE(optimizer.getConstraints(), nullptr);
    ASSERT_NE(optimizer.getBestSolutions(), nullptr);
    EXPECT_EQ(optimizer.getExecutionState(), Optimizer_if::ExecutionState::NOT_READY);
}
