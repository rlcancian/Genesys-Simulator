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
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int COL_NAME = 14;
constexpr int COL_PARAM = 18;
constexpr int COL_SSE = 12;
constexpr double EPSILON = 1.0e-9;

bool isGreaterThan50(double value) {
    return value > 50.0;
}

std::string decision(bool rejectH0) {
    return rejectH0 ? "reject H0" : "do not reject H0";
}

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

std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

void printSection(const std::string& title) {
    std::cout << "\n== " << title << " ==\n";
}

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

void printConfidenceInterval(const std::string& name,
                             HypothesisTester_if::ConfidenceInterval interval) {
    std::cout << std::left << std::setw(34) << name
              << "[" << std::fixed << std::setprecision(4)
              << interval.inferiorLimit() << ", " << interval.superiorLimit() << "]"
              << "  half-width=" << interval.halfWidth() << "\n";
}

void printTestResult(const std::string& name,
                     const HypothesisTester_if::TestResult& result) {
    std::cout << std::left << std::setw(34) << name
              << "stat=" << std::fixed << std::setprecision(4) << result.testStat()
              << "  p=" << pValueText(result.pValue())
              << "  decision=" << decision(result.rejectH0()) << "\n";
}

bool load(DataAnalyserDefaultImpl& analyser, const std::string& filename) {
    if (analyser.loadDataSet(filename) && analyser.summary().count > 1U) {
        return true;
    }

    std::cerr << "ERROR: Could not load a usable dataset from '" << filename << "'.\n";
    return false;
}

bool approximatelyEqual(double lhs, double rhs, double tolerance = EPSILON) {
    return std::abs(lhs - rhs) <= tolerance;
}

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

    printSection("Descriptive statistics and exploratory structures");
    printSummary("Primary sample", primaryFile, primary, primaryBoxplot, primaryHistogram);
    std::cout << "\n";
    printSummary("Independent group A", groupAFile, groupA, groupABoxplot, groupAHistogram);
    std::cout << "\n";
    printSummary("Independent group B", groupBFile, groupB, groupBBoxplot, groupBHistogram);

    // Demonstrate ingestion from an in-memory vector using the same values loaded
    // through the DataAnalyser facade.
    DatasetLoader memoryCopy;
    memoryCopy.loadFromVector(primaryAnalyser.data());
    std::cout << "\nMemory copy verification\n"
              << "  Samples: " << memoryCopy.count()
              << "  Mean: " << std::fixed << std::setprecision(4) << memoryCopy.mean()
              << "  Std. deviation: " << memoryCopy.stddev() << "\n";

    Fitter_if* fitter = primaryAnalyser.fitter();

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

    const bool normalityNotRejected95 = fitter->isNormalDistributed(0.05);
    const bool normalityNotRejected99 = fitter->isNormalDistributed(0.01);
    std::cout << "Normality H0 at alpha=0.05: "
              << (normalityNotRejected95 ? "do not reject" : "reject") << "\n"
              << "Normality H0 at alpha=0.01: "
              << (normalityNotRejected99 ? "do not reject" : "reject") << "\n";

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

    printSection("One-population confidence intervals");
    const auto meanInterval = tester->averageConfidenceInterval(
            primary.mean, primary.stddev, static_cast<unsigned int>(primary.count), confidenceLevel);
    const auto varianceInterval = tester->varianceConfidenceInterval(
            primary.variance, static_cast<unsigned int>(primary.count), confidenceLevel);
    const auto proportionInterval = tester->proportionConfidenceInterval(
            primaryProportionAbove50, static_cast<unsigned int>(primary.count), confidenceLevel);

    printConfidenceInterval("Mean", meanInterval);
    printConfidenceInterval("Variance", varianceInterval);
    printConfidenceInterval("P(value > 50)", proportionInterval);

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
    printConfidenceInterval("Variance ratio (A / B)", varianceRatioInterval);
    std::cout << "P_A(value > 50)=" << std::fixed << std::setprecision(4)
              << groupAProportionAbove50
              << "  P_B(value > 50)=" << groupBProportionAbove50 << "\n";
    printTestResult("Mean A == mean B", groupMeanTest);
    printTestResult("Variance A == variance B", groupVarianceTest);
    printTestResult("P_A(>50) == P_B(>50)", groupProportionTest);

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

    printTestResult("Chi-square fitted normal", chiSquareResult);
    printTestResult("KS fitted normal", ksResult);
    std::cout << "Chi-square configuration: 10 initial classes, minimum expected frequency 5,\n"
              << "2 parameters estimated from the sample.\n"
              << "KS note: the reported p-value is the standard approximation even though\n"
              << "the normal parameters were estimated from this same sample.\n";

    printSection("Deterministic regression checks for the bundled datasets");
    unsigned int failures = 0U;

    regressionCheck(primary.count == 160U,
                    "primary file contains 160 observations",
                    failures);
    regressionCheck(memoryCopy.count() == primary.count
                    && approximatelyEqual(memoryCopy.mean(), primary.mean),
                    "file and in-memory ingestion produce the same count and mean",
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
