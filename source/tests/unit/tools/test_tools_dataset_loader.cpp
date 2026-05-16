#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "tools/DatasetLoader.h"

namespace {
	std::filesystem::path temporaryPath(const std::string& name, const std::string& extension) {
		const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
		return std::filesystem::temp_directory_path() / (name + "_" + std::to_string(suffix) + extension);
	}

	std::filesystem::path writeTextDatasetFile(const std::string& name, const std::string& contents) {
		const std::filesystem::path path = temporaryPath(name, ".txt");
		std::ofstream file(path);
		file << contents;
		return path;
	}

	std::filesystem::path writeBinaryDatasetFile(const std::string& name, const std::vector<double>& values) {
		const std::filesystem::path path = temporaryPath(name, ".bin");
		std::ofstream file(path, std::ios::out | std::ios::binary);
		file.write(reinterpret_cast<const char*>(values.data()),
				static_cast<std::streamsize>(values.size() * sizeof(double)));
		return path;
	}
}

TEST(DatasetLoaderTest, StartsEmptyAndClearResetsState) {
	DatasetLoader loader;

	EXPECT_FALSE(loader.isLoaded());
	EXPECT_FALSE(loader.isUsable());
	EXPECT_TRUE(loader.data().empty());
	EXPECT_TRUE(loader.sortedData().empty());
	EXPECT_EQ(loader.count(), 0u);

	const std::filesystem::path path = writeTextDatasetFile("dataset_loader_clear", "1,2,3\n");
	ASSERT_TRUE(loader.loadFromFile(path.string()));
	ASSERT_TRUE(loader.isUsable());

	loader.clear();

	EXPECT_FALSE(loader.isLoaded());
	EXPECT_FALSE(loader.isUsable());
	EXPECT_TRUE(loader.data().empty());
	EXPECT_TRUE(loader.sortedData().empty());
	EXPECT_EQ(loader.count(), 0u);
	EXPECT_DOUBLE_EQ(loader.min(), 0.0);
	EXPECT_DOUBLE_EQ(loader.max(), 0.0);
	EXPECT_DOUBLE_EQ(loader.mean(), 0.0);
	EXPECT_DOUBLE_EQ(loader.variance(), 0.0);
	EXPECT_DOUBLE_EQ(loader.stddev(), 0.0);
	EXPECT_FALSE(loader.hasNegativeData());
}

TEST(DatasetLoaderTest, LoadsDelimitedTextAndComputesStatistics) {
	const std::filesystem::path path = writeTextDatasetFile(
			"dataset_loader_csv",
			"# ignored comment\n"
			"3.0, 1.0, 2.0\n"
			"\n"
			"-1.0,5.0\n");
	DatasetLoader loader;

	ASSERT_TRUE(loader.loadFromFile(path.string()));

	ASSERT_TRUE(loader.isLoaded());
	ASSERT_TRUE(loader.isUsable());
	ASSERT_EQ(loader.data().size(), 5u);
	EXPECT_DOUBLE_EQ(loader.data().at(0), 3.0);
	EXPECT_DOUBLE_EQ(loader.data().at(3), -1.0);

	const std::vector<double> expectedSorted = {-1.0, 1.0, 2.0, 3.0, 5.0};
	EXPECT_EQ(loader.sortedData(), expectedSorted);
	EXPECT_EQ(loader.count(), 5u);
	EXPECT_DOUBLE_EQ(loader.min(), -1.0);
	EXPECT_DOUBLE_EQ(loader.max(), 5.0);
	EXPECT_DOUBLE_EQ(loader.mean(), 2.0);
	EXPECT_DOUBLE_EQ(loader.variance(), 5.0);
	EXPECT_DOUBLE_EQ(loader.stddev(), std::sqrt(5.0));
	EXPECT_TRUE(loader.hasNegativeData());
}

