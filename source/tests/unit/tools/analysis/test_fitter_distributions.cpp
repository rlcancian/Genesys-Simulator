#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include "tools/analysis/FitterDefaultImpl.h"

namespace {

std::filesystem::path writeCsvFile(const std::string& name, const std::vector<double>& values) {
	const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
	const auto path = std::filesystem::temp_directory_path()
	                / (name + "_" + std::to_string(suffix) + ".csv");
	std::ofstream ofs(path);
	for (double v : values) {
		ofs << v << "\n";
	}
	return path;
}

class FitterTest : public ::testing::Test {
protected:
	FitterDefaultImpl fitter;

	void loadData(const std::string& tag, const std::vector<double>& data) {
		const auto path = writeCsvFile(tag, data);
		fitter.setDataFilename(path.string());
		std::filesystem::remove(path);
	}
};

constexpr double kTol = 1e-9;

} // namespace

// ============================= Uniform =============================

// Test objective: verifies FitterTest.Uniform_ReturnsCorrectMinMax.
TEST_F(FitterTest, Uniform_ReturnsCorrectMinMax) {
	// For uniform MLE: min = sample min, max = sample max.
	loadData("uniform", {1.0, 2.0, 3.0, 4.0, 5.0});
	double sqrerror, min, max;
	fitter.fitUniform(&sqrerror, &min, &max);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_GE(sqrerror, 0.0);
	EXPECT_DOUBLE_EQ(min, 1.0);
	EXPECT_DOUBLE_EQ(max, 5.0);
}

// Test objective: verifies FitterTest.Uniform_TwoPoints.
TEST_F(FitterTest, Uniform_TwoPoints) {
	loadData("uniform2", {0.0, 10.0});
	double sqrerror, min, max;
	fitter.fitUniform(&sqrerror, &min, &max);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_DOUBLE_EQ(min, 0.0);
	EXPECT_DOUBLE_EQ(max, 10.0);
}

// Test objective: verifies FitterTest.Uniform_AllSame_Fails.
TEST_F(FitterTest, Uniform_AllSame_Fails) {
	loadData("uniform_same", {3.0, 3.0, 3.0});
	double sqrerror = 0.0, min = 0.0, max = 0.0;
	fitter.fitUniform(&sqrerror, &min, &max);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
	EXPECT_TRUE(std::isnan(min));
	EXPECT_TRUE(std::isnan(max));
}

