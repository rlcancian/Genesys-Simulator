#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "tools/DataAnalyzerDefaultImpl1.h"

namespace {

constexpr double kTolerance = 1e-6;

TEST(DataAnalyzerDefaultImpl1Test, EmptyDataYieldsEmptyStatistics) {
    DataAnalyzerDefaultImpl1 analyzer;
    auto stats = analyzer.summaryStatistics();
    
    EXPECT_EQ(stats.n, 0);
    EXPECT_EQ(stats.mean, 0.0);
    EXPECT_EQ(stats.median, 0.0);
    EXPECT_EQ(stats.mode, 0.0);
    EXPECT_EQ(stats.skewness, 0.0);
    EXPECT_EQ(stats.kurtosis, 0.0);
    
    EXPECT_TRUE(std::isnan(analyzer.quartile(1)));
    EXPECT_TRUE(std::isnan(analyzer.decile(5)));
    EXPECT_TRUE(std::isnan(analyzer.centile(50)));
}

TEST(DataAnalyzerDefaultImpl1Test, LoadingDataAndClearDataUpdatesCount) {
    DataAnalyzerDefaultImpl1 analyzer;
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    analyzer.setDataValues(data);
    EXPECT_EQ(analyzer.summaryStatistics().n, 5);
    
    analyzer.clearData();
    EXPECT_EQ(analyzer.summaryStatistics().n, 0);
}

TEST(DataAnalyzerDefaultImpl1Test, SummaryStatisticsBasicValuesAreCorrect) {
    DataAnalyzerDefaultImpl1 analyzer;
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0}; // mean 3, min 1, max 5, range 4, var 2.5, std 1.5811388
    analyzer.setDataValues(data);
    
    auto stats = analyzer.summaryStatistics();
    EXPECT_EQ(stats.n, 5);
    EXPECT_NEAR(stats.min, 1.0, kTolerance);
    EXPECT_NEAR(stats.max, 5.0, kTolerance);
    EXPECT_NEAR(stats.range, 4.0, kTolerance);
    EXPECT_NEAR(stats.mean, 3.0, kTolerance);
    EXPECT_NEAR(stats.median, 3.0, kTolerance);
    
    EXPECT_NEAR(stats.variance, 2.5, kTolerance);
    EXPECT_NEAR(stats.stddev, std::sqrt(2.5), kTolerance);
    EXPECT_NEAR(stats.cv, std::sqrt(2.5)/3.0, kTolerance);
}

TEST(DataAnalyzerDefaultImpl1Test, SummaryStatisticsSkewnessAndKurtosisFollowFisherBiasCorrectedFormulas) {
    DataAnalyzerDefaultImpl1 analyzer;
    // Data with known skewness and kurtosis using Fisher's exact formula
    // (matches Excel SKEW and KURT functions)
    std::vector<double> data = {2.0, 5.0, 8.0, 12.0, 14.0, 21.0, 23.0, 29.0};
    analyzer.setDataValues(data);
    
    auto stats = analyzer.summaryStatistics();
    
    EXPECT_NEAR(stats.skewness, 0.297705, 1e-5);
    EXPECT_NEAR(stats.kurtosis, -1.106348, 1e-5);
}

TEST(DataAnalyzerDefaultImpl1Test, QuantilesDecilesAndCentilesReturnCorrectInterpolatedValues) {
    DataAnalyzerDefaultImpl1 analyzer;
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    analyzer.setDataValues(data);
    
    // Using R method 7 (linear interpolation):
    // Q1 (p=0.25): h = (5-1)*0.25 = 1 -> index 1 -> value 2.0
    // Q2 (p=0.50): h = 4*0.5 = 2 -> index 2 -> value 3.0
    // Q3 (p=0.75): h = 4*0.75 = 3 -> index 3 -> value 4.0
    
    EXPECT_NEAR(analyzer.quartile(1), 2.0, kTolerance);
    EXPECT_NEAR(analyzer.quartile(2), 3.0, kTolerance);
    EXPECT_NEAR(analyzer.quartile(3), 4.0, kTolerance);
    
    // Decile 2 (p=0.20): h = 4*0.2 = 0.8 -> 0.2*data[0] + 0.8*data[1] = 0.2*1 + 0.8*2 = 1.8
    EXPECT_NEAR(analyzer.decile(2), 1.8, kTolerance);
    
    // Centile 25 = Q1
    EXPECT_NEAR(analyzer.centile(25), 2.0, kTolerance);
}

