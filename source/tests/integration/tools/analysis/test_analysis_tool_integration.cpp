#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <numeric>
#include <string>
#include <vector>

#include "tools/analysis/DataAnalyserDefaultImpl.h"

namespace {

constexpr double kConfidenceLevel = 0.95;

std::string repoPath(const std::string& relativePath) {
    return (std::filesystem::path(GENESYS_REPOSITORY_ROOT) / relativePath).string();
}

double normalCdf(double value, double mean, double stddev) {
    return 0.5 * std::erfc(-(value - mean) / (stddev * std::sqrt(2.0)));
}

bool isGreaterThan50(double value) {
    return value > 50.0;
}

void expectValidTestResult(const HypothesisTester_if::TestResult& result) {
    EXPECT_TRUE(std::isfinite(result.pValue()));
    EXPECT_GE(result.pValue(), 0.0);
    EXPECT_LE(result.pValue(), 1.0);
    EXPECT_TRUE(std::isfinite(result.testStat()));
}

} // namespace

TEST(AnalysisToolIntegrationTest, CompleteFacadeWorkflowFromBundledCsv) {
    DataAnalyserDefaultImpl analyser;
    ASSERT_TRUE(analyser.loadDataSet(repoPath("examples/data/sample_data.csv")));

    const auto summary = analyser.summary();
    ASSERT_TRUE(summary.usable);
    EXPECT_EQ(summary.count, 160u);
    EXPECT_NEAR(summary.mean, 50.3317, 1e-4);
    EXPECT_NEAR(summary.stddev, 9.8335, 1e-4);

    const auto histogram = analyser.histogram(10);
    ASSERT_TRUE(histogram.usable);
    ASSERT_EQ(histogram.bins.size(), 10u);
    const auto histogramTotal = std::accumulate(
            histogram.bins.begin(),
            histogram.bins.end(),
            std::size_t{0},
            [](std::size_t total, const HistogramBin& bin) {
                return total + bin.frequency;
            });
    EXPECT_EQ(histogramTotal, summary.count);

    const auto boxplot = analyser.boxplot();
    ASSERT_TRUE(boxplot.usable);
    EXPECT_LE(boxplot.min, boxplot.firstQuartile);
    EXPECT_LE(boxplot.firstQuartile, boxplot.median);
    EXPECT_LE(boxplot.median, boxplot.thirdQuartile);
    EXPECT_LE(boxplot.thirdQuartile, boxplot.max);

    double normalSse = 0.0;
    double normalMean = 0.0;
    double normalStddev = 0.0;
    analyser.fitter()->fitNormal(&normalSse, &normalMean, &normalStddev);
    EXPECT_TRUE(std::isfinite(normalSse));
    EXPECT_NEAR(normalMean, summary.mean, 1e-9);
    EXPECT_NEAR(normalStddev, summary.stddev, 1e-9);

    const auto fitSummary = analyser.fitter()->fitAllSummary();
    ASSERT_TRUE(fitSummary.success);
    ASSERT_FALSE(fitSummary.ranking.empty());
    EXPECT_EQ(fitSummary.bestFit.distributionName, "normal");
    EXPECT_EQ(fitSummary.ranking.front().distributionName, fitSummary.bestFit.distributionName);

    const auto meanInterval = analyser.tester()->averageConfidenceInterval(
            summary.mean,
            summary.stddev,
            static_cast<unsigned int>(summary.count),
            kConfidenceLevel);
    EXPECT_LT(meanInterval.inferiorLimit(), 50.0);
    EXPECT_GT(meanInterval.superiorLimit(), 50.0);

    const auto meanTest = analyser.tester()->testAverage(
            50.0,
            summary.stddev,
            static_cast<unsigned int>(summary.count),
            summary.mean,
            kConfidenceLevel,
            HypothesisTester_if::DIFFERENT);
    expectValidTestResult(meanTest);
    EXPECT_FALSE(meanTest.rejectH0());

    const auto fittedNormalCdf = [normalMean, normalStddev](double value) {
        return normalCdf(value, normalMean, normalStddev);
    };
    const auto chiSquare = analyser.tester()->chiSquareGoodnessOfFit(
            analyser.data(),
            fittedNormalCdf,
            2,
            kConfidenceLevel,
            10,
            5.0);
    expectValidTestResult(chiSquare);
    EXPECT_FALSE(chiSquare.rejectH0());
    ASSERT_TRUE(chiSquare.hasGoodnessOfFitDetails());
    EXPECT_EQ(chiSquare.goodnessOfFitDetails().observedTotal, static_cast<double>(summary.count));

    const auto ks = analyser.tester()->kolmogorovSmirnov(
            analyser.data(),
            fittedNormalCdf,
            kConfidenceLevel);
    expectValidTestResult(ks);
    EXPECT_FALSE(ks.rejectH0());
}

