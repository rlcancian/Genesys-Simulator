#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "tools/analysis/DataAnalyserDefaultImpl.h"

namespace {

std::filesystem::path writeSampleFile(const std::string& name) {
    const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path()
                    / (name + "_" + std::to_string(suffix) + ".csv");
    std::ofstream file(path);
    file << "1.0\n2.0\n3.0\n";
    return path;
}

TEST(DataAnalyserDefaultImplTest, ProvidesDefaultFitterAndTester) {
    DataAnalyserDefaultImpl analyser;

    ASSERT_NE(analyser.fitter(), nullptr);
    ASSERT_NE(analyser.tester(), nullptr);
}

TEST(DataAnalyserDefaultImplTest, SummaryStartsEmpty) {
    DataAnalyserDefaultImpl analyser;

    const DataSetSummary summary = analyser.summary();

    EXPECT_FALSE(summary.usable);
    EXPECT_EQ(summary.count, 0u);
    EXPECT_DOUBLE_EQ(summary.min, 0.0);
    EXPECT_DOUBLE_EQ(summary.max, 0.0);
    EXPECT_DOUBLE_EQ(summary.mean, 0.0);
    EXPECT_DOUBLE_EQ(summary.variance, 0.0);
    EXPECT_DOUBLE_EQ(summary.stddev, 0.0);
    EXPECT_FALSE(summary.hasNegativeData);
}

TEST(DataAnalyserDefaultImplTest, HistogramStartsEmpty) {
    DataAnalyserDefaultImpl analyser;

    const DataSetHistogram histogram = analyser.histogram();

    EXPECT_FALSE(histogram.usable);
    EXPECT_EQ(histogram.count, 0u);
    EXPECT_TRUE(histogram.bins.empty());
}

TEST(DataAnalyserDefaultImplTest, LoadDataSetConfiguresFitter) {
    DataAnalyserDefaultImpl analyser;
    const auto path = writeSampleFile("genesys_dataanalyser_valid");

    EXPECT_TRUE(analyser.loadDataSet(path.string()));
    EXPECT_EQ(analyser.fitter()->getDataFilename(), path.string());

    const DataSetSummary summary = analyser.summary();
    EXPECT_TRUE(summary.usable);
    EXPECT_EQ(summary.count, 3u);
    EXPECT_DOUBLE_EQ(summary.min, 1.0);
    EXPECT_DOUBLE_EQ(summary.max, 3.0);
    EXPECT_DOUBLE_EQ(summary.mean, 2.0);
    EXPECT_DOUBLE_EQ(summary.variance, 1.0);
    EXPECT_DOUBLE_EQ(summary.stddev, 1.0);
    EXPECT_FALSE(summary.hasNegativeData);

    std::filesystem::remove(path);
}

TEST(DataAnalyserDefaultImplTest, HistogramReturnsNumericBinsWithFrequencies) {
    DataAnalyserDefaultImpl analyser;
    ASSERT_TRUE(analyser.loadDataSet(std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0}));

    const DataSetHistogram histogram = analyser.histogram(2);

    ASSERT_TRUE(histogram.usable);
    EXPECT_EQ(histogram.count, 5u);
    EXPECT_DOUBLE_EQ(histogram.min, 1.0);
    EXPECT_DOUBLE_EQ(histogram.max, 5.0);
    EXPECT_DOUBLE_EQ(histogram.classWidth, 2.0);
    ASSERT_EQ(histogram.bins.size(), 2u);

    EXPECT_DOUBLE_EQ(histogram.bins[0].lowerLimit, 1.0);
    EXPECT_DOUBLE_EQ(histogram.bins[0].upperLimit, 3.0);
    EXPECT_EQ(histogram.bins[0].frequency, 2u);
    EXPECT_DOUBLE_EQ(histogram.bins[0].relativeFrequency, 0.4);

    EXPECT_DOUBLE_EQ(histogram.bins[1].lowerLimit, 3.0);
    EXPECT_DOUBLE_EQ(histogram.bins[1].upperLimit, 5.0);
    EXPECT_EQ(histogram.bins[1].frequency, 3u);
    EXPECT_DOUBLE_EQ(histogram.bins[1].relativeFrequency, 0.6);
}

TEST(DataAnalyserDefaultImplTest, HistogramUsesSingleBinForConstantData) {
    DataAnalyserDefaultImpl analyser;
    ASSERT_TRUE(analyser.loadDataSet(std::vector<double>{4.0, 4.0, 4.0}));

    const DataSetHistogram histogram = analyser.histogram(5);

    ASSERT_TRUE(histogram.usable);
    EXPECT_EQ(histogram.count, 3u);
    EXPECT_DOUBLE_EQ(histogram.min, 4.0);
    EXPECT_DOUBLE_EQ(histogram.max, 4.0);
    EXPECT_DOUBLE_EQ(histogram.classWidth, 0.0);
    ASSERT_EQ(histogram.bins.size(), 1u);
    EXPECT_DOUBLE_EQ(histogram.bins[0].lowerLimit, 4.0);
    EXPECT_DOUBLE_EQ(histogram.bins[0].upperLimit, 4.0);
    EXPECT_EQ(histogram.bins[0].frequency, 3u);
    EXPECT_DOUBLE_EQ(histogram.bins[0].relativeFrequency, 1.0);
}

