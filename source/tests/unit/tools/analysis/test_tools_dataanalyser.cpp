#include <gtest/gtest.h>

#include <stdexcept>

#include "tools/analysis/DataAnalyserDefaultImpl.h"

namespace {

TEST(DataAnalyserDefaultImplTest, ProvidesDefaultFitterAndTester) {
    DataAnalyserDefaultImpl analyser;

    ASSERT_NE(analyser.fitter(), nullptr);
    ASSERT_NE(analyser.tester(), nullptr);
}

TEST(DataAnalyserDefaultImplTest, LoadDataSetConfiguresFitter) {
    DataAnalyserDefaultImpl analyser;

    EXPECT_TRUE(analyser.loadDataSet("sample-data.txt"));
    EXPECT_EQ(analyser.fitter()->getDataFilename(), "sample-data.txt");
}

TEST(DataAnalyserDefaultImplTest, FutureSamplerAndExperimenterPathsThrowWhenNotInjected) {
    DataAnalyserDefaultImpl analyser;

    EXPECT_THROW(analyser.sampler(), std::runtime_error);
    EXPECT_THROW(analyser.experimenter(), std::runtime_error);
}

} // namespace