TEST(AnalysisToolIntegrationTest, FileMemoryAndFileBasedInferenceStayConsistent) {
    const auto primaryPath = repoPath("examples/data/sample_data.csv");
    const auto groupAPath = repoPath("examples/data/sample_group_a.csv");
    const auto groupBPath = repoPath("examples/data/sample_group_b.csv");

    DataAnalyserDefaultImpl fileAnalyser;
    ASSERT_TRUE(fileAnalyser.loadDataSet(primaryPath));

    DataAnalyserDefaultImpl memoryAnalyser;
    ASSERT_TRUE(memoryAnalyser.loadDataSet(fileAnalyser.data()));

    const auto fileSummary = fileAnalyser.summary();
    const auto memorySummary = memoryAnalyser.summary();
    EXPECT_EQ(memorySummary.count, fileSummary.count);
    EXPECT_NEAR(memorySummary.mean, fileSummary.mean, 1e-12);
    EXPECT_NEAR(memorySummary.stddev, fileSummary.stddev, 1e-12);

    const auto fileFit = fileAnalyser.fitter()->fitAllSummary();
    const auto memoryFit = memoryAnalyser.fitter()->fitAllSummary();
    ASSERT_TRUE(fileFit.success);
    ASSERT_TRUE(memoryFit.success);
    EXPECT_EQ(memoryFit.bestFit.distributionName, fileFit.bestFit.distributionName);
    EXPECT_NEAR(memoryFit.bestFit.squaredError, fileFit.bestFit.squaredError, 1e-12);

    const auto directMeanTest = fileAnalyser.tester()->testAverage(
            50.0,
            fileSummary.stddev,
            static_cast<unsigned int>(fileSummary.count),
            fileSummary.mean,
            kConfidenceLevel,
            HypothesisTester_if::DIFFERENT);
    const auto fileMeanTest = fileAnalyser.tester()->testAverage(
            primaryPath,
            50.0,
            kConfidenceLevel,
            HypothesisTester_if::DIFFERENT);
    expectValidTestResult(directMeanTest);
    expectValidTestResult(fileMeanTest);
    EXPECT_EQ(fileMeanTest.rejectH0(), directMeanTest.rejectH0());
    EXPECT_NEAR(fileMeanTest.testStat(), directMeanTest.testStat(), 1e-9);

    const auto groupMeanTest = fileAnalyser.tester()->testAverage(
            groupAPath,
            groupBPath,
            kConfidenceLevel,
            HypothesisTester_if::DIFFERENT);
    expectValidTestResult(groupMeanTest);
    EXPECT_TRUE(groupMeanTest.rejectH0());

    const auto groupVarianceTest = fileAnalyser.tester()->testVariance(
            groupAPath,
            groupBPath,
            kConfidenceLevel,
            HypothesisTester_if::DIFFERENT);
    expectValidTestResult(groupVarianceTest);
    EXPECT_FALSE(groupVarianceTest.rejectH0());

    const auto groupProportionTest = fileAnalyser.tester()->testProportion(
            groupAPath,
            groupBPath,
            isGreaterThan50,
            kConfidenceLevel,
            HypothesisTester_if::DIFFERENT);
    expectValidTestResult(groupProportionTest);
    EXPECT_TRUE(groupProportionTest.rejectH0());
}
