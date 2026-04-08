/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Sampler_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 14 de Agosto de 2018, 13:20
 */

#ifndef Sampler_IF_H
#define Sampler_IF_H

/*!
 * \brief Abstraction for random-number generation and probability-distribution sampling.
 *
 * Concrete implementations provide a base uniform pseudo-random generator and helper
 * methods to sample supported continuous and discrete probability distributions used
 * across the simulation kernel (e.g., parser functions and stochastic model logic).
 */
class Sampler_if {
public:
	virtual ~Sampler_if() = default;

	/*!
	 * \brief Encapsulates generator-specific configuration/state parameters.
	 *
	 * Implementations may derive this struct to expose seed, multiplier and other
	 * algorithm-specific controls required by a concrete RNG strategy.
	 */
	struct RNG_Parameters {
		virtual ~RNG_Parameters() = default;
	};
public: // RNG
	/*!
	 * \brief Generates a pseudo-random value uniformly distributed in the range [0,1).
	 * \return Base pseudo-random value used by the other distribution samplers.
	 */
	virtual double random() = 0;
public: // continuous probability distributions
	/*!
	 * \brief Samples a Beta-distributed value scaled to an arbitrary output interval.
	 * \param alpha Alpha shape parameter (>0).
	 * \param beta Beta shape parameter (>0).
	 * \param infLimit Lower bound of the output interval.
	 * \param supLimit Upper bound of the output interval.
	 * \return Sampled value within [infLimit, supLimit].
	 */
	virtual double sampleBeta(double alpha, double beta, double infLimit, double supLimit) = 0;
	/*! \brief Samples a standard Beta-distributed value in [0,1]. */
	virtual double sampleBeta(double alpha, double beta) = 0;
	/*! \brief Samples from an Erlang distribution using the provided mean and number of phases. */
	virtual double sampleErlang(double mean, int M, double offset = 0.0) = 0;
	/*! \brief Samples from an Exponential distribution with the provided mean. */
	virtual double sampleExponential(double mean, double offset = 0.0) = 0;
	//virtual double sampleGamma(double mean, double alpha, double offset=0.0) = 0;
	/*! \brief Samples from a Gamma distribution using the provided shape/scale parameters. */
	virtual double sampleGamma(double alpha, double beta, double offset = 0.0) = 0;
	/*! \brief Samples from a Gumbel distribution using mode and scale. */
	virtual double sampleGumbell(double mode, double scale) = 0;
	/*! \brief Samples from a Lognormal distribution parameterized by mean and standard deviation. */
	virtual double sampleLogNormal(double mean, double stddev, double offset = 0.0) = 0;
	/*! \brief Samples from a Normal distribution. */
	virtual double sampleNormal(double mean, double stddev) = 0;
	/*! \brief Samples from a Triangular distribution. */
	virtual double sampleTriangular(double min, double mode, double max) = 0;
	/*! \brief Samples from a continuous Uniform distribution in [min, max]. */
	virtual double sampleUniform(double min, double max) = 0;
	/*! \brief Samples from a Weibull distribution. */
	virtual double sampleWeibull(double alpha, double scale) = 0;
public: // discrete probability distributions
	//TODO: Poisson, si vous plait!!!!
	/*! \brief Samples from a Binomial distribution with \p trials and success probability \p p. */
	virtual double sampleBinomial(int trials, double p) = 0;
	/*! \brief Samples from a Bernoulli distribution (0/1). */
	virtual double sampleBernoulli(double p) = 0;
	/*! \brief Samples from a discrete distribution using a variadic list of probability/value pairs. */
	virtual double sampleDiscrete(double prob, double value, ...) = 0;
	/*! \brief Samples from a discrete distribution using probability/value arrays. */
	virtual double sampleDiscrete(double *prob, double *value, int size) = 0;
	/*! \brief Samples from a Geometric distribution with success probability \p p. */
	virtual double sampleGeometric(double p) = 0;
public:
	/*! \brief Updates the pseudo-random generator internal parameters. */
	virtual void setRNGparameters(RNG_Parameters* param) = 0;
	/*! \brief Returns the current pseudo-random generator parameters. */
	virtual RNG_Parameters* getRNGparameters() const = 0;
};

#endif /* Sampler_IF_H */