TEST(DataAnalyzerDefaultImpl1Test, HistogramStructureGroupsDataCorrectly) {
    DataAnalyzerDefaultImpl1 analyzer;
    std::vector<double> data = {1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5};
    analyzer.setDataValues(data);
    
    // min is 1.0, max is 5.5. Range = 4.5
    // numClasses = 3
    // width = 4.5 / 3 = 1.5
    // Class 0: [1.0, 2.5) -> 1.0, 1.5, 2.0 (3 items)
    // Class 1: [2.5, 4.0) -> 2.5, 3.0, 3.5 (3 items)
    // Class 2: [4.0, 5.5] -> 4.0, 4.5, 5.0, 5.5 (4 items)
    auto hist = analyzer.histogramStructure(3);
    
    EXPECT_EQ(hist.numClasses, 3);
    ASSERT_EQ(hist.frequencies.size(), 3);
    EXPECT_EQ(hist.frequencies[0], 3);
    EXPECT_EQ(hist.frequencies[1], 3);
    EXPECT_EQ(hist.frequencies[2], 4);
    
    EXPECT_NEAR(hist.lowerLimits[0], 1.0, kTolerance);
    EXPECT_NEAR(hist.lowerLimits[1], 2.5, kTolerance);
    EXPECT_NEAR(hist.lowerLimits[2], 4.0, kTolerance);
    
    EXPECT_NEAR(hist.relativeFrequencies[0], 0.3, kTolerance);
    EXPECT_NEAR(hist.relativeFrequencies[1], 0.3, kTolerance);
    EXPECT_NEAR(hist.relativeFrequencies[2], 0.4, kTolerance);
}

TEST(DataAnalyzerDefaultImpl1Test, BoxplotStatisticsDetectsOutliers) {
    DataAnalyzerDefaultImpl1 analyzer;
    std::vector<double> data = {2.0, 3.0, 3.5, 4.0, 4.0, 4.5, 5.0, 15.0, -10.0};
    analyzer.setDataValues(data);
    
    auto boxplot = analyzer.boxplotStatistics();
    
    EXPECT_NEAR(boxplot.min, -10.0, kTolerance);
    EXPECT_NEAR(boxplot.max, 15.0, kTolerance);
    
    // Q1, Median, Q3 with R type 7:
    // n=9, p=0.25 -> h=(9-1)*0.25=2 -> index 2 -> sorted[2] -> (-10, 2, 3, 3.5, 4, 4, 4.5, 5, 15) -> Q1 = 3.0
    // Q3: p=0.75 -> h=6 -> sorted[6] -> 4.5
    // IQR = 4.5 - 3.0 = 1.5
    // fenceLow = 3.0 - 1.5*1.5 = 0.75
    // fenceHigh = 4.5 + 1.5*1.5 = 6.75
    // Outliers: < 0.75 or > 6.75 -> -10.0 and 15.0
    
    EXPECT_NEAR(boxplot.q1, 3.0, kTolerance);
    EXPECT_NEAR(boxplot.q3, 4.5, kTolerance);
    EXPECT_NEAR(boxplot.iqr, 1.5, kTolerance);
    
    ASSERT_EQ(boxplot.outliers.size(), 2);
    EXPECT_TRUE(std::find(boxplot.outliers.begin(), boxplot.outliers.end(), -10.0) != boxplot.outliers.end());
    EXPECT_TRUE(std::find(boxplot.outliers.begin(), boxplot.outliers.end(), 15.0) != boxplot.outliers.end());
}

