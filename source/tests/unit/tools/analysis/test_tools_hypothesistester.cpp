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

double uniform01Cdf(double value) {
    if (value <= 0.0) {
        return 0.0;
    }
    if (value >= 1.0) {
        return 1.0;
    }
    return value;
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

TEST(HypothesisTesterDefaultImplTest, ChiSquareGoodnessOfFitCoversNonRejectionAndRejection) {
    HypothesisTesterDefaultImpl tester;

    auto nonRejection = tester.chiSquareGoodnessOfFit({18.0, 22.0, 20.0}, {20.0, 20.0, 20.0}, 0, 0.95);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);
    EXPECT_GE(nonRejection.testStat(), 0.0);

    auto rejection = tester.chiSquareGoodnessOfFit({40.0, 10.0, 10.0}, {20.0, 20.0, 20.0}, 0, 0.95);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
	EXPECT_GE(rejection.testStat(), 0.0);
}

TEST(HypothesisTesterDefaultImplTest, ChiSquareGoodnessOfFitBuildsFrequenciesFromSampleAndExplicitClasses) {
    HypothesisTesterDefaultImpl tester;
    const std::vector<double> boundaries = {0.0, 0.25, 0.50, 0.75, 1.0};

    auto nonRejection = tester.chiSquareGoodnessOfFit(
            std::vector<double>{0.05, 0.10, 0.30, 0.45, 0.55, 0.70, 0.80, 0.95},
            uniform01Cdf,
            boundaries,
            0,
            0.95,
            1.0);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);

    auto rejection = tester.chiSquareGoodnessOfFit(
            std::vector<double>{0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08},
            uniform01Cdf,
            boundaries,
            0,
            0.95,
            1.0);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
}

TEST(HypothesisTesterDefaultImplTest, ChiSquareGoodnessOfFitSupportsAutomaticClasses) {
    HypothesisTesterDefaultImpl tester;

    auto result = tester.chiSquareGoodnessOfFit(
            std::vector<double>{0.0, 0.25, 0.50, 0.75, 1.0},
            uniform01Cdf,
            0,
            0.95,
            2,
            1.0);

    EXPECT_FALSE(result.rejectH0());
    expectValidResult(result);
}

TEST(HypothesisTesterDefaultImplTest, ChiSquareGoodnessOfFitGroupsLowExpectedClasses) {
    HypothesisTesterDefaultImpl tester;
    const std::vector<double> sample = {
        0.05, 0.15, 0.25, 0.35, 0.45,
        0.55, 0.65, 0.75, 0.85, 0.95
    };
    const std::vector<double> boundaries = {
        0.0, 0.1, 0.2, 0.3, 0.4, 0.5,
        0.6, 0.7, 0.8, 0.9, 1.0
    };

    auto result = tester.chiSquareGoodnessOfFit(sample, uniform01Cdf, boundaries, 0, 0.95, 5.0);

    EXPECT_FALSE(result.rejectH0());
    expectValidResult(result);
    EXPECT_NEAR(result.testStat(), 0.0, 1e-9);
}

TEST(HypothesisTesterDefaultImplTest, ChiSquareGoodnessOfFitRejectsInvalidInputs) {
    HypothesisTesterDefaultImpl tester;

    EXPECT_THROW(tester.chiSquareGoodnessOfFit({1.0, 2.0}, {1.0}, 0, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.chiSquareGoodnessOfFit({1.0, 2.0}, {1.0, 0.0}, 0, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.chiSquareGoodnessOfFit({1.0, 2.0}, {1.0, 2.0}, 1, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.chiSquareGoodnessOfFit({1.0, 2.0}, {1.0, 2.0}, 0, 1.0), std::invalid_argument);
    EXPECT_THROW(tester.chiSquareGoodnessOfFit(std::vector<double>{}, uniform01Cdf, 0, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.chiSquareGoodnessOfFit(std::vector<double>{0.1, 0.2}, distributionCdfFunction{}, 0, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.chiSquareGoodnessOfFit(std::vector<double>{0.1, 0.2}, uniform01Cdf, std::vector<double>{0.0, 1.0}, 0, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.chiSquareGoodnessOfFit(std::vector<double>{0.1, 1.2}, uniform01Cdf, std::vector<double>{0.0, 0.5, 1.0}, 0, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.chiSquareGoodnessOfFit(std::vector<double>{0.1, 0.2}, uniform01Cdf, std::vector<double>{0.0, 0.5, 0.4}, 0, 0.95), std::invalid_argument);
}

TEST(HypothesisTesterDefaultImplTest, KolmogorovSmirnovCoversNonRejectionAndRejection) {
    HypothesisTesterDefaultImpl tester;

    auto nonRejection = tester.kolmogorovSmirnov(std::vector<double>{0.1, 0.2, 0.35, 0.5, 0.65, 0.8, 0.9}, uniform01Cdf, 0.95);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);
    EXPECT_GE(nonRejection.testStat(), 0.0);
    EXPECT_LE(nonRejection.testStat(), 1.0);

    auto rejection = tester.kolmogorovSmirnov(std::vector<double>{0.01, 0.02, 0.03, 0.04, 0.05}, uniform01Cdf, 0.95);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
    EXPECT_GE(rejection.testStat(), 0.0);
    EXPECT_LE(rejection.testStat(), 1.0);
}

TEST(HypothesisTesterDefaultImplTest, KolmogorovSmirnovFileBasedOverloadUsesAnalysisDatasetLoader) {
    HypothesisTesterDefaultImpl tester;
    const std::string sample = writeSampleFile("genesys_ks_uniform_sample.txt", "0.1\n0.2\n0.35\n0.5\n0.65\n0.8\n0.9\n");

    auto result = tester.kolmogorovSmirnov(sample, uniform01Cdf, 0.95);

    EXPECT_FALSE(result.rejectH0());
    expectValidResult(result);
    EXPECT_GE(result.testStat(), 0.0);
    EXPECT_LE(result.testStat(), 1.0);
}

TEST(HypothesisTesterDefaultImplTest, KolmogorovSmirnovRejectsInvalidInputs) {
    HypothesisTesterDefaultImpl tester;

    EXPECT_THROW(tester.kolmogorovSmirnov(std::vector<double>{}, uniform01Cdf, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.kolmogorovSmirnov(std::vector<double>{0.1, 0.2}, distributionCdfFunction{}, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.kolmogorovSmirnov(std::vector<double>{0.1, 0.2}, [](double) { return 2.0; }, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.kolmogorovSmirnov(std::vector<double>{0.1, 0.2}, uniform01Cdf, 0.0), std::invalid_argument);
}

} // namespace
