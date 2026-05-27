#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "tools/analysis/HypothesisTesterDefaultImpl.h"

namespace {

constexpr double kCenterTolerance = 1e-9;

bool isPositive(double value) {
    return value > 0.0;
}

std::string writeSampleFile(const std::string& name, const std::string& contents) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream file(path);
    file << contents;
    return path.string();
}

void expectValidResult(const HypothesisTester_if::TestResult& result) {
    EXPECT_TRUE(std::isfinite(result.pValue()));
    EXPECT_GE(result.pValue(), 0.0);
    EXPECT_LE(result.pValue(), 1.0);
    EXPECT_TRUE(std::isfinite(result.testStat()));
}

TEST(HypothesisTesterDefaultImplTest, ProportionDifferenceConfidenceIntervalHasExpectedCenterAndFiniteBounds) {
    HypothesisTesterDefaultImpl tester;

    auto ci = tester.proportionDifferenceConfidenceInterval(0.60, 0.0, 100, 0.45, 0.0, 120, 0.95);

    ASSERT_TRUE(std::isfinite(ci.inferiorLimit()));
    ASSERT_TRUE(std::isfinite(ci.superiorLimit()));
    ASSERT_TRUE(std::isfinite(ci.halfWidth()));
    EXPECT_GT(ci.halfWidth(), 0.0);

    const double center = (ci.inferiorLimit() + ci.superiorLimit()) * 0.5;
    EXPECT_NEAR(center, 0.15, kCenterTolerance);
}

TEST(HypothesisTesterDefaultImplTest, ProportionConfidenceIntervalWithoutFinitePopulationHasExpectedCenterAndFiniteBounds) {
    HypothesisTesterDefaultImpl tester;

    auto ci = tester.proportionConfidenceInterval(0.80, 200, 0.95);

    ASSERT_TRUE(std::isfinite(ci.inferiorLimit()));
    ASSERT_TRUE(std::isfinite(ci.superiorLimit()));
    ASSERT_TRUE(std::isfinite(ci.halfWidth()));
    EXPECT_GT(ci.halfWidth(), 0.0);

    const double center = (ci.inferiorLimit() + ci.superiorLimit()) * 0.5;
    EXPECT_NEAR(center, 0.80, kCenterTolerance);
}

TEST(HypothesisTesterDefaultImplTest, ProportionConfidenceIntervalWithFinitePopulationShrinksHalfWidth) {
    HypothesisTesterDefaultImpl tester;

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

TEST(HypothesisTesterDefaultImplTest, AverageAndVarianceConfidenceIntervalsHaveFiniteBounds) {
    HypothesisTesterDefaultImpl tester;

    auto average = tester.averageConfidenceInterval(10.0, 2.0, 30, 0.95);
    EXPECT_TRUE(std::isfinite(average.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(average.superiorLimit()));
    EXPECT_GT(average.halfWidth(), 0.0);
    EXPECT_NEAR((average.inferiorLimit() + average.superiorLimit()) * 0.5, 10.0, kCenterTolerance);

    auto variance = tester.varianceConfidenceInterval(4.0, 30, 0.95);
    EXPECT_TRUE(std::isfinite(variance.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(variance.superiorLimit()));
    EXPECT_GT(variance.halfWidth(), 0.0);
}

TEST(HypothesisTesterDefaultImplTest, TwoPopulationConfidenceIntervalsHaveExpectedCenters) {
    HypothesisTesterDefaultImpl tester;

    auto average = tester.averageDifferenceConfidenceInterval(10.0, 2.0, 30, 8.5, 2.1, 28, 0.95);
    EXPECT_TRUE(std::isfinite(average.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(average.superiorLimit()));
    EXPECT_GT(average.halfWidth(), 0.0);
    EXPECT_NEAR((average.inferiorLimit() + average.superiorLimit()) * 0.5, 1.5, kCenterTolerance);

    auto varianceRatio = tester.varianceRatioConfidenceInterval(4.0, 30, 2.0, 28, 0.95);
    EXPECT_TRUE(std::isfinite(varianceRatio.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(varianceRatio.superiorLimit()));
    EXPECT_GT(varianceRatio.halfWidth(), 0.0);
}

TEST(HypothesisTesterDefaultImplTest, TestAverageOnePopulationCoversNonRejectionAndRejectionWithValidPValue) {
    HypothesisTesterDefaultImpl tester;

    auto nonRejection = tester.testAverage(10.0, 2.0, 30, 10.2, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);

    auto rejection = tester.testAverage(10.0, 2.0, 30, 11.5, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
}

TEST(HypothesisTesterDefaultImplTest, TestProportionOnePopulationReturnsValidPValues) {
    HypothesisTesterDefaultImpl tester;

    auto nonRejection = tester.testProportion(0.50, 100, 0.54, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);

    auto rejection = tester.testProportion(0.50, 100, 0.80, 0.95, HypothesisTester_if::H1Comparition::GREATER_THAN);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
}

TEST(HypothesisTesterDefaultImplTest, TestVarianceOnePopulationCoversNonRejectionAndRejectionWithValidPValue) {
    HypothesisTesterDefaultImpl tester;

    auto nonRejection = tester.testVariance(4.0, 30, 4.2, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);

    auto rejection = tester.testVariance(4.0, 30, 1.5, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
}

TEST(HypothesisTesterDefaultImplTest, TwoPopulationTestsReturnValidPValues) {
    HypothesisTesterDefaultImpl tester;

    expectValidResult(tester.testAverage(10.0, 2.0, 30, 8.5, 2.1, 28, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testProportion(0.60, 100, 0.45, 120, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testVariance(4.0, 30, 2.0, 28, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
}

TEST(HypothesisTesterDefaultImplTest, FileBasedMethodsUseAnalysisDatasetLoader) {
    HypothesisTesterDefaultImpl tester;
    const std::string first = writeSampleFile("genesys_hypothesis_first.txt", "-1\n1\n2\n3\n4\n");
    const std::string second = writeSampleFile("genesys_hypothesis_second.txt", "2\n3\n4\n5\n6\n");

    auto averageCi = tester.averageConfidenceInterval(first, 0.95);
    EXPECT_TRUE(std::isfinite(averageCi.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(averageCi.superiorLimit()));

    auto proportionCi = tester.proportionConfidenceInterval(first, isPositive, 0.95);
    EXPECT_TRUE(std::isfinite(proportionCi.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(proportionCi.superiorLimit()));

    expectValidResult(tester.testAverage(first, 2.0, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testProportion(first, isPositive, 0.50, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testVariance(first, 4.0, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testAverage(first, second, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testProportion(first, second, isPositive, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testVariance(first, second, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
}

TEST(HypothesisTesterDefaultImplTest, InvalidConfidenceLevelThrows) {
    HypothesisTesterDefaultImpl tester;

    EXPECT_THROW(tester.averageConfidenceInterval(10.0, 2.0, 30, 1.0), std::invalid_argument);
    EXPECT_THROW(tester.testAverage(10.0, 2.0, 30, 10.2, 0.0, HypothesisTester_if::H1Comparition::DIFFERENT), std::invalid_argument);
}

} // namespace
