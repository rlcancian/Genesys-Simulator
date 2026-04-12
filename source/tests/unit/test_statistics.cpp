#include <gtest/gtest.h>

#include <cmath>

#include "kernel/statistics/CollectorDefaultImpl1.h"
#include "kernel/statistics/StatisticsDefaultImpl1.h"

namespace {

constexpr double kTolerance = 1e-9;

TEST(CollectorTest, InitialStateStartsEmpty) {
    CollectorDefaultImpl1 collector;

    EXPECT_EQ(collector.numElements(), 0u);
}

TEST(CollectorTest, AddValueUpdatesCountAndLastValue) {
    CollectorDefaultImpl1 collector;

    collector.addValue(4.25);

    EXPECT_EQ(collector.numElements(), 1u);
    EXPECT_DOUBLE_EQ(collector.getLastValue(), 4.25);
}

TEST(CollectorTest, AddValueInvokesCallbackWithValueAndWeight) {
    CollectorDefaultImpl1 collector;

    double capturedValue = 0.0;
    double capturedWeight = 0.0;
    unsigned int callbackCount = 0;

    collector.setAddValueHandler([&](double value, double weight) {
        capturedValue = value;
        capturedWeight = weight;
        ++callbackCount;
    });

    collector.addValue(9.5, 3.0);

    EXPECT_EQ(callbackCount, 1u);
    EXPECT_DOUBLE_EQ(capturedValue, 9.5);
    EXPECT_DOUBLE_EQ(capturedWeight, 3.0);
}

TEST(CollectorTest, ClearResetsCountAndInvokesCallback) {
    CollectorDefaultImpl1 collector;

    unsigned int clearCallbackCount = 0;
    collector.setClearHandler([&]() {
        ++clearCallbackCount;
    });

    collector.addValue(1.0);
    collector.addValue(2.0);

    ASSERT_EQ(collector.numElements(), 2u);

    collector.clear();

    EXPECT_EQ(collector.numElements(), 0u);
    EXPECT_EQ(clearCallbackCount, 1u);
}

TEST(CollectorTest, MultipleInsertionsKeepLastValueAndCount) {
    CollectorDefaultImpl1 collector;

    collector.addValue(1.0);
    collector.addValue(2.0);
    collector.addValue(3.5);

    EXPECT_EQ(collector.numElements(), 3u);
    EXPECT_DOUBLE_EQ(collector.getLastValue(), 3.5);
}

TEST(StatisticsTest, DefaultsAreStableWithExplicitCollector) {
    CollectorDefaultImpl1 collector;
    StatisticsDefaultImpl1 stats(&collector);

    EXPECT_EQ(stats.numElements(), 0u);
    EXPECT_DOUBLE_EQ(stats.min(), 0.0);
    EXPECT_DOUBLE_EQ(stats.max(), 0.0);
    EXPECT_DOUBLE_EQ(stats.average(), 0.0);
    EXPECT_DOUBLE_EQ(stats.variance(), 0.0);
    EXPECT_DOUBLE_EQ(stats.stddeviation(), 0.0);
    EXPECT_DOUBLE_EQ(stats.variationCoef(), 0.0);
    EXPECT_DOUBLE_EQ(stats.halfWidthConfidenceInterval(), 0.0);
    EXPECT_DOUBLE_EQ(stats.confidenceLevel(), 0.95);
}

TEST(StatisticsTest, IncrementalUpdatesMatchAnalyticalValuesForThreeSamples) {
    CollectorDefaultImpl1 collector;
    StatisticsDefaultImpl1 stats(&collector);

    collector.addValue(1.0);
    collector.addValue(2.0);
    collector.addValue(3.0);

    EXPECT_EQ(stats.numElements(), 3u);
    EXPECT_DOUBLE_EQ(stats.min(), 1.0);
    EXPECT_DOUBLE_EQ(stats.max(), 3.0);
    EXPECT_NEAR(stats.average(), 2.0, kTolerance);
    EXPECT_NEAR(stats.variance(), 1.0, kTolerance);
    EXPECT_NEAR(stats.stddeviation(), 1.0, kTolerance);
    EXPECT_NEAR(stats.variationCoef(), 0.5, kTolerance);
    EXPECT_NEAR(stats.halfWidthConfidenceInterval(), 1.96 / std::sqrt(3.0), kTolerance);
}

TEST(StatisticsTest, CollectorClearResetsStatisticsViaCallback) {
    CollectorDefaultImpl1 collector;
    StatisticsDefaultImpl1 stats(&collector);

    collector.addValue(10.0);
    collector.addValue(20.0);

    ASSERT_EQ(stats.numElements(), 2u);
    ASSERT_DOUBLE_EQ(stats.max(), 20.0);

    collector.clear();

    EXPECT_EQ(stats.numElements(), 0u);
    EXPECT_DOUBLE_EQ(stats.min(), 0.0);
    EXPECT_DOUBLE_EQ(stats.max(), 0.0);
    EXPECT_DOUBLE_EQ(stats.average(), 0.0);
    EXPECT_DOUBLE_EQ(stats.variance(), 0.0);
    EXPECT_DOUBLE_EQ(stats.stddeviation(), 0.0);
    EXPECT_DOUBLE_EQ(stats.variationCoef(), 0.0);
    EXPECT_DOUBLE_EQ(stats.halfWidthConfidenceInterval(), 0.0);
}

TEST(StatisticsTest, ConfidenceLevelSetterUpdatesObservableState) {
    CollectorDefaultImpl1 collector;
    StatisticsDefaultImpl1 stats(&collector);

    stats.setConfidenceLevel(0.9);

    EXPECT_DOUBLE_EQ(stats.confidenceLevel(), 0.9);
}

TEST(StatisticsTest, NewSampleSizeCurrentlyReturnsZero) {
    CollectorDefaultImpl1 collector;
    StatisticsDefaultImpl1 stats(&collector);

    collector.addValue(1.0);
    collector.addValue(2.0);

    EXPECT_EQ(stats.newSampleSize(0.25), 0u);
}

} // namespace