TEST(DataAnalyserDefaultImplTest, LoadInMemoryDataSetConfiguresFitter) {
    DataAnalyserDefaultImpl analyser;

    ASSERT_TRUE(analyser.loadDataSet(std::vector<double>{3.0, 5.0, 7.0}));
    EXPECT_TRUE(analyser.fitter()->getDataFilename().empty());

    const DataSetSummary summary = analyser.summary();
    EXPECT_TRUE(summary.usable);
    EXPECT_EQ(summary.count, 3u);
    EXPECT_DOUBLE_EQ(summary.min, 3.0);
    EXPECT_DOUBLE_EQ(summary.max, 7.0);
    EXPECT_DOUBLE_EQ(summary.mean, 5.0);
    EXPECT_DOUBLE_EQ(summary.variance, 4.0);
    EXPECT_DOUBLE_EQ(summary.stddev, 2.0);
    EXPECT_FALSE(summary.hasNegativeData);

    double sqrerror = 0.0;
    double avg = 0.0;
    double stddev = 0.0;
    analyser.fitter()->fitNormal(&sqrerror, &avg, &stddev);

    EXPECT_TRUE(std::isfinite(sqrerror));
    EXPECT_DOUBLE_EQ(avg, 5.0);
    EXPECT_DOUBLE_EQ(stddev, 2.0);
}

TEST(DataAnalyserDefaultImplTest, BoxplotReturnsQuartilesAndWhiskers) {
    DataAnalyserDefaultImpl analyser;
    ASSERT_TRUE(analyser.loadDataSet(std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0}));

    const DataSetBoxPlot boxplot = analyser.boxplot();

    ASSERT_TRUE(boxplot.usable);
    EXPECT_EQ(boxplot.count, 5u);
    EXPECT_DOUBLE_EQ(boxplot.min, 1.0);
    EXPECT_DOUBLE_EQ(boxplot.firstQuartile, 2.0);
    EXPECT_DOUBLE_EQ(boxplot.median, 3.0);
    EXPECT_DOUBLE_EQ(boxplot.thirdQuartile, 4.0);
    EXPECT_DOUBLE_EQ(boxplot.max, 5.0);
    EXPECT_DOUBLE_EQ(boxplot.interquartileRange, 2.0);
    EXPECT_DOUBLE_EQ(boxplot.lowerFence, -1.0);
    EXPECT_DOUBLE_EQ(boxplot.upperFence, 7.0);
    EXPECT_DOUBLE_EQ(boxplot.lowerWhisker, 1.0);
    EXPECT_DOUBLE_EQ(boxplot.upperWhisker, 5.0);
    EXPECT_TRUE(boxplot.outliers.empty());
}

TEST(DataAnalyserDefaultImplTest, BoxplotIdentifiesOutliers) {
    DataAnalyserDefaultImpl analyser;
    ASSERT_TRUE(analyser.loadDataSet(std::vector<double>{1.0, 2.0, 3.0, 4.0, 100.0}));

    const DataSetBoxPlot boxplot = analyser.boxplot();

    ASSERT_TRUE(boxplot.usable);
    EXPECT_DOUBLE_EQ(boxplot.firstQuartile, 2.0);
    EXPECT_DOUBLE_EQ(boxplot.median, 3.0);
    EXPECT_DOUBLE_EQ(boxplot.thirdQuartile, 4.0);
    EXPECT_DOUBLE_EQ(boxplot.lowerWhisker, 1.0);
    EXPECT_DOUBLE_EQ(boxplot.upperWhisker, 4.0);
    ASSERT_EQ(boxplot.outliers.size(), 1u);
    EXPECT_DOUBLE_EQ(boxplot.outliers[0], 100.0);
}

TEST(DataAnalyserDefaultImplTest, LoadDataSetReturnsFalseForInvalidDataset) {
    DataAnalyserDefaultImpl analyser;
    const auto path = std::filesystem::temp_directory_path()
                    / "genesys_dataanalyser_missing_dataset.csv";

    EXPECT_FALSE(analyser.loadDataSet(path.string()));
    EXPECT_TRUE(analyser.fitter()->getDataFilename().empty());
    EXPECT_FALSE(analyser.summary().usable);
}

TEST(DataAnalyserDefaultImplTest, LoadInMemoryDataSetReturnsFalseForInvalidDataset) {
    DataAnalyserDefaultImpl analyser;
    ASSERT_TRUE(analyser.loadDataSet(std::vector<double>{-1.0, 1.0, 3.0}));

    EXPECT_FALSE(analyser.loadDataSet(std::vector<double>{}));
    EXPECT_TRUE(analyser.fitter()->getDataFilename().empty());

    const DataSetSummary summary = analyser.summary();
    EXPECT_TRUE(summary.usable);
    EXPECT_EQ(summary.count, 3u);
    EXPECT_DOUBLE_EQ(summary.min, -1.0);
    EXPECT_DOUBLE_EQ(summary.max, 3.0);
    EXPECT_DOUBLE_EQ(summary.mean, 1.0);
    EXPECT_DOUBLE_EQ(summary.variance, 4.0);
    EXPECT_DOUBLE_EQ(summary.stddev, 2.0);
    EXPECT_TRUE(summary.hasNegativeData);
}

TEST(DataAnalyserDefaultImplTest, FutureSamplerAndExperimenterPathsThrowWhenNotInjected) {
    DataAnalyserDefaultImpl analyser;

    EXPECT_THROW(analyser.sampler(), std::runtime_error);
    EXPECT_THROW(analyser.experimenter(), std::runtime_error);
}

} // namespace
