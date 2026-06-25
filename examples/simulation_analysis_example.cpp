#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/components/InputOutput/Record.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "tools/analysis/DataAnalyserDefaultImpl.h"
#include "tools/analysis/SimulationResultsDataset.h"

#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

// Converts a hypothesis-test decision into readable output text.
std::string decision(bool rejectH0) {
	return rejectH0 ? "reject H0" : "do not reject H0";
}

// Evaluates the normal CDF used by the KS diagnostic in this example.
double normalCdf(double mean, double stddev, double value) {
	const double z = (value - mean) / (stddev * std::sqrt(2.0));
	return 0.5 * (1.0 + std::erf(z));
}

// Prints a section title for the example output.
void printSection(const std::string& title) {
	std::cout << "\n== " << title << " ==\n";
}

// Prints the minimum descriptive summary needed to inspect the simulation output.
void printSummary(const DataSetSummary& summary) {
	std::cout << std::fixed << std::setprecision(4)
	          << "  samples : " << summary.count << "\n"
	          << "  min/max : " << summary.min << " / " << summary.max << "\n"
	          << "  mean    : " << summary.mean << "\n"
	          << "  stddev  : " << summary.stddev << "\n";
}

// Turns runtime checks into exceptions so main can report a single failure path.
void ensure(bool condition, const std::string& message) {
	if (!condition) {
		throw std::runtime_error(message);
	}
}

// Selects a temporary output file for the Record-generated simulation dataset.
std::filesystem::path simulationOutputFile() {
	return std::filesystem::temp_directory_path() / "genesys_dcs_simulation_analysis_record.txt";
}

// Builds and runs the GenESyS model that produces the Record dataset.
void buildAndRunInspectionStationModel(const std::filesystem::path& outputFile) {
	// Modeling objective:
	// This model represents a lightweight inspection sampling process. Parts
	// arrive according to a stochastic interarrival-time expression and then
	// leave the system. At each arriving part, a Record module writes a simulated
	// inspection measurement to a GenESyS result file. The data-analysis facade
	// then consumes that simulation output as an in-memory dataset.
	Simulator simulator;
	simulator.getTraceManager()->setTraceLevel(TraceManager::Level::L0_noTraces);
	simulator.getPluginManager()->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ensure(model != nullptr, "Could not create GenESyS model.");

	auto* partType = new EntityType(model, "Part");
	auto* arrivals = new Create(model, "PartArrivals");
	auto* record = new Record(model, "RecordInspectionMeasurement");
	auto* dispose = new Dispose(model, "DisposeInspectedPart");

	ensure(partType != nullptr
	       && arrivals != nullptr
	       && record != nullptr
	       && dispose != nullptr,
	       "Could not create one or more model components.");

	arrivals->setEntityType(partType);
	arrivals->setFirstCreation(0.0);
	arrivals->setTimeBetweenCreationsExpression("expo(5)");
	arrivals->setTimeUnit(Util::TimeUnit::second);
	arrivals->setEntitiesPerCreation(1);
	arrivals->setMaxCreations(60);

	record->setFilename(outputFile.string());
	record->setExpression("norm(50,9.83)");
	record->setExpressionName("Inspection measurement");
	record->setDatasetName("Inspection sampling output");
	record->setRandomVariableName("InspectionMeasurement");
	record->setVariableType("Continuous numeric");
	record->setDatasetDescription("Quality measurement recorded when each simulated part leaves inspection.");
	record->setTimeDependent(false);

	arrivals->connectTo(record);
	record->connectTo(dispose);

	ModelSimulation* simulation = model->getSimulation();
	simulation->setReplicationLength(500.0, Util::TimeUnit::second);
	simulation->setNumberOfReplications(3);
	simulation->start();
}

// Parses the Record output into the analysis package's simulation dataset object.
SimulationResultsDataset loadSimulationResults(const std::filesystem::path& outputFile) {
	SimulationResultsDataset dataset;
	std::string errorMessage;
	ensure(SimulationResultsParser::loadFromTextFile(outputFile.string(), &dataset, &errorMessage),
	       "Could not parse simulation output: " + errorMessage);
	ensure(dataset.hasNumericData(), "Simulation output did not contain numeric observations.");
	return dataset;
}

// Runs descriptive, fitting and inference operations over simulation results.
void analyseSimulationOutput(const SimulationResultsDataset& dataset) {
	DataAnalyserDefaultImpl analyser;
	ensure(analyser.loadDataSet(dataset.data()), "Could not load simulation output into DataAnalyser.");

	printSection("Simulation output metadata");
	std::cout << "  dataset        : " << dataset.datasetName << "\n"
	          << "  random variable: " << dataset.randomVariableName << "\n"
	          << "  format         : " << dataset.formatKindName() << "\n"
	          << "  replications   : " << dataset.replications().size() << "\n";

	printSection("Descriptive summary");
	const DataSetSummary summary = analyser.summary();
	printSummary(summary);

	printSection("Distribution fitting");
	const FitSummary fitSummary = analyser.fitter()->fitAllSummary();
	ensure(fitSummary.success, "Could not fit simulation output.");
	std::cout << "  best fit       : " << fitSummary.bestFit.distributionName << "\n"
	          << "  squared error  : " << std::fixed << std::setprecision(6)
	          << fitSummary.bestFit.squaredError << "\n";

	printSection("Inference over simulation output");
	const auto meanCi = analyser.tester()->averageConfidenceInterval(
	    summary.mean,
	    summary.stddev,
	    static_cast<unsigned int>(summary.count),
	    0.95
	);
	std::cout << std::fixed << std::setprecision(4)
	          << "  95% mean CI    : [" << meanCi.inferiorLimit()
	          << ", " << meanCi.superiorLimit() << "]\n";

	const auto meanTest = analyser.tester()->testAverage(
	    summary.mean,
	    summary.stddev,
	    static_cast<unsigned int>(summary.count),
	    50.0,
	    0.95,
	    HypothesisTester_if::H1Comparition::DIFFERENT
	);
	std::cout << "  mean test H0=50: stat=" << meanTest.testStat()
	          << " p=" << meanTest.pValue()
	          << " decision=" << decision(meanTest.rejectH0()) << "\n";

	const double fittedMean = summary.mean;
	const double fittedStddev = summary.stddev;
	const auto fittedNormalCdf = [fittedMean, fittedStddev](double value) {
		return normalCdf(fittedMean, fittedStddev, value);
	};

	const auto ks = analyser.tester()->kolmogorovSmirnov(analyser.data(), fittedNormalCdf, 0.95);
	std::cout << "  KS diagnostic  : stat=" << ks.testStat()
	          << " p=" << ks.pValue()
	          << " decision=" << decision(ks.rejectH0()) << "\n";
}

} // namespace

int main() {
	try {
		// Prepare a clean temporary Record file before running the simulation.
		const std::filesystem::path outputFile = simulationOutputFile();
		std::filesystem::remove(outputFile);

		// Run the model, parse its output, and analyze the generated observations.
		buildAndRunInspectionStationModel(outputFile);
		const SimulationResultsDataset dataset = loadSimulationResults(outputFile);
		analyseSimulationOutput(dataset);

		// Remove the temporary dataset after a successful run.
		std::filesystem::remove(outputFile);
		std::cout << "\nSimulation analysis example: SUCCESS\n";
		return 0;
	}
	catch (const std::exception& exception) {
		std::cerr << "Simulation analysis example failed: " << exception.what() << "\n";
		return 1;
	}
}
