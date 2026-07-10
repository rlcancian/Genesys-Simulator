#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "tools/Continuous/DiffusionMethodOfLinesSystem.h"
#include "tools/Continuous/OdeSolverFactory.h"

namespace {
using DMS = DiffusionMethodOfLinesSystem;

bool integrateOnce(const DMS& sys, std::vector<double>& field, double dt) {
	auto solver = OdeSolverFactory::create("DormandPrince54");
	std::vector<double> next(field.size(), 0.0);
	if (!solver->advance(sys, 0.0, dt, field.data(), next.data())) {
		return false;
	}
	field = next;
	return true;
}

} // namespace

TEST(DiffusionMolTest, OneDimensionalSineModeDecaysAtDiscreteRate) {
	DMS sys({41}, {0.05}, 0.2, DMS::Boundary::Dirichlet);
	ASSERT_TRUE(sys.isValid());
	std::vector<double> y(sys.totalNodes());
	sys.fillSineModes({2}, 1.0, y.data());
	const std::vector<double> y0 = y;
	const double lambda = sys.sineModeDecayRate({2});
	ASSERT_TRUE(integrateOnce(sys, y, 0.5));
	const double factor = std::exp(-lambda * 0.5);
	double maxError = 0.0;
	for (std::size_t i = 0; i < y.size(); ++i) {
		maxError = std::max(maxError, std::fabs(y[i] - y0[i] * factor));
	}
	EXPECT_LT(maxError, 1e-7);
}

TEST(DiffusionMolTest, NeumannConservesMass) {
	DMS sys({31, 31}, {0.1, 0.1}, 0.3, DMS::Boundary::Neumann);
	ASSERT_TRUE(sys.isValid());
	std::vector<double> y(sys.totalNodes());
	sys.fillGaussian(1.0, 0.3, y.data());
	const double initialMass = sys.totalMass(y.data());
	ASSERT_TRUE(integrateOnce(sys, y, 0.5));
	EXPECT_NEAR(sys.totalMass(y.data()), initialMass, 1e-9);
	EXPECT_LT(sys.maxValue(y.data()), 1.0);
}

TEST(DiffusionMolTest, RejectsInvalidConfiguration) {
	EXPECT_FALSE(DMS({2}, {0.1}, 0.1, DMS::Boundary::Dirichlet).isValid());
	EXPECT_FALSE(DMS({10}, {0.1, 0.1}, 0.1, DMS::Boundary::Dirichlet).isValid());
	EXPECT_FALSE(DMS({10}, {-0.1}, 0.1, DMS::Boundary::Dirichlet).isValid());
	EXPECT_EQ(DMS({2}, {0.1}, 0.1, DMS::Boundary::Dirichlet).dimension(), 0u);
}

TEST(DiffusionMolTest, RowMajorIndexingRoundTrips) {
	DMS sys({4, 5, 6}, {0.1, 0.1, 0.1}, 0.1, DMS::Boundary::Dirichlet);
	ASSERT_TRUE(sys.isValid());
	const std::vector<unsigned int> multiIndex = {2, 3, 4};
	const unsigned int node = sys.nodeIndex(multiIndex);
	EXPECT_EQ(sys.coordinate(node, 0), 2u);
	EXPECT_EQ(sys.coordinate(node, 1), 3u);
	EXPECT_EQ(sys.coordinate(node, 2), 4u);
}
