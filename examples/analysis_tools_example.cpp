#include "tools/analysis/DataAnalyserDefaultImpl.h"
#include "tools/analysis/DatasetLoader.h"
#include "tools/analysis/ProbabilityDistribution.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int COL_NAME = 14;
constexpr int COL_PARAM = 18;
constexpr int COL_SSE = 12;
constexpr double EPSILON = 1.0e-9;

enum class IntervalPrintMode {
    HalfWidth,
    IntervalWidth
};

// Classifies sample values for proportion examples and tests.
bool isGreaterThan50(double value) {
    return value > 50.0;
}

// Converts a boolean hypothesis decision into readable output text.
std::string decision(bool rejectH0) {
    return rejectH0 ? "reject H0" : "do not reject H0";
}

// Formats p-values consistently for compact console tables.
std::string pValueText(double pValue) {
    if (!std::isfinite(pValue)) {
        return "N/A";
    }
    if (pValue < 0.0001) {
        return "<0.0001";
    }

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(4) << pValue;
    return stream.str();
}

// Normalizes distribution names for case-insensitive regression checks.
std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

// Prints a section title for the example output.
void printSection(const std::string& title) {
    std::cout << "\n== " << title << " ==\n";
}

// Prints descriptive statistics, boxplot summary and histogram metadata.
void printSummary(const std::string& label,
                  const std::string& filename,
                  const DataSetSummary& summary,
                  const DataSetBoxPlot& boxplot,
                  const DataSetHistogram& histogram) {
    std::cout << label << "\n"
              << "  File                 : " << filename << "\n"
              << "  Samples              : " << summary.count << "\n"
              << std::fixed << std::setprecision(4)
              << "  Mean                 : " << summary.mean << "\n"
              << "  Sample variance      : " << summary.variance << "\n"
              << "  Sample std. deviation: " << summary.stddev << "\n"
              << "  Minimum / maximum    : " << summary.min << " / " << summary.max << "\n"
              << "  Five-number summary  : " << summary.min
              << ", " << boxplot.firstQuartile
              << ", " << boxplot.median
              << ", " << boxplot.thirdQuartile
              << ", " << summary.max << "\n"
              << "  Histogram            : " << histogram.bins.size()
              << " classes, width=" << histogram.classWidth << "\n";
}

// Prints the distribution-fitting table header.
void printTableHeader() {
    std::cout << std::left
              << std::setw(COL_NAME) << "Distribution"
              << std::setw(COL_PARAM) << "Param1"
              << std::setw(COL_PARAM) << "Param2"
              << std::setw(COL_PARAM) << "Param3"
              << std::setw(COL_PARAM) << "Param4"
              << "SSE\n"
              << std::string(COL_NAME + 4 * COL_PARAM + COL_SSE, '-') << "\n";
}

// Prints one fitted-distribution row with up to four parameters.
void printRow(const std::string& name,
              const std::string& p1Label,
              double p1,
              const std::string& p2Label,
              double p2,
              const std::string& p3Label,
              double p3,
              const std::string& p4Label,
              double p4,
              double sse) {
    const auto formatParameter = [](const std::string& label, double value) {
        if (label.empty()) {
            return std::string("---");
        }
        if (!std::isfinite(value)) {
            return std::string("N/A");
        }

        std::ostringstream stream;
        stream << std::fixed << std::setprecision(4) << label << "=" << value;
        return stream.str();
    };

    std::cout << std::left
              << std::setw(COL_NAME) << name
              << std::setw(COL_PARAM) << formatParameter(p1Label, p1)
              << std::setw(COL_PARAM) << formatParameter(p2Label, p2)
              << std::setw(COL_PARAM) << formatParameter(p3Label, p3)
              << std::setw(COL_PARAM) << formatParameter(p4Label, p4);

    if (std::isfinite(sse)) {
        std::cout << std::fixed << std::setprecision(6) << sse;
    } else {
        std::cout << "N/A";
    }
    std::cout << "\n";
}

// Aligns labels used by confidence-interval and test result output.
void printFixedLabel(const std::string& name) {
    if (name.size() < 34U) {
        std::cout << std::left << std::setw(34) << name;
    } else {
        std::cout << name << "  ";
    }
}

