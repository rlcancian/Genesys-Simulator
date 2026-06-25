#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "tools/analysis/HypothesisTesterDefaultImpl.h"

namespace {

constexpr double kCenterTolerance = 1e-9;
constexpr double kReferenceTolerance = 1e-3;

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

// Test objective: verifies HypothesisTesterDefaultImplTest.ConfidenceIntervalsMatchPublishedReferenceValues.
TEST(HypothesisTesterDefaultImplTest, ConfidenceIntervalsMatchPublishedReferenceValues) {
    HypothesisTesterDefaultImpl tester;

    // Reference values:
    // t(0.975, 29) = 2.0452
    // z(0.975) = 1.9600
    // chi-square(0.975, 29) = 45.7223; chi-square(0.025, 29) = 16.0471
    auto meanCi = tester.averageConfidenceInterval(10.0, 2.0, 30, 0.95);
    EXPECT_NEAR(meanCi.inferiorLimit(), 9.2531, kReferenceTolerance);
    EXPECT_NEAR(meanCi.superiorLimit(), 10.7469, kReferenceTolerance);
    EXPECT_NEAR(meanCi.halfWidth(), 0.7469, kReferenceTolerance);

    auto proportionCi = tester.proportionConfidenceInterval(0.80, 200, 0.95);
    EXPECT_NEAR(proportionCi.inferiorLimit(), 0.7446, kReferenceTolerance);
    EXPECT_NEAR(proportionCi.superiorLimit(), 0.8554, kReferenceTolerance);
    EXPECT_NEAR(proportionCi.halfWidth(), 0.0554, kReferenceTolerance);

    auto varianceCi = tester.varianceConfidenceInterval(4.0, 30, 0.95);
    EXPECT_NEAR(varianceCi.inferiorLimit(), 2.5370, 2e-3);
    EXPECT_NEAR(varianceCi.superiorLimit(), 7.2287, 2e-3);

    // F-ratio CI reference with unequal sample sizes:
    // s1^2/s2^2 = 61.1617/62.2748, df1=79, df2=84.
    auto varianceRatioCi = tester.varianceRatioConfidenceInterval(61.1617, 80, 62.2748, 85, 0.95);
    EXPECT_NEAR(varianceRatioCi.inferiorLimit(), 0.6351, 5e-3);
    EXPECT_NEAR(varianceRatioCi.superiorLimit(), 1.5234, 5e-3);

    auto reciprocalVarianceRatioCi = tester.varianceRatioConfidenceInterval(62.2748, 85, 61.1617, 80, 0.95);
    EXPECT_NEAR(reciprocalVarianceRatioCi.inferiorLimit(), 1.0 / varianceRatioCi.superiorLimit(), 5e-4);
    EXPECT_NEAR(reciprocalVarianceRatioCi.superiorLimit(), 1.0 / varianceRatioCi.inferiorLimit(), 5e-4);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.ParametricTestsMatchPublishedReferenceValues.
TEST(HypothesisTesterDefaultImplTest, ParametricTestsMatchPublishedReferenceValues) {
    HypothesisTesterDefaultImpl tester;

    // t = (11.5 - 10) / (2 / sqrt(30)) = 4.1079; t(0.975, 29) = 2.0452.
    auto meanTest = tester.testAverage(10.0, 2.0, 30, 11.5, 0.95, HypothesisTester_if::DIFFERENT);
    EXPECT_NEAR(meanTest.testStat(), 4.1079, kReferenceTolerance);
    EXPECT_NEAR(meanTest.acceptanceInferiorLimit(), -2.0452, kReferenceTolerance);
    EXPECT_NEAR(meanTest.acceptanceSuperiorLimit(), 2.0452, kReferenceTolerance);
    EXPECT_NEAR(meanTest.pValue(), 0.0003, 5e-4);
    EXPECT_TRUE(meanTest.rejectH0());

    // z = (0.80 - 0.50) / sqrt(0.50 * 0.50 / 100) = 6.0000; z(0.95) = 1.6449.
    auto proportionTest = tester.testProportion(0.50, 100, 0.80, 0.95, HypothesisTester_if::GREATER_THAN);
    EXPECT_NEAR(proportionTest.testStat(), 6.0000, kReferenceTolerance);
    EXPECT_NEAR(proportionTest.acceptanceSuperiorLimit(), 1.6449, kReferenceTolerance);
    EXPECT_NEAR(proportionTest.pValue(), 0.0, 1e-8);
    EXPECT_TRUE(proportionTest.rejectH0());

    // chi-square observed = (n - 1) * s^2 / sigma0^2 = 29 * 4.0 / 4.2 = 27.6190.
    auto varianceTest = tester.testVariance(4.0, 30, 4.2, 0.95, HypothesisTester_if::DIFFERENT);
    EXPECT_NEAR(varianceTest.testStat(), 27.6190, kReferenceTolerance);
    EXPECT_NEAR(varianceTest.acceptanceInferiorLimit(), 16.0471, 2e-3);
    EXPECT_NEAR(varianceTest.acceptanceSuperiorLimit(), 45.7223, 2e-3);
    EXPECT_FALSE(varianceTest.rejectH0());

    // Equal-variance two-sample t reference:
    // pooled variance = 4.1977, t = (10.0 - 8.5) / sqrt(sp^2 * (1/30 + 1/28)) = 2.7847.
    auto twoMeanTest = tester.testAverage(10.0, 2.0, 30, 8.5, 2.1, 28, 0.95, HypothesisTester_if::DIFFERENT);
    EXPECT_NEAR(twoMeanTest.testStat(), 2.7847, 2e-3);
    EXPECT_NEAR(twoMeanTest.acceptanceInferiorLimit(), -2.0032, 2e-3);
    EXPECT_NEAR(twoMeanTest.acceptanceSuperiorLimit(), 2.0032, 2e-3);
    EXPECT_TRUE(twoMeanTest.rejectH0());
}

// Test objective: verifies HypothesisTesterDefaultImplTest.ChiSquareGoodnessOfFitMatchesReferenceValues.
TEST(HypothesisTesterDefaultImplTest, ChiSquareGoodnessOfFitMatchesReferenceValues) {
    HypothesisTesterDefaultImpl tester;

    // Sum((O - E)^2 / E) = 0.4, df = 2, p-value = exp(-0.4 / 2) = 0.8187.
    auto result = tester.chiSquareGoodnessOfFit({18.0, 22.0, 20.0}, {20.0, 20.0, 20.0}, 0, 0.95);

    EXPECT_NEAR(result.testStat(), 0.4000, kReferenceTolerance);
    EXPECT_NEAR(result.pValue(), 0.8187, kReferenceTolerance);
    EXPECT_NEAR(result.acceptanceInferiorLimit(), 0.0, kReferenceTolerance);
    EXPECT_NEAR(result.acceptanceSuperiorLimit(), 5.9915, 2e-3);
    EXPECT_FALSE(result.rejectH0());
    ASSERT_TRUE(result.hasGoodnessOfFitDetails());
    const auto details = result.goodnessOfFitDetails();
    EXPECT_EQ(details.initialClasses, 3u);
    EXPECT_EQ(details.effectiveClasses, 3u);
    EXPECT_EQ(details.estimatedParameters, 0u);
    EXPECT_DOUBLE_EQ(details.degreesOfFreedom, 2.0);
    EXPECT_DOUBLE_EQ(details.observedTotal, 60.0);
    EXPECT_DOUBLE_EQ(details.expectedTotal, 60.0);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.KolmogorovSmirnovMatchesReferenceValues.
TEST(HypothesisTesterDefaultImplTest, KolmogorovSmirnovMatchesReferenceValues) {
    HypothesisTesterDefaultImpl tester;

    const std::vector<double> sample = {0.1, 0.2, 0.35, 0.5, 0.65, 0.8, 0.9};
    auto result = tester.kolmogorovSmirnov(sample, uniform01Cdf, 0.95);

    // For this sample against U(0,1), D = max(D+, D-) = 0.1000.
    // The implemented critical value follows the standard asymptotic form:
    // sqrt(-0.5 * ln(alpha / 2) / n), which gives 0.5133 for alpha=0.05, n=7.
    EXPECT_NEAR(result.testStat(), 0.1000, kReferenceTolerance);
    EXPECT_NEAR(result.acceptanceInferiorLimit(), 0.0, kReferenceTolerance);
    EXPECT_NEAR(result.acceptanceSuperiorLimit(), 0.5133, kReferenceTolerance);
    EXPECT_NEAR(result.pValue(), 1.0000, kReferenceTolerance);
    EXPECT_FALSE(result.rejectH0());
}

// Test objective: verifies HypothesisTesterDefaultImplTest.ProportionDifferenceConfidenceIntervalHasExpectedCenterAndFiniteBounds.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.ProportionConfidenceIntervalWithoutFinitePopulationHasExpectedCenterAndFiniteBounds.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.ProportionConfidenceIntervalWithFinitePopulationShrinksHalfWidth.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.AverageAndVarianceConfidenceIntervalsHaveFiniteBounds.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.TwoPopulationConfidenceIntervalsHaveExpectedCenters.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.TestAverageOnePopulationCoversNonRejectionAndRejectionWithValidPValue.
TEST(HypothesisTesterDefaultImplTest, TestAverageOnePopulationCoversNonRejectionAndRejectionWithValidPValue) {
    HypothesisTesterDefaultImpl tester;

    auto nonRejection = tester.testAverage(10.0, 2.0, 30, 10.2, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);

    auto rejection = tester.testAverage(10.0, 2.0, 30, 11.5, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.TestProportionOnePopulationReturnsValidPValues.
TEST(HypothesisTesterDefaultImplTest, TestProportionOnePopulationReturnsValidPValues) {
    HypothesisTesterDefaultImpl tester;

    auto nonRejection = tester.testProportion(0.50, 100, 0.54, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);

    auto rejection = tester.testProportion(0.50, 100, 0.80, 0.95, HypothesisTester_if::H1Comparition::GREATER_THAN);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.TestVarianceOnePopulationCoversNonRejectionAndRejectionWithValidPValue.
TEST(HypothesisTesterDefaultImplTest, TestVarianceOnePopulationCoversNonRejectionAndRejectionWithValidPValue) {
    HypothesisTesterDefaultImpl tester;

    auto nonRejection = tester.testVariance(4.0, 30, 4.2, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(nonRejection.rejectH0());
    expectValidResult(nonRejection);

    auto rejection = tester.testVariance(4.0, 30, 1.5, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_TRUE(rejection.rejectH0());
    expectValidResult(rejection);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.TwoPopulationTestsReturnValidPValues.
TEST(HypothesisTesterDefaultImplTest, TwoPopulationTestsReturnValidPValues) {
    HypothesisTesterDefaultImpl tester;

    expectValidResult(tester.testAverage(10.0, 2.0, 30, 8.5, 2.1, 28, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testProportion(0.60, 100, 0.45, 120, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
    expectValidResult(tester.testVariance(4.0, 30, 2.0, 28, 0.95, HypothesisTester_if::H1Comparition::DIFFERENT));
}

// Test objective: verifies HypothesisTesterDefaultImplTest.FileBasedMethodsUseAnalysisDatasetLoader.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.InvalidConfidenceLevelThrows.
TEST(HypothesisTesterDefaultImplTest, InvalidConfidenceLevelThrows) {
    HypothesisTesterDefaultImpl tester;

    EXPECT_THROW(tester.averageConfidenceInterval(10.0, 2.0, 30, 1.0), std::invalid_argument);
    EXPECT_THROW(tester.testAverage(10.0, 2.0, 30, 10.2, 0.0, HypothesisTester_if::H1Comparition::DIFFERENT), std::invalid_argument);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.ChiSquareGoodnessOfFitCoversNonRejectionAndRejection.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.ChiSquareGoodnessOfFitBuildsFrequenciesFromSampleAndExplicitClasses.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.ChiSquareGoodnessOfFitSupportsAutomaticClasses.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.ChiSquareGoodnessOfFitGroupsLowExpectedClasses.
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
    ASSERT_TRUE(result.hasGoodnessOfFitDetails());
    const auto details = result.goodnessOfFitDetails();
    EXPECT_EQ(details.initialClasses, 10u);
    EXPECT_EQ(details.effectiveClasses, 2u);
    EXPECT_EQ(details.estimatedParameters, 0u);
    EXPECT_DOUBLE_EQ(details.degreesOfFreedom, 1.0);
    EXPECT_NEAR(result.testStat(), 0.0, 1e-9);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.ChiSquareGoodnessOfFitRejectsInvalidInputs.
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
    EXPECT_THROW(tester.chiSquareGoodnessOfFit(std::vector<double>{0.1, 0.2, 0.8, 0.9}, uniform01Cdf, std::vector<double>{0.0, 0.5, 1.0}, 1, 0.95), std::invalid_argument);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.KolmogorovSmirnovCoversNonRejectionAndRejection.
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

// Test objective: verifies HypothesisTesterDefaultImplTest.KolmogorovSmirnovFileBasedOverloadUsesAnalysisDatasetLoader.
TEST(HypothesisTesterDefaultImplTest, KolmogorovSmirnovFileBasedOverloadUsesAnalysisDatasetLoader) {
    HypothesisTesterDefaultImpl tester;
    const std::string sample = writeSampleFile("genesys_ks_uniform_sample.txt", "0.1\n0.2\n0.35\n0.5\n0.65\n0.8\n0.9\n");

    auto result = tester.kolmogorovSmirnov(sample, uniform01Cdf, 0.95);

    EXPECT_FALSE(result.rejectH0());
    expectValidResult(result);
    EXPECT_GE(result.testStat(), 0.0);
    EXPECT_LE(result.testStat(), 1.0);
}

// Test objective: verifies HypothesisTesterDefaultImplTest.KolmogorovSmirnovRejectsInvalidInputs.
TEST(HypothesisTesterDefaultImplTest, KolmogorovSmirnovRejectsInvalidInputs) {
    HypothesisTesterDefaultImpl tester;

    EXPECT_THROW(tester.kolmogorovSmirnov(std::vector<double>{}, uniform01Cdf, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.kolmogorovSmirnov(std::vector<double>{0.1, 0.2}, distributionCdfFunction{}, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.kolmogorovSmirnov(std::vector<double>{0.1, 0.2}, [](double) { return 2.0; }, 0.95), std::invalid_argument);
    EXPECT_THROW(tester.kolmogorovSmirnov(std::vector<double>{0.1, 0.2}, uniform01Cdf, 0.0), std::invalid_argument);
}

} // namespace
