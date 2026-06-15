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

} // namespace
