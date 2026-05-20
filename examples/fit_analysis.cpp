#include "tools/analysis/DataAnalyserDefaultImpl.h"
#include "tools/analysis/FitterDefaultImpl.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {

void printHeader(const std::string& dataFile, std::size_t n, double mean, double stddev, double min, double max) {
    std::cout << "====================================================\n"
              << "  Genesys Simulator -- Distribution Fit Analysis\n"
              << "====================================================\n"
              << "Data file  : " << dataFile << "\n"
              << "Samples    : " << n << "\n"
              << std::fixed << std::setprecision(4)
              << "Mean       : " << mean << "\n"
              << "Std dev    : " << stddev << "\n"
              << "Min        : " << min << "   Max: " << max << "\n"
              << "\n";
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

} // namespace

int main(int argc, char* argv[]) {
    const std::string dataFile = (argc > 1) ? argv[1] : "examples/data/sample_data.csv";

    FitterDefaultImpl fitter;
    DataAnalyserDefaultImpl analyser(&fitter);

    analyser.loadDataSet(dataFile);

    // Verify that data was loaded by probing the Normal fit result.
    {
        double sse = 0.0, avg = 0.0, sd = 0.0;
        analyser.fitter()->fitNormal(&sse, &avg, &sd);
        if (!std::isfinite(avg)) {
            std::cerr << "ERROR: Could not load data from '" << dataFile << "'.\n"
                      << "       Execute a partir da raiz do projeto, ex.:\n"
                      << "         ./build/examples/examples/genesys_fit_analysis\n"
                      << "       ou passe o caminho do arquivo como argumento.\n";
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

    printHeader(dataFile, 0 /* count not exposed via Fitter_if */, avg, sd, uMin, uMax);

    // --- Individual fits ---
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
    std::string bestName;
    double bestSse;
    f->fitAll(&bestSse, &bestName);

    std::cout << "\n"
              << "=> Best fit : " << bestName
              << "  (SSE = " << std::fixed << std::setprecision(6) << bestSse << ")\n";

    // --- Normality check ---
    const bool isNormal95 = f->isNormalDistributed(0.05);
    const bool isNormal99 = f->isNormalDistributed(0.01);
    std::cout << "=> Is normally distributed (alpha=0.05): " << (isNormal95 ? "YES" : "NO") << "\n"
              << "=> Is normally distributed (alpha=0.01): " << (isNormal99 ? "YES" : "NO") << "\n";

    return 0;
}