// Prints a confidence interval and its displayed width metric.
void printConfidenceInterval(const std::string& name,
                             HypothesisTester_if::ConfidenceInterval interval,
                             IntervalPrintMode mode = IntervalPrintMode::HalfWidth) {
    printFixedLabel(name);
    std::cout << "[" << std::fixed << std::setprecision(4)
              << interval.inferiorLimit() << ", " << interval.superiorLimit() << "]";
    if (mode == IntervalPrintMode::HalfWidth) {
        std::cout << "  half-width=" << interval.halfWidth();
    } else {
        std::cout << "  interval-width="
                  << (interval.superiorLimit() - interval.inferiorLimit());
    }
    std::cout << "\n";
}

// Prints a hypothesis-test statistic, p-value and decision.
void printTestResult(const std::string& name,
                     const HypothesisTester_if::TestResult& result) {
    printFixedLabel(name);
    std::cout << "stat=" << std::fixed << std::setprecision(4) << result.testStat()
              << "  p=" << pValueText(result.pValue())
              << "  decision=" << decision(result.rejectH0()) << "\n";
}

// Prints chi-square results including grouped-class diagnostics.
void printChiSquareResult(const std::string& name,
                          const HypothesisTester_if::TestResult& result) {
    printTestResult(name, result);
    if (result.hasGoodnessOfFitDetails()) {
        const auto details = result.goodnessOfFitDetails();
        std::cout << "  initial classes     : " << details.initialClasses << "\n"
                  << "  effective classes   : " << details.effectiveClasses << "\n"
                  << "  estimated parameters: " << details.estimatedParameters << "\n"
                  << "  degrees of freedom  : " << details.degreesOfFreedom << "\n"
                  << "  observed / expected : " << std::fixed << std::setprecision(4)
                  << details.observedTotal << " / " << details.expectedTotal << "\n";
    }
}

// Loads a dataset through the facade and reports a readable error on failure.
bool load(DataAnalyserDefaultImpl& analyser, const std::string& filename) {
    if (analyser.loadDataSet(filename) && analyser.summary().count > 1U) {
        return true;
    }

    std::cerr << "ERROR: Could not load a usable dataset from '" << filename << "'.\n";
    return false;
}

// Compares two floating-point values with the example tolerance.
bool approximatelyEqual(double lhs, double rhs, double tolerance = EPSILON) {
    return std::abs(lhs - rhs) <= tolerance;
}

// Checks whether two facade summaries represent the same dataset.
bool summariesEquivalent(const DataSetSummary& lhs, const DataSetSummary& rhs) {
    return lhs.usable == rhs.usable
           && lhs.count == rhs.count
           && approximatelyEqual(lhs.min, rhs.min)
           && approximatelyEqual(lhs.max, rhs.max)
           && approximatelyEqual(lhs.mean, rhs.mean)
           && approximatelyEqual(lhs.variance, rhs.variance)
           && approximatelyEqual(lhs.stddev, rhs.stddev)
           && lhs.hasNegativeData == rhs.hasNegativeData;
}

// Checks whether two boxplot structures are equivalent.
bool boxplotsEquivalent(const DataSetBoxPlot& lhs, const DataSetBoxPlot& rhs) {
    return lhs.usable == rhs.usable
           && lhs.count == rhs.count
           && approximatelyEqual(lhs.min, rhs.min)
           && approximatelyEqual(lhs.firstQuartile, rhs.firstQuartile)
           && approximatelyEqual(lhs.median, rhs.median)
           && approximatelyEqual(lhs.thirdQuartile, rhs.thirdQuartile)
           && approximatelyEqual(lhs.max, rhs.max)
           && approximatelyEqual(lhs.interquartileRange, rhs.interquartileRange)
           && approximatelyEqual(lhs.lowerWhisker, rhs.lowerWhisker)
           && approximatelyEqual(lhs.upperWhisker, rhs.upperWhisker)
           && lhs.outliers == rhs.outliers;
}

