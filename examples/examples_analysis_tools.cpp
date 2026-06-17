#include "tools/analysis/DataAnalyserDefaultImpl.h"
#include "tools/analysis/DatasetLoader.h"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

bool isGreaterThan50(double value) {
    return value > 50.0;
}

std::string decision(bool rejectH0) {
    return rejectH0 ? "reject H0" : "do not reject H0";
}

void printHeader(const std::string& dataFile, std::size_t n, double mean, double stddev, double min, double max) {
    std::cout << "====================================================\n"
              << "  Genesys Simulator -- Analysis Tools Example\n"
              << "====================================================\n"
              << "Data file  : " << dataFile << "\n"
              << "Samples    : " << n << "\n"
              << std::fixed << std::setprecision(4)
              << "Mean       : " << mean << "\n"
              << "Std dev    : " << stddev << "\n"
              << "Min        : " << min << "   Max: " << max << "\n"
              << "\n";
}

void printSection(const std::string& title) {
    std::cout << "\n"
              << "== " << title << " ==\n";
}

constexpr int COL_NAME  = 14;
constexpr int COL_PARAM = 18;
constexpr int COL_SSE   = 12;

void printTableHeader() {
    std::cout << std::left
              << std::setw(COL_NAME)  << "Distribution"
              << std::setw(COL_PARAM) << "Param1"
              << std::setw(COL_PARAM) << "Param2"
              << std::setw(COL_PARAM) << "Param3"
              << std::setw(COL_PARAM) << "Param4"
              << "SSE"
              << "\n"
              << std::string(COL_NAME + 4 * COL_PARAM + COL_SSE, '-') << "\n";
}

void printRow(const std::string& name,
              const std::string& p1label, double p1,
              const std::string& p2label, double p2,
              const std::string& p3label, double p3,
              const std::string& p4label, double p4,
              double sse) {
    auto fmtParam = [](const std::string& label, double v) -> std::string {
        if (label.empty()) {
            return "---";
        }
        if (!std::isfinite(v)) {
            return "N/A";
        }
        std::ostringstream os;
        os << std::fixed << std::setprecision(4) << label << "=" << v;
        return os.str();
    };

    std::cout << std::left
              << std::setw(COL_NAME)  << name
              << std::setw(COL_PARAM) << fmtParam(p1label, p1)
              << std::setw(COL_PARAM) << fmtParam(p2label, p2)
              << std::setw(COL_PARAM) << fmtParam(p3label, p3)
              << std::setw(COL_PARAM) << fmtParam(p4label, p4);
    if (std::isfinite(sse)) {
        std::cout << std::fixed << std::setprecision(6) << sse;
    } else {
        std::cout << "N/A";
    }
    std::cout << "\n";
}

void printConfidenceInterval(const std::string& name, HypothesisTester_if::ConfidenceInterval interval) {
    std::cout << std::left << std::setw(28) << name
              << " [" << std::fixed << std::setprecision(4) << interval.inferiorLimit()
              << ", " << interval.superiorLimit()
              << "]  half-width=" << interval.halfWidth() << "\n";
}

void printTestResult(const std::string& name, HypothesisTester_if::TestResult result) {
    std::cout << std::left << std::setw(28) << name
              << " stat=" << std::fixed << std::setprecision(4) << result.testStat()
              << "  p=" << result.pValue()
              << "  decision=" << decision(result.rejectH0()) << "\n";
}

