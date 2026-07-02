// Spike test for TASK-01 (distributed-simulation): confirms that a per-run RNG seed
// can be set through the public model -> parser -> sampler path WITHOUT touching the
// simulation kernel logic, and that the seed deterministically controls the RNG stream
// (same seed reproduces the sequence; different seeds diverge). This is the foundation
// for partitioning replications across workers with distinct, reproducible seeds.

#include <gtest/gtest.h>

#include <vector>

#include "kernel/simulator/Simulator.h"
#include "../../kernel/simulator/model/Model.h"
#include "../../kernel/simulator/Parser_if.h"
#include "../../kernel/statistics/Sampler_if.h"
#include "../../kernel/statistics/SamplerDefaultImpl1.h"

namespace {

// Owns the parameter objects handed to the sampler. The sampler keeps a non-owning
// pointer (see SamplerDefaultImpl1::setRNGparameters), so the caller must keep them
// alive for as long as the sampler is used and release them afterwards.
class SeededSamplerFixture : public ::testing::Test {
protected:
	Sampler_if* samplerFor(Model* model) {
		Parser_if* parser = model->getParser();
		EXPECT_NE(parser, nullptr);
		return parser->getSampler();
	}

	void applySeed(Sampler_if* sampler, uint32_t seed) {
		auto* params = new SamplerDefaultImpl1::DefaultImpl1RNG_Parameters();
		params->seed = seed;
		_ownedParams.push_back(params);
		sampler->setRNGparameters(params);
	}

	std::vector<double> drawSequence(Sampler_if* sampler, int count) {
		std::vector<double> values;
		values.reserve(count);
		for (int i = 0; i < count; ++i) {
			values.push_back(sampler->random());
		}
		return values;
	}

	void TearDown() override {
		for (auto* params : _ownedParams) {
			delete params;
		}
		_ownedParams.clear();
	}

private:
	std::vector<SamplerDefaultImpl1::DefaultImpl1RNG_Parameters*> _ownedParams;
};

} // namespace

TEST_F(SeededSamplerFixture, ShouldExposeParserAndSamplerWhenModelIsCreated) {
	// Arrange
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	// Act
	Parser_if* parser = model->getParser();
	Sampler_if* sampler = parser != nullptr ? parser->getSampler() : nullptr;

	// Assert
	ASSERT_NE(parser, nullptr);
	ASSERT_NE(sampler, nullptr);
}

TEST_F(SeededSamplerFixture, ShouldReproduceSequenceWhenSameSeedIsApplied) {
	// Arrange
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	Sampler_if* sampler = samplerFor(model);
	ASSERT_NE(sampler, nullptr);

	// Act
	applySeed(sampler, 12345u);
	std::vector<double> first = drawSequence(sampler, 8);
	applySeed(sampler, 12345u);
	std::vector<double> second = drawSequence(sampler, 8);

	// Assert
	EXPECT_EQ(first, second);
}

TEST_F(SeededSamplerFixture, ShouldDivergeSequenceWhenDifferentSeedsAreApplied) {
	// Arrange
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	Sampler_if* sampler = samplerFor(model);
	ASSERT_NE(sampler, nullptr);

	// Act
	applySeed(sampler, 1u);
	std::vector<double> seqA = drawSequence(sampler, 8);
	applySeed(sampler, 2u);
	std::vector<double> seqB = drawSequence(sampler, 8);

	// Assert
	EXPECT_NE(seqA, seqB);
}