TEST(DatasetLoaderTest, SupportsWhitespaceSeparatedText) {
	const std::filesystem::path path = writeTextDatasetFile("dataset_loader_whitespace", "4 1\n2\t3\n");
	DatasetLoader loader;

	ASSERT_TRUE(loader.loadFromFile(path.string(), ' '));

	const std::vector<double> expectedData = {4.0, 1.0, 2.0, 3.0};
	const std::vector<double> expectedSorted = {1.0, 2.0, 3.0, 4.0};
	EXPECT_EQ(loader.data(), expectedData);
	EXPECT_EQ(loader.sortedData(), expectedSorted);
	EXPECT_EQ(loader.count(), 4u);
	EXPECT_DOUBLE_EQ(loader.min(), 1.0);
	EXPECT_DOUBLE_EQ(loader.max(), 4.0);
	EXPECT_DOUBLE_EQ(loader.mean(), 2.5);
	EXPECT_DOUBLE_EQ(loader.variance(), 5.0 / 3.0);
	EXPECT_DOUBLE_EQ(loader.stddev(), std::sqrt(5.0 / 3.0));
	EXPECT_FALSE(loader.hasNegativeData());
}

TEST(DatasetLoaderTest, LoadsBinaryDoubleDataset) {
	const std::vector<double> values = {10.0, 4.0, 6.0};
	const std::filesystem::path path = writeBinaryDatasetFile("dataset_loader_binary", values);
	DatasetLoader loader;

	ASSERT_TRUE(loader.loadFromFile(path.string()));

	const std::vector<double> expectedSorted = {4.0, 6.0, 10.0};
	EXPECT_EQ(loader.data(), values);
	EXPECT_EQ(loader.sortedData(), expectedSorted);
	EXPECT_EQ(loader.count(), 3u);
	EXPECT_DOUBLE_EQ(loader.min(), 4.0);
	EXPECT_DOUBLE_EQ(loader.max(), 10.0);
	EXPECT_DOUBLE_EQ(loader.mean(), 20.0 / 3.0);
	EXPECT_DOUBLE_EQ(loader.variance(), 28.0 / 3.0);
	EXPECT_DOUBLE_EQ(loader.stddev(), std::sqrt(28.0 / 3.0));
	EXPECT_FALSE(loader.hasNegativeData());
}

TEST(DatasetLoaderTest, SingleObservationIsUsableWithZeroVariance) {
	const std::filesystem::path path = writeTextDatasetFile("dataset_loader_single", "7.5\n");
	DatasetLoader loader;

	ASSERT_TRUE(loader.loadFromFile(path.string()));

	EXPECT_TRUE(loader.isLoaded());
	EXPECT_TRUE(loader.isUsable());
	EXPECT_EQ(loader.count(), 1u);
	EXPECT_DOUBLE_EQ(loader.min(), 7.5);
	EXPECT_DOUBLE_EQ(loader.max(), 7.5);
	EXPECT_DOUBLE_EQ(loader.mean(), 7.5);
	EXPECT_DOUBLE_EQ(loader.variance(), 0.0);
	EXPECT_DOUBLE_EQ(loader.stddev(), 0.0);
	EXPECT_FALSE(loader.hasNegativeData());
}

TEST(DatasetLoaderTest, RejectsInvalidOrEmptyTextAndKeepsEmptyState) {
	const std::filesystem::path invalidPath = writeTextDatasetFile("dataset_loader_invalid", "1.0,not-number\n");
	const std::filesystem::path emptyPath = writeTextDatasetFile("dataset_loader_empty", "\n  \n# comment only\n");

	DatasetLoader loader;

	EXPECT_FALSE(loader.loadFromFile(invalidPath.string()));
	EXPECT_FALSE(loader.isLoaded());
	EXPECT_FALSE(loader.isUsable());
	EXPECT_TRUE(loader.data().empty());
	EXPECT_TRUE(loader.sortedData().empty());

	EXPECT_FALSE(loader.loadFromFile(emptyPath.string()));
	EXPECT_FALSE(loader.isLoaded());
	EXPECT_FALSE(loader.isUsable());
	EXPECT_TRUE(loader.data().empty());
	EXPECT_TRUE(loader.sortedData().empty());
}

TEST(DatasetLoaderTest, RejectsMissingFile) {
	DatasetLoader loader;

	EXPECT_FALSE(loader.loadFromFile((std::filesystem::temp_directory_path() / "missing_dataset_loader_file.bin").string()));
	EXPECT_FALSE(loader.isLoaded());
	EXPECT_FALSE(loader.isUsable());
	EXPECT_TRUE(loader.data().empty());
	EXPECT_TRUE(loader.sortedData().empty());
}
