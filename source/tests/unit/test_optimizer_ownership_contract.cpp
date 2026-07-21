#include <gtest/gtest.h>

#include "tools/Optimization/OptimizerDefaultImpl1.h"

#include <type_traits>

static_assert(std::is_default_constructible_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 must remain directly constructible by OptimizerWindow");
static_assert(std::is_destructible_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 must remain directly destructible");
static_assert(!std::is_copy_constructible_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 must not shallow-copy its seven owned List objects");
static_assert(!std::is_copy_assignable_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 must not shallow-copy-assign its seven owned List objects");
static_assert(!std::is_move_constructible_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 move construction is unsupported until ownership is migrated");
static_assert(!std::is_move_assignable_v<OptimizerDefaultImpl1>,
              "OptimizerDefaultImpl1 move assignment is unsupported until ownership is migrated");

TEST(OptimizerOwnershipContractTest, CompileTimeOwnershipContractIsAvailable) {
    SUCCEED();
}