TEST(DataAnalyzerDefaultImpl1Test, FitDistributionReturnsValidResultsForCommonDistributions) {
    DataAnalyzerDefaultImpl1 analyzer;
    // Simple uniform-like data in [0, 10]
    std::vector<double> data = {0.1, 1.2, 2.5, 3.8, 4.5, 5.5, 6.2, 7.5, 8.8, 9.9};
    analyzer.setDataValues(data);
    
    auto fitUni = analyzer.fitDistribution("uniform");
    EXPECT_TRUE(fitUni.valid);
    EXPECT_EQ(fitUni.distributionName, "uniform");
    EXPECT_NEAR(fitUni.param1, 0.1, kTolerance); // min
    EXPECT_NEAR(fitUni.param2, 9.9, kTolerance); // max
    EXPECT_TRUE(std::isfinite(fitUni.sse));
    
    auto fitNorm = analyzer.fitDistribution("normal");
    EXPECT_TRUE(fitNorm.valid);
    EXPECT_NEAR(fitNorm.param1, 5.0, kTolerance); // mean
    EXPECT_TRUE(std::isfinite(fitNorm.param2));   // stddev
    EXPECT_TRUE(std::isfinite(fitNorm.sse));
}

TEST(DataAnalyzerDefaultImpl1Test, FitAllRankedReturnsAllSevenDistributionsSortedBySSE) {
    DataAnalyzerDefaultImpl1 analyzer;
    // Data roughly exponential
    std::vector<double> data = {0.1, 0.2, 0.4, 0.8, 1.6, 3.2, 6.4};
    analyzer.setDataValues(data);
    
    auto ranked = analyzer.fitAllRanked();
    EXPECT_EQ(ranked.size(), 7); // uniform, triangular, normal, expo, erlang, beta, weibull
    
    // Check that it's sorted by SSE ascending for valid fits
    for (size_t i = 1; i < ranked.size(); ++i) {
        if (ranked[i-1].valid && ranked[i].valid) {
            EXPECT_LE(ranked[i-1].sse, ranked[i].sse);
        }
    }
    
    // fitAll() should return the best one, which matches the first valid element of ranked
    auto best = analyzer.fitAll();
    
    // Find first valid in ranked
    size_t firstValid = 0;
    while (firstValid < ranked.size() && !ranked[firstValid].valid) {
        firstValid++;
    }
    
    if (firstValid < ranked.size()) {
        EXPECT_TRUE(best.valid);
        EXPECT_EQ(best.distributionName, ranked[firstValid].distributionName);
        EXPECT_EQ(best.sse, ranked[firstValid].sse);
    }
}

TEST(DataAnalyzerDefaultImpl1Test, GoodnessOfFitTestsProduceValidConclusions) {
    DataAnalyzerDefaultImpl1 analyzer;
    // 50 samples of roughly normal data to pass GoF
    std::vector<double> data = {
        4.1, 4.3, 4.5, 4.7, 4.8, 4.9, 4.9, 5.0, 5.1, 5.2,
        5.2, 5.3, 5.4, 5.5, 5.5, 5.6, 5.7, 5.8, 5.9, 6.0,
        4.6, 4.8, 5.1, 5.3, 5.6, 5.9, 4.4, 5.4, 4.2, 5.8,
        5.0, 5.0, 5.1, 4.9, 5.1, 5.0, 4.9, 5.0, 5.1, 4.8,
        4.9, 5.2, 5.3, 4.7, 5.1, 5.0, 5.1, 5.2, 5.3, 5.4
    };
    analyzer.setDataValues(data);
    
    // chi-square test for normality
    auto chi2 = analyzer.chiSquareGoodnessOfFit("normal", 0.05);
    EXPECT_EQ(chi2.distributionName, "normal");
    EXPECT_NEAR(chi2.significanceLevel, 0.05, kTolerance);
    EXPECT_TRUE(std::isfinite(chi2.testStatistic));
    EXPECT_TRUE(std::isfinite(chi2.pValue));
    EXPECT_TRUE(std::isfinite(chi2.criticalValue));
    
    // KS test
    auto ks = analyzer.kolmogorovSmirnov("normal", 0.05);
    EXPECT_EQ(ks.distributionName, "normal");
    EXPECT_NEAR(ks.significanceLevel, 0.05, kTolerance);
    EXPECT_TRUE(std::isfinite(ks.testStatistic));
    EXPECT_TRUE(std::isfinite(ks.pValue));
    EXPECT_TRUE(std::isfinite(ks.criticalValue));
}

