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

TEST(DataAnalyserDefaultImplTest, LoadDataSetConfiguresFitter) {
    DataAnalyserDefaultImpl analyser;
    const auto path = writeSampleFile("genesys_dataanalyser_valid");

    EXPECT_TRUE(analyser.loadDataSet(path.string()));
    EXPECT_EQ(analyser.fitter()->getDataFilename(), path.string());

    std::filesystem::remove(path);
}

TEST(DataAnalyserDefaultImplTest, LoadInMemoryDataSetConfiguresFitter) {
    DataAnalyserDefaultImpl analyser;

    ASSERT_TRUE(analyser.loadDataSet(std::vector<double>{3.0, 5.0, 7.0}));
    EXPECT_TRUE(analyser.fitter()->getDataFilename().empty());

    double sqrerror = 0.0;
    double avg = 0.0;
    double stddev = 0.0;
    analyser.fitter()->fitNormal(&sqrerror, &avg, &stddev);

    EXPECT_TRUE(std::isfinite(sqrerror));
    EXPECT_DOUBLE_EQ(avg, 5.0);
    EXPECT_DOUBLE_EQ(stddev, 2.0);
}

TEST(DataAnalyserDefaultImplTest, LoadDataSetReturnsFalseForInvalidDataset) {
    DataAnalyserDefaultImpl analyser;
    const auto path = std::filesystem::temp_directory_path()
                    / "genesys_dataanalyser_missing_dataset.csv";

    EXPECT_FALSE(analyser.loadDataSet(path.string()));
    EXPECT_TRUE(analyser.fitter()->getDataFilename().empty());
}

TEST(DataAnalyserDefaultImplTest, LoadInMemoryDataSetReturnsFalseForInvalidDataset) {
    DataAnalyserDefaultImpl analyser;

    EXPECT_FALSE(analyser.loadDataSet(std::vector<double>{}));
    EXPECT_TRUE(analyser.fitter()->getDataFilename().empty());
}

TEST(DataAnalyserDefaultImplTest, FutureSamplerAndExperimenterPathsThrowWhenNotInjected) {
    DataAnalyserDefaultImpl analyser;

    EXPECT_THROW(analyser.sampler(), std::runtime_error);
    EXPECT_THROW(analyser.experimenter(), std::runtime_error);
}

} // namespace