std::vector<double> normalQuantileObservedFrequencies(const std::vector<double>& sortedData, double mean, double stddev) {
    const std::vector<double> zBreaks = {-0.841621, -0.253347, 0.253347, 0.841621};
    std::vector<double> observed(5, 0.0);
    for (double value : sortedData) {
        std::size_t bucket = 0;
        while (bucket < zBreaks.size() && value > mean + zBreaks[bucket] * stddev) {
            ++bucket;
        }
        observed[bucket] += 1.0;
    }
    return observed;
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string dataFile = (argc > 1) ? argv[1] : "examples/data/sample_data.csv";

    DatasetLoader dataset;
    if (!dataset.loadFromFile(dataFile, ',') && !dataset.loadFromFile(dataFile, ' ')) {
        std::cerr << "ERROR: Could not load data from '" << dataFile << "'.\n"
                  << "       Run from the project root or pass a data file path as argument.\n";
        return 1;
    }

    DataAnalyserDefaultImpl analyser;

    if (!analyser.loadDataSet(dataFile)) {
        std::cerr << "ERROR: Could not configure DataAnalyser with '" << dataFile << "'.\n";
        return 1;
    }
    const DataSetSummary summary = analyser.summary();
    const DataSetHistogram histogram = analyser.histogram(6);
    const DataSetBoxPlot boxplot = analyser.boxplot();

    // Verify that data was loaded by probing the Normal fit result.
    {
        double sse = 0.0, avg = 0.0, sd = 0.0;
        analyser.fitter()->fitNormal(&sse, &avg, &sd);
        if (!std::isfinite(avg)) {
            std::cerr << "ERROR: Could not load data from '" << dataFile << "'.\n"
                      << "       Run from the project root or pass a data file path as argument.\n";
            return 1;
        }
    }

    Fitter_if* f = analyser.fitter();

    // --- Normal fit ---
    double sse, avg, sd;
    f->fitNormal(&sse, &avg, &sd);
    const double normalSse = sse;

    double uMin, uMax, uSse;
    f->fitUniform(&uSse, &uMin, &uMax);

    printHeader(dataFile, summary.count, summary.mean, summary.stddev, summary.min, summary.max);
    std::cout << "Median     : " << boxplot.median
              << "   Q1: " << boxplot.firstQuartile
              << "   Q3: " << boxplot.thirdQuartile
              << "\n"
              << "Hist bins  : " << histogram.bins.size()
              << "   Class width: " << histogram.classWidth
              << "\n";

    // --- Individual fits ---
    printSection("Distribution fitting");
    printTableHeader();

    // Uniform
    printRow("Uniform", "min", uMin, "max", uMax, "", 0.0, "", 0.0, uSse);

    // Triangular
    double triMin, triMo, triMax;
    f->fitTriangular(&sse, &triMin, &triMo, &triMax);
    printRow("Triangular", "min", triMin, "mo", triMo, "max", triMax, "", 0.0, sse);

    // Normal
    printRow("Normal", "avg", avg, "stddev", sd, "", 0.0, "", 0.0, normalSse);

    // Exponential
    double expAvg;
    f->fitExpo(&sse, &expAvg);
    printRow("Exponential", "avg", expAvg, "", 0.0, "", 0.0, "", 0.0, sse);

    // Erlang
    double erlAvg, erlM;
    f->fitErlang(&sse, &erlAvg, &erlM);
    printRow("Erlang", "avg", erlAvg, "m", erlM, "", 0.0, "", 0.0, sse);

    // Beta
    double betaA, betaB, betaInf, betaSup;
    f->fitBeta(&sse, &betaA, &betaB, &betaInf, &betaSup);
    printRow("Beta", "alpha", betaA, "beta", betaB, "inf", betaInf, "sup", betaSup, sse);

    // Weibull
    double wAlpha, wScale;
    f->fitWeibull(&sse, &wAlpha, &wScale);
    printRow("Weibull", "alpha", wAlpha, "scale", wScale, "", 0.0, "", 0.0, sse);

	// --- Best fit ---
	const FitSummary fitSummary = f->fitAllSummary();

	std::cout << "\n"
	          << "=> Best fit : " << fitSummary.bestFit.distributionName
	          << "  (SSE = " << std::fixed << std::setprecision(6) << fitSummary.bestFit.squaredError
	          << ", ranked candidates = " << fitSummary.ranking.size() << ")\n";

    // --- Normality check ---
    const bool isNormal95 = f->isNormalDistributed(0.05);
    const bool isNormal99 = f->isNormalDistributed(0.01);
    std::cout << "=> Is normally distributed (alpha=0.05): " << (isNormal95 ? "YES" : "NO") << "\n"
              << "=> Is normally distributed (alpha=0.01): " << (isNormal99 ? "YES" : "NO") << "\n";

    // --- Parametric inference examples ---
    HypothesisTester_if* h = analyser.tester();
    const double confidenceLevel = 0.95;
    const double meanH0 = 50.0;
    const double varianceH0 = 100.0;
    const double proportionH0 = 0.5;
    const double sampleProportionGreaterThan50 = static_cast<double>(
            std::count_if(dataset.data().begin(), dataset.data().end(), isGreaterThan50)
    ) / static_cast<double>(dataset.count());

    printSection("Confidence intervals");
    printConfidenceInterval("Average mean", h->averageConfidenceInterval(dataset.mean(), dataset.stddev(), static_cast<unsigned int>(dataset.count()), confidenceLevel));
    printConfidenceInterval("Average mean (file)", h->averageConfidenceInterval(dataFile, confidenceLevel));
    printConfidenceInterval("Variance", h->varianceConfidenceInterval(dataset.variance(), static_cast<unsigned int>(dataset.count()), confidenceLevel));
    printConfidenceInterval("P(value > 50)", h->proportionConfidenceInterval(sampleProportionGreaterThan50, static_cast<unsigned int>(dataset.count()), confidenceLevel));
    printConfidenceInterval("P(value > 50) (file)", h->proportionConfidenceInterval(dataFile, isGreaterThan50, confidenceLevel));

    printSection("Hypothesis tests");
    printTestResult("Mean == 50", h->testAverage(meanH0, dataset.stddev(), static_cast<unsigned int>(dataset.count()), dataset.mean(), confidenceLevel, HypothesisTester_if::DIFFERENT));
    printTestResult("Mean == 50 (file)", h->testAverage(dataFile, meanH0, confidenceLevel, HypothesisTester_if::DIFFERENT));
    printTestResult("Variance == 100", h->testVariance(dataset.variance(), static_cast<unsigned int>(dataset.count()), varianceH0, confidenceLevel, HypothesisTester_if::DIFFERENT));
    printTestResult("P(value > 50) == 0.5", h->testProportion(proportionH0, static_cast<unsigned int>(dataset.count()), sampleProportionGreaterThan50, confidenceLevel, HypothesisTester_if::DIFFERENT));
    printTestResult("P(value > 50) == 0.5 (file)", h->testProportion(dataFile, isGreaterThan50, proportionH0, confidenceLevel, HypothesisTester_if::DIFFERENT));

    const auto& sorted = dataset.sortedData();
    const std::size_t half = sorted.size() / 2;
    DatasetLoader lowerHalf;
    DatasetLoader upperHalf;
    lowerHalf.loadFromVector(std::vector<double>(sorted.begin(), sorted.begin() + static_cast<std::ptrdiff_t>(half)));
    upperHalf.loadFromVector(std::vector<double>(sorted.begin() + static_cast<std::ptrdiff_t>(half), sorted.end()));

    printSection("Two-sample examples");
    printConfidenceInterval("Mean lower-upper", h->averageDifferenceConfidenceInterval(
            lowerHalf.mean(), lowerHalf.stddev(), static_cast<unsigned int>(lowerHalf.count()),
            upperHalf.mean(), upperHalf.stddev(), static_cast<unsigned int>(upperHalf.count()),
            confidenceLevel));
    printTestResult("Lower mean == upper mean", h->testAverage(
            lowerHalf.mean(), lowerHalf.stddev(), static_cast<unsigned int>(lowerHalf.count()),
            upperHalf.mean(), upperHalf.stddev(), static_cast<unsigned int>(upperHalf.count()),
            confidenceLevel, HypothesisTester_if::DIFFERENT));
    printTestResult("Lower variance == upper variance", h->testVariance(
            lowerHalf.variance(), static_cast<unsigned int>(lowerHalf.count()),
            upperHalf.variance(), static_cast<unsigned int>(upperHalf.count()),
            confidenceLevel, HypothesisTester_if::DIFFERENT));

    printSection("Goodness-of-fit tests");
    const std::vector<double> observedNormalQuantiles = normalQuantileObservedFrequencies(dataset.sortedData(), avg, sd);
    const std::vector<double> expectedNormalQuantiles(5, static_cast<double>(dataset.count()) / 5.0);
    printTestResult("Chi-square normal fit", h->chiSquareGoodnessOfFit(
            observedNormalQuantiles,
            expectedNormalQuantiles,
            2,
            confidenceLevel));

    auto fittedNormalCdf = [avg, sd](double value) {
        return 0.5 * std::erfc(-(value - avg) / (sd * std::sqrt(2.0)));
    };
    printTestResult("KS normal fit", h->kolmogorovSmirnov(dataset.data(), fittedNormalCdf, confidenceLevel));
    printTestResult("KS normal fit (file)", h->kolmogorovSmirnov(dataFile, fittedNormalCdf, confidenceLevel));

    return 0;
}