TEST(DataAnalyzerDefaultImpl1Test, TimeSeriesAnalysisReturnsCorrectMovingAverageAndCorrelogram) {
    DataAnalyzerDefaultImpl1 analyzer;
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    analyzer.setDataValues(data);
    
    // Moving Average (window=3)
    auto ma = analyzer.movingAverage(3);
    ASSERT_EQ(ma.size(), 3); // n - w + 1 = 5 - 3 + 1 = 3
    EXPECT_NEAR(ma[0], (1+2+3)/3.0, kTolerance);
    EXPECT_NEAR(ma[1], (2+3+4)/3.0, kTolerance);
    EXPECT_NEAR(ma[2], (3+4+5)/3.0, kTolerance);
    
    // Autocorrelation (maxLag=2)
    auto acf = analyzer.autocorrelation(2);
    ASSERT_EQ(acf.size(), 3);
    EXPECT_NEAR(acf[0], 1.0, kTolerance); // lag 0 is always 1.0
    
    // Correlogram
    auto correlogram = analyzer.correlogram(2);
    ASSERT_EQ(correlogram.acf.size(), 3);
    EXPECT_NEAR(correlogram.acf[0], 1.0, kTolerance);
    EXPECT_EQ(correlogram.acf, acf);
    EXPECT_EQ(correlogram.n, 5);
    // 95% confidence bound: 1.96 / sqrt(5) = 1.96 / 2.236 = 0.8765
    EXPECT_NEAR(correlogram.confidenceBound, 1.96 / std::sqrt(5.0), kTolerance);
}

TEST(DataAnalyzerDefaultImpl1Test, InferenceOnePopulationConfidenceIntervalsAreFinite) {
    DataAnalyzerDefaultImpl1 analyzer;
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    analyzer.setDataValues(data);
    analyzer.setConfidenceLevel(0.95);
    
    auto avgCi = analyzer.averageConfidenceInterval();
    EXPECT_TRUE(std::isfinite(avgCi.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(avgCi.superiorLimit()));
    
    auto varCi = analyzer.varianceConfidenceInterval();
    EXPECT_TRUE(std::isfinite(varCi.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(varCi.superiorLimit()));
    
    auto propCi = analyzer.proportionConfidenceInterval([](double v){ return v > 5.0; });
    EXPECT_TRUE(std::isfinite(propCi.inferiorLimit()));
    EXPECT_TRUE(std::isfinite(propCi.superiorLimit()));
}

TEST(DataAnalyzerDefaultImpl1Test, InferenceTwoPopulationGuardsAgainstEmptySecondDataset) {
    DataAnalyzerDefaultImpl1 analyzer;
    std::vector<double> data1 = {1.0, 2.0, 3.0, 4.0, 5.0};
    analyzer.setDataValues(data1);
    
    // _data2 is empty by default
    auto tAvg = analyzer.testAverageTwoSamples(HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(tAvg.rejectH0());
    EXPECT_EQ(tAvg.testStat(), 0.0);
    
    auto tVar = analyzer.testVarianceTwoSamples(HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(tVar.rejectH0());
    EXPECT_EQ(tVar.testStat(), 0.0);
    
    auto tProp = analyzer.testProportionTwoSamples([](double v){ return v > 2.0; }, HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_FALSE(tProp.rejectH0());
    EXPECT_EQ(tProp.testStat(), 0.0);
    
    // Now load valid second dataset
    std::vector<double> data2 = {2.0, 3.0, 4.0, 5.0, 6.0};
    EXPECT_TRUE(analyzer.loadSecondSample(data2));
    
    tAvg = analyzer.testAverageTwoSamples(HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_TRUE(std::isfinite(tAvg.testStat()));
    
    tVar = analyzer.testVarianceTwoSamples(HypothesisTester_if::H1Comparition::DIFFERENT);
    EXPECT_TRUE(std::isfinite(tVar.testStat()));
}

} // namespace