// Checks whether two histogram structures are equivalent.
bool histogramsEquivalent(const DataSetHistogram& lhs, const DataSetHistogram& rhs) {
    if (lhs.usable != rhs.usable
        || lhs.count != rhs.count
        || !approximatelyEqual(lhs.min, rhs.min)
        || !approximatelyEqual(lhs.max, rhs.max)
        || !approximatelyEqual(lhs.classWidth, rhs.classWidth)
        || lhs.bins.size() != rhs.bins.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.bins.size(); ++i) {
        if (!approximatelyEqual(lhs.bins[i].lowerLimit, rhs.bins[i].lowerLimit)
            || !approximatelyEqual(lhs.bins[i].upperLimit, rhs.bins[i].upperLimit)
            || lhs.bins[i].frequency != rhs.bins[i].frequency
            || !approximatelyEqual(lhs.bins[i].relativeFrequency, rhs.bins[i].relativeFrequency)) {
            return false;
        }
    }
    return true;
}

std::size_t histogramFrequencySum(const DataSetHistogram& histogram) {
    std::size_t total = 0U;
    for (const auto& bin : histogram.bins) {
        total += bin.frequency;
    }
    return total;
}

// Verifies that histogram bins form an ordered partition of the data range.
bool histogramIsOrderedAndContiguous(const DataSetHistogram& histogram) {
    if (!histogram.usable || histogram.bins.empty()) {
        return false;
    }
    for (std::size_t i = 0; i < histogram.bins.size(); ++i) {
        if (histogram.bins[i].upperLimit < histogram.bins[i].lowerLimit) {
            return false;
        }
        if (i > 0U && !approximatelyEqual(histogram.bins[i - 1U].upperLimit, histogram.bins[i].lowerLimit)) {
            return false;
        }
    }
    return approximatelyEqual(histogram.bins.front().lowerLimit, histogram.min)
           && approximatelyEqual(histogram.bins.back().upperLimit, histogram.max);
}

// Verifies that the boxplot values follow the expected numeric order.
bool boxplotIsOrdered(const DataSetBoxPlot& boxplot) {
    return boxplot.usable
           && boxplot.min <= boxplot.firstQuartile
           && boxplot.firstQuartile <= boxplot.median
           && boxplot.median <= boxplot.thirdQuartile
           && boxplot.thirdQuartile <= boxplot.max
           && boxplot.lowerWhisker <= boxplot.upperWhisker;
}

// Describes which two-sample mean policy the tester will apply.
std::string selectedTwoMeanMethod(HypothesisTester_if* tester,
                                  const DataSetSummary& lhs,
                                  const DataSetSummary& rhs,
                                  double confidenceLevel) {
    const auto varianceRatioInterval = tester->varianceRatioConfidenceInterval(
            lhs.variance,
            static_cast<unsigned int>(lhs.count),
            rhs.variance,
            static_cast<unsigned int>(rhs.count),
            confidenceLevel);
    const bool pooled = varianceRatioInterval.inferiorLimit() <= 1.0
                        && varianceRatioInterval.superiorLimit() >= 1.0;
    if (pooled) {
        return "pooled t-test, equal variances selected by variance-ratio CI";
    }
    return "Welch t-test, unequal variances selected by variance-ratio CI";
}

// Prints a regression-check line and counts failures for the example exit code.
void regressionCheck(bool condition,
                     const std::string& description,
                     unsigned int& failures) {
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << description << "\n";
    if (!condition) {
        ++failures;
    }
}

} // namespace