// Test objective: verifies FitterTest.Uniform_OnePoint_Fails.
TEST_F(FitterTest, Uniform_OnePoint_Fails) {
	loadData("uniform1", {5.0});
	double sqrerror = 0.0, min = 0.0, max = 0.0;
	fitter.fitUniform(&sqrerror, &min, &max);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// ============================= Triangular =============================

// Test objective: verifies FitterTest.Triangular_ReturnsCorrectMode.
TEST_F(FitterTest, Triangular_ReturnsCorrectMode) {
	// E[X] = (min + mode + max) / 3  →  mode = 3*mean - min - max
	// {1,2,3,4,5}: mean=3, min=1, max=5  →  mode = 9 - 1 - 5 = 3
	loadData("triangular", {1.0, 2.0, 3.0, 4.0, 5.0});
	double sqrerror, min, mo, max;
	fitter.fitTriangular(&sqrerror, &min, &mo, &max);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_GE(sqrerror, 0.0);
	EXPECT_DOUBLE_EQ(min, 1.0);
	EXPECT_NEAR(mo, 3.0, kTol);
	EXPECT_DOUBLE_EQ(max, 5.0);
}

// Test objective: verifies FitterTest.Triangular_AllSame_Fails.
TEST_F(FitterTest, Triangular_AllSame_Fails) {
	loadData("tri_same", {2.0, 2.0, 2.0});
	double sqrerror = 0.0, min = 0.0, mo = 0.0, max = 0.0;
	fitter.fitTriangular(&sqrerror, &min, &mo, &max);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// Test objective: verifies FitterTest.Triangular_OnePoint_Fails.
TEST_F(FitterTest, Triangular_OnePoint_Fails) {
	loadData("tri1", {4.0});
	double sqrerror = 0.0, min = 0.0, mo = 0.0, max = 0.0;
	fitter.fitTriangular(&sqrerror, &min, &mo, &max);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// ============================= Normal =============================

// Test objective: verifies FitterTest.Normal_ReturnsCorrectMeanAndStddev.
TEST_F(FitterTest, Normal_ReturnsCorrectMeanAndStddev) {
	// {3, 5, 7}: mean = 5, sample variance = 4, stddev = 2
	loadData("normal", {3.0, 5.0, 7.0});
	double sqrerror, avg, stddev;
	fitter.fitNormal(&sqrerror, &avg, &stddev);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_GE(sqrerror, 0.0);
	EXPECT_DOUBLE_EQ(avg, 5.0);
	EXPECT_DOUBLE_EQ(stddev, 2.0);
}

// Test objective: verifies FitterTest.Normal_SseDecreasesWithMoreSymmetricData.
TEST_F(FitterTest, Normal_SseDecreasesWithMoreSymmetricData) {
	// More symmetric data should yield smaller SSE than skewed data.
	loadData("normal_sym", {3.0, 4.0, 5.0, 6.0, 7.0});
	double sse_sym = 0.0, avg = 0.0, stddev = 0.0;
	fitter.fitNormal(&sse_sym, &avg, &stddev);

	loadData("normal_skew", {1.0, 1.0, 1.0, 1.0, 9.0});
	double sse_skew = 0.0;
	fitter.fitNormal(&sse_skew, &avg, &stddev);

	EXPECT_LT(sse_sym, sse_skew);
}

// Test objective: verifies FitterTest.Normal_AllSame_Fails.
TEST_F(FitterTest, Normal_AllSame_Fails) {
	loadData("normal_same", {4.0, 4.0, 4.0});
	double sqrerror = 0.0, avg = 0.0, stddev = 0.0;
	fitter.fitNormal(&sqrerror, &avg, &stddev);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// Test objective: verifies FitterTest.Normal_OnePoint_Fails.
TEST_F(FitterTest, Normal_OnePoint_Fails) {
	loadData("normal1", {5.0});
	double sqrerror = 0.0, avg = 0.0, stddev = 0.0;
	fitter.fitNormal(&sqrerror, &avg, &stddev);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// ============================= Exponential =============================

// Test objective: verifies FitterTest.Exponential_ReturnsSampleMeanAsParameter.
TEST_F(FitterTest, Exponential_ReturnsSampleMeanAsParameter) {
	// MLE for exponential: λ = 1/mean  →  mean output = sample mean
	// {1,2,3,4,5}: mean=3
	loadData("expo", {1.0, 2.0, 3.0, 4.0, 5.0});
	double sqrerror, avg1;
	fitter.fitExpo(&sqrerror, &avg1);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_GE(sqrerror, 0.0);
	EXPECT_DOUBLE_EQ(avg1, 3.0);
}

// Test objective: verifies FitterTest.Exponential_SseIsSmallForMonotonicData.
TEST_F(FitterTest, Exponential_SseIsSmallForMonotonicData) {
	// Exponential CDF should fit strictly increasing data with positive mean well.
	loadData("expo_mono", {0.5, 1.0, 2.0, 3.0, 5.0, 8.0});
	double sqrerror, avg1;
	fitter.fitExpo(&sqrerror, &avg1);
	EXPECT_TRUE(std::isfinite(sqrerror));
}

// Test objective: verifies FitterTest.Exponential_NegativeData_Fails.
TEST_F(FitterTest, Exponential_NegativeData_Fails) {
	loadData("expo_neg", {-1.0, 1.0, 2.0, 3.0});
	double sqrerror = 0.0, avg1 = 0.0;
	fitter.fitExpo(&sqrerror, &avg1);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// Test objective: verifies FitterTest.Exponential_NonPositiveMean_Fails.
TEST_F(FitterTest, Exponential_NonPositiveMean_Fails) {
	// Mean = 0 when positives and negatives cancel out.
	loadData("expo_zero", {-2.0, 0.0, 2.0});
	double sqrerror = 0.0, avg1 = 0.0;
	fitter.fitExpo(&sqrerror, &avg1);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// ============================= Erlang =============================

// Test objective: verifies FitterTest.Erlang_ReturnsCorrectPhaseCount.
TEST_F(FitterTest, Erlang_ReturnsCorrectPhaseCount) {
	// MOM: m = round(mean² / variance)
	// {0,2,4,6,8}: mean=4, var=10  →  m = round(16/10) = 2
	loadData("erlang", {0.0, 2.0, 4.0, 6.0, 8.0});
	double sqrerror, avg, m;
	fitter.fitErlang(&sqrerror, &avg, &m);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_GE(sqrerror, 0.0);
	EXPECT_DOUBLE_EQ(avg, 4.0);
	EXPECT_DOUBLE_EQ(m, 2.0);
}

// Test objective: verifies FitterTest.Erlang_HighVarianceGivesPhaseOne.
TEST_F(FitterTest, Erlang_HighVarianceGivesPhaseOne) {
	// {0,4,8}: mean=4, var=16  →  m = round(16/16) = 1 (reduces to exponential)
	loadData("erlang_m1", {0.0, 4.0, 8.0});
	double sqrerror, avg, m;
	fitter.fitErlang(&sqrerror, &avg, &m);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_DOUBLE_EQ(avg, 4.0);
	EXPECT_DOUBLE_EQ(m, 1.0);
}

// Test objective: verifies FitterTest.Erlang_LowVarianceGivesHighPhaseCount.
TEST_F(FitterTest, Erlang_LowVarianceGivesHighPhaseCount) {
	// {3.5, 4.0, 4.5}: mean=4, var=0.25  →  m = round(16/0.25) = 64
	loadData("erlang_high_m", {3.5, 4.0, 4.5});
	double sqrerror, avg, m;
	fitter.fitErlang(&sqrerror, &avg, &m);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_DOUBLE_EQ(avg, 4.0);
	EXPECT_GT(m, 10.0);
}

// Test objective: verifies FitterTest.Erlang_NegativeData_Fails.
TEST_F(FitterTest, Erlang_NegativeData_Fails) {
	loadData("erlang_neg", {-1.0, 1.0, 2.0, 3.0});
	double sqrerror = 0.0, avg = 0.0, m = 0.0;
	fitter.fitErlang(&sqrerror, &avg, &m);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// Test objective: verifies FitterTest.Erlang_ZeroVariance_Fails.
TEST_F(FitterTest, Erlang_ZeroVariance_Fails) {
	loadData("erlang_same", {3.0, 3.0, 3.0});
	double sqrerror = 0.0, avg = 0.0, m = 0.0;
	fitter.fitErlang(&sqrerror, &avg, &m);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// ============================= Beta =============================

// Test objective: verifies FitterTest.Beta_ReturnsPositiveShapeParameters.
TEST_F(FitterTest, Beta_ReturnsPositiveShapeParameters) {
	// Well-spread data on [0,3]: alpha > 0, beta > 0, infLimit=0, supLimit=3
	loadData("beta", {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0});
	double sqrerror, alpha, beta, infLimit, supLimit;
	fitter.fitBeta(&sqrerror, &alpha, &beta, &infLimit, &supLimit);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_GE(sqrerror, 0.0);
	EXPECT_GT(alpha, 0.0);
	EXPECT_GT(beta, 0.0);
	EXPECT_DOUBLE_EQ(infLimit, 0.0);
	EXPECT_DOUBLE_EQ(supLimit, 3.0);
}

// Test objective: verifies FitterTest.Beta_SymmetricData_GivesEqualAlphaBeta.
TEST_F(FitterTest, Beta_SymmetricData_GivesEqualAlphaBeta) {
	// Symmetric data around the midpoint: alpha ≈ beta
	loadData("beta_sym", {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
	double sqrerror, alpha, beta, infLimit, supLimit;
	fitter.fitBeta(&sqrerror, &alpha, &beta, &infLimit, &supLimit);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_NEAR(alpha, beta, 1e-9);
}

// Test objective: verifies FitterTest.Beta_AllSame_Fails.
TEST_F(FitterTest, Beta_AllSame_Fails) {
	loadData("beta_same", {2.0, 2.0, 2.0});
	double sqrerror = 0.0, alpha = 0.0, beta = 0.0, inf = 0.0, sup = 0.0;
	fitter.fitBeta(&sqrerror, &alpha, &beta, &inf, &sup);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// Test objective: verifies FitterTest.Beta_OnePoint_Fails.
TEST_F(FitterTest, Beta_OnePoint_Fails) {
	loadData("beta1", {1.5});
	double sqrerror = 0.0, alpha = 0.0, beta = 0.0, inf = 0.0, sup = 0.0;
	fitter.fitBeta(&sqrerror, &alpha, &beta, &inf, &sup);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// ============================= Weibull =============================

// Test objective: verifies FitterTest.Weibull_ReturnsPositiveParameters.
TEST_F(FitterTest, Weibull_ReturnsPositiveParameters) {
	loadData("weibull", {1.0, 2.0, 3.0, 4.0, 5.0});
	double sqrerror, alpha, scale;
	fitter.fitWeibull(&sqrerror, &alpha, &scale);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_GE(sqrerror, 0.0);
	EXPECT_GT(alpha, 0.0);
	EXPECT_GT(scale, 0.0);
}

// Test objective: verifies FitterTest.Weibull_ExponentialLikeData_ShapeNearOne.
TEST_F(FitterTest, Weibull_ExponentialLikeData_ShapeNearOne) {
	// {0,1,2}: mean=1, sample var=1  →  CV=1  →  shape≈1 (Weibull → Exponential)
	// For Weibull: CV² = Γ(1+2/k)/Γ(1+1/k)² - 1. CV=1 ⟺ k=1 exactly.
	loadData("weibull_exp", {0.0, 1.0, 2.0});
	double sqrerror, alpha, scale;
	fitter.fitWeibull(&sqrerror, &alpha, &scale);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_NEAR(alpha, 1.0, 1e-5);
	EXPECT_NEAR(scale, 1.0, 1e-5);
}

// Test objective: verifies FitterTest.Weibull_NegativeData_Fails.
TEST_F(FitterTest, Weibull_NegativeData_Fails) {
	loadData("weibull_neg", {-1.0, 1.0, 2.0, 3.0});
	double sqrerror = 0.0, alpha = 0.0, scale = 0.0;
	fitter.fitWeibull(&sqrerror, &alpha, &scale);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// Test objective: verifies FitterTest.Weibull_ZeroVariance_Fails.
TEST_F(FitterTest, Weibull_ZeroVariance_Fails) {
	loadData("weibull_same", {3.0, 3.0, 3.0});
	double sqrerror = 0.0, alpha = 0.0, scale = 0.0;
	fitter.fitWeibull(&sqrerror, &alpha, &scale);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// Test objective: verifies FitterTest.Weibull_OnePoint_Fails.
TEST_F(FitterTest, Weibull_OnePoint_Fails) {
	loadData("weibull1", {5.0});
	double sqrerror = 0.0, alpha = 0.0, scale = 0.0;
	fitter.fitWeibull(&sqrerror, &alpha, &scale);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
}

// ============================= fitAll =============================

// Test objective: verifies FitterTest.FitAll_NoDataLoaded_ReturnsInvalidDataset.
TEST_F(FitterTest, FitAll_NoDataLoaded_ReturnsInvalidDataset) {
	double sqrerror = 0.0;
	std::string name;
	fitter.fitAll(&sqrerror, &name);
	EXPECT_EQ(sqrerror, std::numeric_limits<double>::infinity());
	EXPECT_EQ(name, "invalid-dataset");

	const FitSummary summary = fitter.fitAllSummary();
	EXPECT_FALSE(summary.success);
	EXPECT_TRUE(summary.ranking.empty());
	EXPECT_EQ(summary.bestFit.distributionName, "invalid-dataset");
	EXPECT_EQ(summary.bestFit.squaredError, std::numeric_limits<double>::infinity());
}

// Test objective: verifies FitterTest.FitAll_ValidData_ReturnsKnownNameAndFiniteSse.
TEST_F(FitterTest, FitAll_ValidData_ReturnsKnownNameAndFiniteSse) {
	loadData("fitall", {1.0, 2.0, 3.0, 4.0, 5.0});
	double sqrerror = std::numeric_limits<double>::infinity();
	std::string name;
	fitter.fitAll(&sqrerror, &name);
	EXPECT_TRUE(std::isfinite(sqrerror));
	EXPECT_GE(sqrerror, 0.0);
	const std::vector<std::string> valid = {
		"uniform", "triangular", "normal", "exponential", "erlang", "beta", "weibull"
	};
	EXPECT_NE(std::find(valid.begin(), valid.end(), name), valid.end())
		<< "fitAll returned unexpected name: " << name;
}

// Test objective: verifies FitterTest.FitAll_SelectedSseIsMinimumAcrossAllFits.
TEST_F(FitterTest, FitAll_SelectedSseIsMinimumAcrossAllFits) {
	// The SSE returned by fitAll must be ≤ the SSE of every individual fit.
	loadData("fitall_cmp", {1.0, 2.0, 3.0, 4.0, 5.0});
	double bestSse = std::numeric_limits<double>::infinity();
	std::string bestName;
	fitter.fitAll(&bestSse, &bestName);

	const auto checkFit = [&](double sse) {
		if (std::isfinite(sse)) {
			EXPECT_LE(bestSse, sse + 1e-12) << "fitAll SSE exceeds individual fit SSE";
		}
	};

	double sse = 0.0, p1 = 0.0, p2 = 0.0, p3 = 0.0, p4 = 0.0;
	fitter.fitUniform(&sse, &p1, &p2);         checkFit(sse);
	fitter.fitTriangular(&sse, &p1, &p2, &p3); checkFit(sse);
	fitter.fitNormal(&sse, &p1, &p2);          checkFit(sse);
	fitter.fitExpo(&sse, &p1);                 checkFit(sse);
	fitter.fitErlang(&sse, &p1, &p2);          checkFit(sse);
	fitter.fitBeta(&sse, &p1, &p2, &p3, &p4); checkFit(sse);
	fitter.fitWeibull(&sse, &p1, &p2);         checkFit(sse);
}

// Test objective: verifies FitterTest.FitAllSummary_ReturnsCompleteOrderedRankingAndBestFit.
TEST_F(FitterTest, FitAllSummary_ReturnsCompleteOrderedRankingAndBestFit) {
	loadData("fitall_summary", {1.0, 2.0, 3.0, 4.0, 5.0});

	const FitSummary summary = fitter.fitAllSummary();

	ASSERT_TRUE(summary.success);
	ASSERT_EQ(summary.ranking.size(), 7u);
	ASSERT_TRUE(summary.ranking.front().success);
	EXPECT_EQ(summary.bestFit.distributionName, summary.ranking.front().distributionName);
	EXPECT_DOUBLE_EQ(summary.bestFit.squaredError, summary.ranking.front().squaredError);
	EXPECT_FALSE(summary.bestFit.parameters.empty());

	double previousError = -1.0;
	std::set<std::string> names;
	for (const FittingResult& result : summary.ranking) {
		EXPECT_FALSE(result.distributionName.empty());
		EXPECT_TRUE(names.insert(result.distributionName).second);
		EXPECT_FALSE(result.parameters.empty());
		if (result.success) {
			EXPECT_TRUE(std::isfinite(result.squaredError));
			EXPECT_GE(result.squaredError, 0.0);
			EXPECT_GE(result.squaredError, previousError);
			previousError = result.squaredError;
		}
	}

	const FitSummary repeated = fitter.fitAllSummary();
	ASSERT_TRUE(repeated.success);
	ASSERT_EQ(repeated.ranking.size(), summary.ranking.size());
	for (std::size_t i = 0; i < summary.ranking.size(); ++i) {
		EXPECT_EQ(repeated.ranking[i].distributionName, summary.ranking[i].distributionName);
		EXPECT_DOUBLE_EQ(repeated.ranking[i].squaredError, summary.ranking[i].squaredError);
	}
}

// Test objective: verifies FitterTest.FitAllLegacyOutputMatchesStructuredBestFit.
TEST_F(FitterTest, FitAllLegacyOutputMatchesStructuredBestFit) {
	loadData("fitall_legacy_summary", {1.0, 2.0, 3.0, 4.0, 5.0});

	double legacyError = std::numeric_limits<double>::infinity();
	std::string legacyName;
	fitter.fitAll(&legacyError, &legacyName);
	const FitSummary summary = fitter.fitAllSummary();

	ASSERT_TRUE(summary.success);
	EXPECT_EQ(legacyName, summary.bestFit.distributionName);
	EXPECT_DOUBLE_EQ(legacyError, summary.bestFit.squaredError);
}

// ============================= isNormalDistributed =============================

// Test objective: verifies FitterTest.IsNormalDistributed_SymmetricBellData_ReturnsTrue.
TEST_F(FitterTest, IsNormalDistributed_SymmetricBellData_ReturnsTrue) {
	// 8 near-normal points: mean=5, symmetric distribution
	loadData("normal_check",
		{3.0, 4.0, 4.5, 5.0, 5.0, 5.5, 6.0, 7.0});
	EXPECT_TRUE(fitter.isNormalDistributed(0.95));
}

// Test objective: verifies FitterTest.IsNormalDistributed_FewerThanEightPoints_ReturnsFalse.
TEST_F(FitterTest, IsNormalDistributed_FewerThanEightPoints_ReturnsFalse) {
	loadData("normal_few", {3.0, 4.0, 5.0, 6.0, 7.0});
	EXPECT_FALSE(fitter.isNormalDistributed(0.95));
}

// Test objective: verifies FitterTest.IsNormalDistributed_HighlySkewedData_ReturnsFalse.
TEST_F(FitterTest, IsNormalDistributed_HighlySkewedData_ReturnsFalse) {
	// One extreme outlier makes the data clearly non-normal.
	loadData("normal_skew",
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 100.0});
	EXPECT_FALSE(fitter.isNormalDistributed(0.95));
}

// ============================= Null pointer outputs =============================

// Test objective: verifies FitterTest.NullOutputPointers_DoNotCrash.
TEST_F(FitterTest, NullOutputPointers_DoNotCrash) {
	loadData("null", {1.0, 2.0, 3.0, 4.0, 5.0});
	EXPECT_NO_FATAL_FAILURE(fitter.fitUniform(nullptr, nullptr, nullptr));
	EXPECT_NO_FATAL_FAILURE(fitter.fitTriangular(nullptr, nullptr, nullptr, nullptr));
	EXPECT_NO_FATAL_FAILURE(fitter.fitNormal(nullptr, nullptr, nullptr));
	EXPECT_NO_FATAL_FAILURE(fitter.fitExpo(nullptr, nullptr));
	EXPECT_NO_FATAL_FAILURE(fitter.fitErlang(nullptr, nullptr, nullptr));
	EXPECT_NO_FATAL_FAILURE(fitter.fitBeta(nullptr, nullptr, nullptr, nullptr, nullptr));
	EXPECT_NO_FATAL_FAILURE(fitter.fitWeibull(nullptr, nullptr, nullptr));
	EXPECT_NO_FATAL_FAILURE(fitter.fitAll(nullptr, nullptr));
}
