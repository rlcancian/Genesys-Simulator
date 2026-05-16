#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "tools/SimulationResultsDataset.h"

namespace {
	std::filesystem::path writeDatasetFile(const std::string& name, const std::string& contents) {
		const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
		std::filesystem::path path = std::filesystem::temp_directory_path() / (name + "_" + std::to_string(suffix) + ".txt");
		std::ofstream file(path);
		file << contents;
		return path;
	}
}

TEST(SimulationResultsDatasetParserTest, LoadsRawNumericDataset) {
	const std::filesystem::path path = writeDatasetFile("raw_numeric", "2.1\n\n4.0\n3.8\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	ASSERT_TRUE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage)) << errorMessage;
	EXPECT_EQ(dataset.formatKind, SimulationResultsDatasetFormat::RawNumeric);
	EXPECT_EQ(dataset.variableType, "Continuous numeric");
	EXPECT_FALSE(dataset.recordFile);
	EXPECT_FALSE(dataset.timeDependent);
	ASSERT_EQ(dataset.observations.size(), 3u);
	EXPECT_DOUBLE_EQ(dataset.observations.at(0).value, 2.1);
	EXPECT_EQ(dataset.observations.at(0).replication, 1u);
	EXPECT_FALSE(dataset.observations.at(0).hasTime);
}

TEST(SimulationResultsDatasetParserTest, RejectsEmptyRawNumericDataset) {
	const std::filesystem::path path = writeDatasetFile("raw_empty", "\n  \n\t\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	EXPECT_FALSE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage));
	EXPECT_NE(errorMessage.find("does not contain observations"), std::string::npos);
}

TEST(SimulationResultsDatasetParserTest, RejectsInvalidRawNumericDataset) {
	const std::filesystem::path path = writeDatasetFile("raw_invalid", "2.1\nnot numeric\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	EXPECT_FALSE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage));
	EXPECT_NE(errorMessage.find("Invalid raw numeric observation"), std::string::npos);
}

TEST(SimulationResultsDatasetParserTest, LoadsLegacyRecordDatasetWithTime) {
	const std::filesystem::path path = writeDatasetFile(
			"record_legacy",
			"#Expression=\"Entity.WaitingTime\", ExpressionName=\"WaitingTime\"\n"
			"#TimeDependent=true\n"
			"#ReplicationNumber=1\n"
			"0.5 3.2\n"
			"1.0 4.1\n"
			"#ReplicationNumber=2\n"
			"0.5 3.0\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	ASSERT_TRUE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage)) << errorMessage;
	EXPECT_EQ(dataset.formatKind, SimulationResultsDatasetFormat::RecordLegacy);
	EXPECT_TRUE(dataset.recordFile);
	EXPECT_EQ(dataset.datasetName, "WaitingTime");
	EXPECT_EQ(dataset.randomVariableName, "WaitingTime");
	EXPECT_EQ(dataset.expressionName, "WaitingTime");
	EXPECT_TRUE(dataset.timeDependent);
	ASSERT_EQ(dataset.observations.size(), 3u);
	EXPECT_EQ(dataset.observations.back().replication, 2u);
	EXPECT_DOUBLE_EQ(dataset.observations.front().time, 0.5);
}

TEST(SimulationResultsDatasetParserTest, LoadsLegacyRecordDatasetWithoutTime) {
	const std::filesystem::path path = writeDatasetFile(
			"record_legacy_without_time",
			"#Expression=\"NQ(Queue_1)\", ExpressionName=\"QueueLength\"\n"
			"#ReplicationNumber=1\n"
			"4\n"
			"7\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	ASSERT_TRUE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage)) << errorMessage;
	EXPECT_EQ(dataset.formatKind, SimulationResultsDatasetFormat::RecordLegacy);
	EXPECT_TRUE(dataset.recordFile);
	EXPECT_EQ(dataset.datasetName, "QueueLength");
	EXPECT_EQ(dataset.randomVariableName, "QueueLength");
	EXPECT_FALSE(dataset.timeDependent);
	ASSERT_EQ(dataset.observations.size(), 2u);
	EXPECT_FALSE(dataset.observations.front().hasTime);
	EXPECT_DOUBLE_EQ(dataset.observations.back().value, 7.0);
}

TEST(SimulationResultsDatasetParserTest, LoadsEnrichedRecordDatasetWithoutTime) {
	const std::filesystem::path path = writeDatasetFile(
			"record_enriched",
			"#Format=\"GenesysRecordDataset\"\n"
			"#FormatVersion=\"1\"\n"
			"#DatasetName=\"Queue Length Dataset\"\n"
			"#RandomVariableName=\"QueueLength\"\n"
			"#VariableType=\"Discrete numeric\"\n"
			"#Description=\"Queue length observations.\"\n"
			"#Source=\"Genesys Record\"\n"
			"#Expression=\"NQ(Queue_1)\"\n"
			"#ExpressionName=\"QueueLength\"\n"
			"#TimeDependent=false\n"
			"#Columns=\"value\"\n"
			"#ReplicationNumber=1\n"
			"4\n7\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	ASSERT_TRUE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage)) << errorMessage;
	EXPECT_EQ(dataset.formatKind, SimulationResultsDatasetFormat::RecordEnriched);
	EXPECT_EQ(dataset.formatVersion, "1");
	EXPECT_EQ(dataset.datasetName, "Queue Length Dataset");
	EXPECT_EQ(dataset.randomVariableName, "QueueLength");
	EXPECT_EQ(dataset.variableType, "Discrete numeric");
	EXPECT_EQ(dataset.source, "Genesys Record");
	EXPECT_FALSE(dataset.timeDependent);
	ASSERT_EQ(dataset.observations.size(), 2u);
	EXPECT_DOUBLE_EQ(dataset.observations.at(1).value, 7.0);
}

TEST(SimulationResultsDatasetParserTest, LoadsEnrichedRecordDatasetWithTimeAndNormalizesVariableType) {
	const std::filesystem::path path = writeDatasetFile(
			"record_enriched_with_time",
			"#Format=\"GenesysRecordDataset\"\n"
			"#FormatVersion=\"1\"\n"
			"#DatasetName=\"Queue Waiting Time Dataset\"\n"
			"#RandomVariableName=\"WaitingTime\"\n"
			"#VariableType=\"continuous NUMERIC\"\n"
			"#Description=\"Waiting-time observations.\"\n"
			"#Source=\"Genesys Record\"\n"
			"#Expression=\"Entity.WaitingTime\"\n"
			"#ExpressionName=\"WaitingTime\"\n"
			"#Columns=\"time value\"\n"
			"#ReplicationNumber=1\n"
			"0.5 3.2\n"
			"#ReplicationNumber=2\n"
			"0.5 3.0\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	ASSERT_TRUE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage)) << errorMessage;
	EXPECT_EQ(dataset.formatKind, SimulationResultsDatasetFormat::RecordEnriched);
	EXPECT_EQ(dataset.variableType, "Continuous numeric");
	EXPECT_TRUE(dataset.timeDependent);
	EXPECT_EQ(dataset.replications().size(), 2u);
	ASSERT_EQ(dataset.observations.size(), 2u);
	EXPECT_TRUE(dataset.observations.front().hasTime);
	EXPECT_DOUBLE_EQ(dataset.observations.front().time, 0.5);
}

TEST(SimulationResultsDatasetParserTest, LoadsGuiTabularDatasetBySemanticHeader) {
	const std::filesystem::path path = writeDatasetFile(
			"gui_tabular",
			"dataset,variable,replication,time,value,ignored\n"
			"\"Queue Waiting Time Dataset\",\"WaitingTime\",1,0.5,3.2,x\n"
			"\"Queue Waiting Time Dataset\",\"WaitingTime\",2,,4.1,y\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	ASSERT_TRUE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage)) << errorMessage;
	EXPECT_EQ(dataset.formatKind, SimulationResultsDatasetFormat::GuiTabular);
	EXPECT_EQ(dataset.datasetName, "Queue Waiting Time Dataset");
	EXPECT_EQ(dataset.randomVariableName, "WaitingTime");
	EXPECT_TRUE(dataset.timeDependent);
	ASSERT_EQ(dataset.observations.size(), 2u);
	EXPECT_TRUE(dataset.observations.at(0).hasTime);
	EXPECT_FALSE(dataset.observations.at(1).hasTime);
	EXPECT_EQ(dataset.observations.at(1).replication, 2u);
}

TEST(SimulationResultsDatasetParserTest, LoadsGuiTabularDatasetWithoutTime) {
	const std::filesystem::path path = writeDatasetFile(
			"gui_tabular_without_time",
			"dataset,variable,replication,time,value\n"
			"\"Queue Length Dataset\",\"QueueLength\",1,,4\n"
			"\"Queue Length Dataset\",\"QueueLength\",1,,7\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	ASSERT_TRUE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage)) << errorMessage;
	EXPECT_EQ(dataset.formatKind, SimulationResultsDatasetFormat::GuiTabular);
	EXPECT_EQ(dataset.datasetName, "Queue Length Dataset");
	EXPECT_EQ(dataset.randomVariableName, "QueueLength");
	EXPECT_FALSE(dataset.timeDependent);
	ASSERT_EQ(dataset.observations.size(), 2u);
	EXPECT_FALSE(dataset.observations.front().hasTime);
}

TEST(SimulationResultsDatasetParserTest, RejectsGuiTabularDatasetWithoutValueColumn) {
	const std::filesystem::path path = writeDatasetFile("gui_no_value", "dataset,variable,replication,time\nA,X,1,0.5\n");
	SimulationResultsDataset dataset;
	std::string errorMessage;

	EXPECT_FALSE(SimulationResultsDatasetParser::loadFromTextFile(path.string(), &dataset, &errorMessage));
	EXPECT_FALSE(errorMessage.empty());
}
