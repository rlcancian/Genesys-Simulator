#include <gtest/gtest.h>

#include <cmath>

#include "tools/HypothesisTesterDefaultImpl1.h"

namespace {

constexpr double kCenterTolerance = 1e-9;

TEST(HypothesisTesterDefaultImpl1Test, ProportionDifferenceConfidenceIntervalHasExpectedCenterAndFiniteBounds) {
    HypothesisTesterDefaultImpl1 tester;

    auto ci = tester.proportionDifferenceConfidenceInterval(0.60, 0.0, 100, 0.45, 0.0, 120, 0.95);

    ASSERT_TRUE(std::isfinite(ci.inferiorLimit()));
    ASSERT_TRUE(std::isfinite(ci.superiorLimit()));
    ASSERT_TRUE(std::isfinite(ci.halfWidth()));
    EXPECT_GT(ci.halfWidth(), 0.0);

    const double center = (ci.inferiorLimit() + ci.superiorLimit()) * 0.5;
    EXPECT_NEAR(center, 0.15, kCenterTolerance);
}

TEST(HypothesisTesterDefaultImpl1Test, ProportionConfidenceIntervalWithoutFinitePopulationHasExpectedCenterAndFiniteBounds) {
    HypothesisTesterDefaultImpl1 tester;

    auto ci = tester.proportionConfidenceInterval(0.80, 200, 0.95);

    ASSERT_TRUE(std::isfinite(ci.inferiorLimit()));
    ASSERT_TRUE(std::isfinite(ci.superiorLimit()));
    ASSERT_TRUE(std::isfinite(ci.halfWidth()));
    EXPECT_GT(ci.halfWidth(), 0.0);

    const double center = (ci.inferiorLimit() + ci.superiorLimit()) * 0.5;
    EXPECT_NEAR(center, 0.80, kCenterTolerance);
}

TEST(HypothesisTesterDefaultImpl1Test, ProportionConfidenceIntervalWithFinitePopulationShrinksHalfWidth) {
    HypothesisTesterDefaultImpl1 tester;

    auto noPopulationCorrection = tester.proportionConfidenceInterval(0.80, 200, 0.95);
    auto finitePopulation = tester.proportionConfidenceInterval(0.80, 200, 1000, 0.95);

    ASSERT_TRUE(std::isfinite(finitePopulation.inferiorLimit()));
    ASSERT_TRUE(std::isfinite(finitePopulation.superiorLimit()));
    ASSERT_TRUE(std::isfinite(finitePopulation.halfWidth()));
    EXPECT_GT(finitePopulation.halfWidth(), 0.0);

    const double center = (finitePopulation.inferiorLimit() + finitePopulation.superiorLimit()) * 0.5;
    EXPECT_NEAR(center, 0.80, kCenterTolerance);
    EXPECT_LT(finitePopulation.halfWidth(), noPopulationCorrection.halfWidth());
}

TEST(HypothesisTesterDefaultImpl1Test, TestAverageOnePopulationCoversNonRejectionAndRejectionWithValidPValue) {
    HypothesisTesterDefaultImpl1 tester;

    auto nonRejection = tester.testAverage(10.0, 2.0, 30, 10.2, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(nonRejection.rejectH0());
    EXPECT_TRUE(std::isfinite(nonRejection.pValue()));
    EXPECT_GE(nonRejection.pValue(), 0.0);
    EXPECT_LE(nonRejection.pValue(), 1.0);

    auto rejection = tester.testAverage(10.0, 2.0, 30, 11.5, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_TRUE(rejection.rejectH0());
    EXPECT_TRUE(std::isfinite(rejection.pValue()));
    EXPECT_GE(rejection.pValue(), 0.0);
    EXPECT_LE(rejection.pValue(), 1.0);
}

TEST(HypothesisTesterDefaultImpl1Test, TestVarianceOnePopulationCoversNonRejectionAndRejectionWithValidPValue) {
    HypothesisTesterDefaultImpl1 tester;

    auto nonRejection = tester.testVariance(4.0, 30, 4.2, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(nonRejection.rejectH0());
    EXPECT_TRUE(std::isfinite(nonRejection.pValue()));
    EXPECT_GE(nonRejection.pValue(), 0.0);
    EXPECT_LE(nonRejection.pValue(), 1.0);

    auto rejection = tester.testVariance(4.0, 30, 1.5, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_TRUE(rejection.rejectH0());
    EXPECT_TRUE(std::isfinite(rejection.pValue()));
    EXPECT_GE(rejection.pValue(), 0.0);
    EXPECT_LE(rejection.pValue(), 1.0);
}

} // namespace