int main(int argc, char* argv[]) {
    // Select the bundled datasets unless paths are passed explicitly.
    const std::string primaryFile =
            (argc > 1) ? argv[1] : "examples/data/sample_data.csv";
    const std::string groupAFile =
            (argc > 2) ? argv[2] : "examples/data/sample_group_a.csv";
    const std::string groupBFile =
            (argc > 3) ? argv[3] : "examples/data/sample_group_b.csv";

    std::cout << "============================================================\n"
              << "  Genesys Simulator -- Analysis Tools Complete Example\n"
              << "============================================================\n"
              << "The datasets are reproducible pseudo-random samples.\n"
              << "The two comparison groups were generated independently.\n";

    // Build one facade per dataset so each analysis service shares its dataset.
    DataAnalyserDefaultImpl primaryAnalyser;
    DataAnalyserDefaultImpl groupAAnalyser;
    DataAnalyserDefaultImpl groupBAnalyser;

    if (!load(primaryAnalyser, primaryFile)
        || !load(groupAAnalyser, groupAFile)
        || !load(groupBAnalyser, groupBFile)) {
        std::cerr << "Run from the project root or pass the three CSV paths explicitly.\n";
        return 1;
    }

    const DataSetSummary primary = primaryAnalyser.summary();
    const DataSetSummary groupA = groupAAnalyser.summary();
    const DataSetSummary groupB = groupBAnalyser.summary();

    const DataSetBoxPlot primaryBoxplot = primaryAnalyser.boxplot();
    const DataSetBoxPlot groupABoxplot = groupAAnalyser.boxplot();
    const DataSetBoxPlot groupBBoxplot = groupBAnalyser.boxplot();

    const DataSetHistogram primaryHistogram = primaryAnalyser.histogram(10);
    const DataSetHistogram groupAHistogram = groupAAnalyser.histogram(8);
    const DataSetHistogram groupBHistogram = groupBAnalyser.histogram(8);

    // Show the descriptive structures exposed by the facade.
    printSection("Descriptive statistics and exploratory structures");
    printSummary("Primary sample", primaryFile, primary, primaryBoxplot, primaryHistogram);
    std::cout << "\n";
    printSummary("Independent group A", groupAFile, groupA, groupABoxplot, groupAHistogram);
    std::cout << "\n";
    printSummary("Independent group B", groupBFile, groupB, groupBBoxplot, groupBHistogram);

    // Demonstrate ingestion from an in-memory vector through the same facade.
    DataAnalyserDefaultImpl memoryAnalyser;
    if (!memoryAnalyser.loadDataSet(primaryAnalyser.data())) {
        std::cerr << "ERROR: Could not load the in-memory copy of the primary dataset.\n";
        return 1;
    }
    const DataSetSummary memorySummary = memoryAnalyser.summary();
    const DataSetBoxPlot memoryBoxplot = memoryAnalyser.boxplot();
    const DataSetHistogram memoryHistogram = memoryAnalyser.histogram(10);
    std::cout << "\nMemory copy verification\n"
              << "  Samples: " << memorySummary.count
              << "  Mean: " << std::fixed << std::setprecision(4) << memorySummary.mean
              << "  Std. deviation: " << memorySummary.stddev << "\n";

    Fitter_if* fitter = primaryAnalyser.fitter();

    // Demonstrate every distribution fit and the structured ranking.
    printSection("Distribution fitting using EDF/CDF and Hazen positions");
    printTableHeader();

    double sse = std::numeric_limits<double>::quiet_NaN();

    double uniformMin = 0.0;
    double uniformMax = 0.0;
    fitter->fitUniform(&sse, &uniformMin, &uniformMax);
    printRow("Uniform", "min", uniformMin, "max", uniformMax,
             "", 0.0, "", 0.0, sse);

    double triangularMin = 0.0;
    double triangularMode = 0.0;
    double triangularMax = 0.0;
    fitter->fitTriangular(&sse, &triangularMin, &triangularMode, &triangularMax);
    printRow("Triangular", "min", triangularMin, "mode", triangularMode,
             "max", triangularMax, "", 0.0, sse);

    double normalMean = 0.0;
    double normalStddev = 0.0;
    fitter->fitNormal(&sse, &normalMean, &normalStddev);
    const double normalSse = sse;
    printRow("Normal", "mean", normalMean, "stddev", normalStddev,
             "", 0.0, "", 0.0, normalSse);

    double exponentialMean = 0.0;
    fitter->fitExpo(&sse, &exponentialMean);
    printRow("Exponential", "mean", exponentialMean, "", 0.0,
             "", 0.0, "", 0.0, sse);

    double erlangMean = 0.0;
    double erlangM = 0.0;
    fitter->fitErlang(&sse, &erlangMean, &erlangM);
    printRow("Erlang", "mean", erlangMean, "m", erlangM,
             "", 0.0, "", 0.0, sse);

    double betaAlpha = 0.0;
    double betaBeta = 0.0;
    double betaInfimum = 0.0;
    double betaSupremum = 0.0;
    fitter->fitBeta(&sse, &betaAlpha, &betaBeta, &betaInfimum, &betaSupremum);
    printRow("Beta", "alpha", betaAlpha, "beta", betaBeta,
             "inf", betaInfimum, "sup", betaSupremum, sse);

    double weibullAlpha = 0.0;
    double weibullScale = 0.0;
    fitter->fitWeibull(&sse, &weibullAlpha, &weibullScale);
    printRow("Weibull", "alpha", weibullAlpha, "scale", weibullScale,
             "", 0.0, "", 0.0, sse);

    const FitSummary fitSummary = fitter->fitAllSummary();

    std::cout << "\nComplete ranking\n";
    for (std::size_t index = 0; index < fitSummary.ranking.size(); ++index) {
        const auto& candidate = fitSummary.ranking[index];
        std::cout << "  " << (index + 1U) << ". "
                  << std::left << std::setw(12) << candidate.distributionName
                  << " SSE=" << std::fixed << std::setprecision(6)
                  << candidate.squaredError << "\n";
    }

    std::cout << "\nBest fit: " << fitSummary.bestFit.distributionName
              << " (SSE=" << std::fixed << std::setprecision(6)
              << fitSummary.bestFit.squaredError << ")\n";

    const bool normalSseHeuristic95 = fitter->isNormalDistributed(0.95);
    const bool normalSseHeuristic99 = fitter->isNormalDistributed(0.99);
    std::cout << "Normal EDF/CDF SSE heuristic (confidence-like input=0.95): "
              << (normalSseHeuristic95 ? "pass" : "fail") << "\n"
              << "Normal EDF/CDF SSE heuristic (confidence-like input=0.99): "
              << (normalSseHeuristic99 ? "pass" : "fail") << "\n";

    // Demonstrate the probability helper functions used by fitter/tester code.
    printSection("Probability-distribution helpers");
    const double z975 = ProbabilityDistribution::inverseNormal(0.975, 0.0, 1.0);
    const double t975 = ProbabilityDistribution::inverseTStudent(
            0.975,
            0.0,
            1.0,
            static_cast<double>(primary.count - 1U));
    const double fittedNormalPdfAtMean =
            ProbabilityDistribution::normal(normalMean, normalMean, normalStddev);

    std::cout << "Standard normal q(0.975)         : " << std::fixed << std::setprecision(4)
              << z975 << "\n"
              << "Student t q(0.975, df=" << (primary.count - 1U) << "): "
              << t975 << "\n"
              << "Fitted normal PDF at fitted mean : " << fittedNormalPdfAtMean << "\n";

    HypothesisTester_if* tester = primaryAnalyser.tester();
    const double confidenceLevel = 0.95;
    const double expectedMean = 50.0;
    const double expectedVariance = 100.0;
    const double expectedProportionAbove50 = 0.5;

    const double primaryProportionAbove50 = static_cast<double>(
            std::count_if(primaryAnalyser.data().begin(),
                          primaryAnalyser.data().end(),
                          isGreaterThan50))
            / static_cast<double>(primary.count);

    // Demonstrate confidence intervals for one sample.
    printSection("One-population confidence intervals");
    const auto meanInterval = tester->averageConfidenceInterval(
            primary.mean, primary.stddev, static_cast<unsigned int>(primary.count), confidenceLevel);
    const auto varianceInterval = tester->varianceConfidenceInterval(
            primary.variance, static_cast<unsigned int>(primary.count), confidenceLevel);
    const auto proportionInterval = tester->proportionConfidenceInterval(
            primaryProportionAbove50, static_cast<unsigned int>(primary.count), confidenceLevel);

    printConfidenceInterval("Mean", meanInterval);
    printConfidenceInterval("Variance", varianceInterval, IntervalPrintMode::IntervalWidth);
    printConfidenceInterval("P(value > 50)", proportionInterval);

    // Demonstrate one-population tests with two-sided and one-sided alternatives.
    printSection("One-population hypothesis tests");
    const auto meanTwoSided = tester->testAverage(
            expectedMean,
            primary.stddev,
            static_cast<unsigned int>(primary.count),
            primary.mean,
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);
    const auto meanRightTailed = tester->testAverage(
            48.0,
            primary.stddev,
            static_cast<unsigned int>(primary.count),
            primary.mean,
            confidenceLevel,
            HypothesisTester_if::GREATER_THAN);
    const auto meanLeftTailed = tester->testAverage(
            52.0,
            primary.stddev,
            static_cast<unsigned int>(primary.count),
            primary.mean,
            confidenceLevel,
            HypothesisTester_if::LESS_THAN);
    const auto varianceTwoSided = tester->testVariance(
            primary.variance,
            static_cast<unsigned int>(primary.count),
            expectedVariance,
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);
    const auto proportionTwoSided = tester->testProportion(
            expectedProportionAbove50,
            static_cast<unsigned int>(primary.count),
            primaryProportionAbove50,
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);

    printTestResult("Mean == 50 (two-sided)", meanTwoSided);
    printTestResult("Mean > 48 (right-tailed)", meanRightTailed);
    printTestResult("Mean < 52 (left-tailed)", meanLeftTailed);
    printTestResult("Variance == 100", varianceTwoSided);
    printTestResult("P(value > 50) == 0.5", proportionTwoSided);

    const double groupAProportionAbove50 = static_cast<double>(
            std::count_if(groupAAnalyser.data().begin(),
                          groupAAnalyser.data().end(),
                          isGreaterThan50))
            / static_cast<double>(groupA.count);
    const double groupBProportionAbove50 = static_cast<double>(
            std::count_if(groupBAnalyser.data().begin(),
                          groupBAnalyser.data().end(),
                          isGreaterThan50))
            / static_cast<double>(groupB.count);

    // Demonstrate confidence intervals and tests comparing two independent groups.
    printSection("Two independent populations");
    const auto meanDifferenceInterval = tester->averageDifferenceConfidenceInterval(
            groupA.mean,
            groupA.stddev,
            static_cast<unsigned int>(groupA.count),
            groupB.mean,
            groupB.stddev,
            static_cast<unsigned int>(groupB.count),
            confidenceLevel);
    const auto proportionDifferenceInterval = tester->proportionDifferenceConfidenceInterval(
            groupAProportionAbove50,
            0.0,
            static_cast<unsigned int>(groupA.count),
            groupBProportionAbove50,
            0.0,
            static_cast<unsigned int>(groupB.count),
            confidenceLevel);
    const auto varianceRatioInterval = tester->varianceRatioConfidenceInterval(
            groupA.variance,
            static_cast<unsigned int>(groupA.count),
            groupB.variance,
            static_cast<unsigned int>(groupB.count),
            confidenceLevel);

    const auto groupMeanTest = tester->testAverage(
            groupA.mean,
            groupA.stddev,
            static_cast<unsigned int>(groupA.count),
            groupB.mean,
            groupB.stddev,
            static_cast<unsigned int>(groupB.count),
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);
    const auto groupVarianceTest = tester->testVariance(
            groupA.variance,
            static_cast<unsigned int>(groupA.count),
            groupB.variance,
            static_cast<unsigned int>(groupB.count),
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);
    const auto groupProportionTest = tester->testProportion(
            groupAProportionAbove50,
            static_cast<unsigned int>(groupA.count),
            groupBProportionAbove50,
            static_cast<unsigned int>(groupB.count),
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);

    printConfidenceInterval("Difference of means (A - B)", meanDifferenceInterval);
    printConfidenceInterval("Difference of proportions (A - B)", proportionDifferenceInterval);
    printConfidenceInterval("Variance ratio (A / B)", varianceRatioInterval, IntervalPrintMode::IntervalWidth);
    std::cout << "P_A(value > 50)=" << std::fixed << std::setprecision(4)
              << groupAProportionAbove50
              << "  P_B(value > 50)=" << groupBProportionAbove50 << "\n";
    printTestResult("Mean A == mean B (" + selectedTwoMeanMethod(tester, groupA, groupB, confidenceLevel) + ")", groupMeanTest);
    printTestResult("Variance A == variance B", groupVarianceTest);
    printTestResult("P_A(>50) == P_B(>50)", groupProportionTest);

    // Demonstrate file-based overloads that load datasets internally.
    printSection("File-based inference overloads");
    const auto fileMeanInterval = tester->averageConfidenceInterval(primaryFile, confidenceLevel);
    const auto fileMeanTest = tester->testAverage(
            primaryFile,
            expectedMean,
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);
    const auto fileGroupMeanTest = tester->testAverage(
            groupAFile,
            groupBFile,
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);
    const auto fileGroupVarianceTest = tester->testVariance(
            groupAFile,
            groupBFile,
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);
    const auto fileGroupProportionTest = tester->testProportion(
            groupAFile,
            groupBFile,
            isGreaterThan50,
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);

    printConfidenceInterval("Mean from filename", fileMeanInterval);
    printTestResult("Mean == 50 from filename", fileMeanTest);
    printTestResult("Group means from filenames", fileGroupMeanTest);
    printTestResult("Group variances from filenames", fileGroupVarianceTest);
    printTestResult("Group proportions from filenames", fileGroupProportionTest);

    // Demonstrate goodness-of-fit tests against the fitted normal CDF.
    printSection("Goodness-of-fit for fitted normal distribution");
    const auto fittedNormalCdf = [normalMean, normalStddev](double value) {
        return 0.5 * std::erfc(
                -(value - normalMean) / (normalStddev * std::sqrt(2.0)));
    };

    const auto chiSquareResult = tester->chiSquareGoodnessOfFit(
            primaryAnalyser.data(),
            fittedNormalCdf,
            2,
            confidenceLevel,
            10,
            5.0);
    const auto ksResult = tester->kolmogorovSmirnov(
            primaryAnalyser.data(),
            fittedNormalCdf,
            confidenceLevel);
    const auto memoryMeanInterval = memoryAnalyser.tester()->averageConfidenceInterval(
            memorySummary.mean,
            memorySummary.stddev,
            static_cast<unsigned int>(memorySummary.count),
            confidenceLevel);
    const auto memoryMeanTest = memoryAnalyser.tester()->testAverage(
            expectedMean,
            memorySummary.stddev,
            static_cast<unsigned int>(memorySummary.count),
            memorySummary.mean,
            confidenceLevel,
            HypothesisTester_if::DIFFERENT);
    double memoryNormalSse = 0.0;
    double memoryNormalMean = 0.0;
    double memoryNormalStddev = 0.0;
    memoryAnalyser.fitter()->fitNormal(&memoryNormalSse, &memoryNormalMean, &memoryNormalStddev);
    const FitSummary memoryFitSummary = memoryAnalyser.fitter()->fitAllSummary();

    printChiSquareResult("Chi-square fitted normal", chiSquareResult);
    printTestResult("KS fitted normal", ksResult);
    std::cout << "Chi-square configuration: 10 initial classes, minimum expected frequency 5,\n"
              << "2 parameters estimated from the sample.\n"
              << "KS note: the reported p-value is the standard approximation even though\n"
              << "the normal parameters were estimated from this same sample.\n";

    // Keep this example useful as a lightweight regression check.
    printSection("Deterministic regression checks for the bundled datasets");
    unsigned int failures = 0U;

    regressionCheck(primary.count == 160U,
                    "primary file contains 160 observations",
                    failures);
    regressionCheck(summariesEquivalent(memorySummary, primary),
                    "file and in-memory facade ingestion produce the same summary",
                    failures);
    regressionCheck(boxplotsEquivalent(memoryBoxplot, primaryBoxplot),
                    "file and in-memory facade ingestion produce the same boxplot",
                    failures);
    regressionCheck(histogramsEquivalent(memoryHistogram, primaryHistogram),
                    "file and in-memory facade ingestion produce the same histogram",
                    failures);
    regressionCheck(approximatelyEqual(memoryNormalMean, normalMean)
                    && approximatelyEqual(memoryNormalStddev, normalStddev)
                    && approximatelyEqual(memoryNormalSse, normalSse),
                    "file and in-memory facade ingestion produce the same normal fit",
                    failures);
    regressionCheck(memoryFitSummary.success
                    && memoryFitSummary.bestFit.distributionName == fitSummary.bestFit.distributionName
                    && approximatelyEqual(memoryFitSummary.bestFit.squaredError, fitSummary.bestFit.squaredError),
                    "file and in-memory facade ingestion produce the same best fit",
                    failures);
    regressionCheck(approximatelyEqual(memoryMeanInterval.inferiorLimit(), meanInterval.inferiorLimit())
                    && approximatelyEqual(memoryMeanInterval.superiorLimit(), meanInterval.superiorLimit())
                    && approximatelyEqual(memoryMeanTest.testStat(), meanTwoSided.testStat())
                    && approximatelyEqual(memoryMeanTest.pValue(), meanTwoSided.pValue(), 1e-7),
                    "file and in-memory facade ingestion produce the same mean inference",
                    failures);
    regressionCheck(histogramFrequencySum(primaryHistogram) == primary.count,
                    "histogram bin frequencies sum to the sample count",
                    failures);
    regressionCheck(histogramIsOrderedAndContiguous(primaryHistogram),
                    "histogram bins are ordered, contiguous and cover min/max",
                    failures);
    regressionCheck(boxplotIsOrdered(primaryBoxplot),
                    "boxplot five-number summary and whiskers are ordered",
                    failures);
    regressionCheck(!fitSummary.ranking.empty(),
                    "fitAllSummary returned at least one valid candidate",
                    failures);
    regressionCheck(std::is_sorted(
                            fitSummary.ranking.begin(),
                            fitSummary.ranking.end(),
                            [](const auto& lhs, const auto& rhs) {
                                return lhs.squaredError < rhs.squaredError;
                            }),
                    "fitAllSummary ranking is ordered by nondecreasing SSE",
                    failures);
    regressionCheck(lower(fitSummary.bestFit.distributionName) == "normal",
                    "normal is the best EDF/CDF fit for the primary random sample",
                    failures);
    std::set<std::string> rankingNames;
    bool rankingHasFiniteNonnegativeErrors = true;
    for (const auto& candidate : fitSummary.ranking) {
        rankingNames.insert(lower(candidate.distributionName));
        rankingHasFiniteNonnegativeErrors = rankingHasFiniteNonnegativeErrors
                && std::isfinite(candidate.squaredError)
                && candidate.squaredError >= 0.0;
    }
    regressionCheck(fitSummary.ranking.size() == 7U && rankingNames.size() == fitSummary.ranking.size(),
                    "fitAllSummary ranking contains seven unique candidates",
                    failures);
    regressionCheck(rankingHasFiniteNonnegativeErrors,
                    "fitAllSummary ranking errors are finite and nonnegative",
                    failures);
    regressionCheck(fitSummary.ranking.front().distributionName == fitSummary.bestFit.distributionName
                    && approximatelyEqual(fitSummary.ranking.front().squaredError, fitSummary.bestFit.squaredError),
                    "fitAllSummary best fit matches the first ranked candidate",
                    failures);
    regressionCheck(!meanTwoSided.rejectH0(),
                    "the generating mean 50 is not rejected",
                    failures);
    regressionCheck(!varianceTwoSided.rejectH0(),
                    "the generating variance 100 is not rejected",
                    failures);
    regressionCheck(!proportionTwoSided.rejectH0(),
                    "the theoretical P(X > 50)=0.5 is not rejected",
                    failures);
    regressionCheck(groupMeanTest.rejectH0(),
                    "independent groups with different generating means are distinguished",
                    failures);
    regressionCheck(!groupVarianceTest.rejectH0(),
                    "equal generating variances are not rejected",
                    failures);
    regressionCheck(groupProportionTest.rejectH0(),
                    "the independent groups have distinguishable proportions above 50",
                    failures);
    regressionCheck(!chiSquareResult.rejectH0(),
                    "chi-square does not reject the fitted normal model",
                    failures);
    regressionCheck(!ksResult.rejectH0(),
                    "KS does not reject the fitted normal model",
                    failures);

    std::cout << "\nRegression result: "
              << (failures == 0U ? "ALL CHECKS PASSED" : "CHECKS FAILED")
              << " (failures=" << failures << ")\n";

    return failures == 0U ? 0 : 2;
}
