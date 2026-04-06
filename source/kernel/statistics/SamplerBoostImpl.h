/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SamplerBoostImpl.h
 * Author: rlcancian
 *
 * Created on 21 de Outubro de 2019, 17:24
 */

#ifndef SAMPLERBOOSTIMPL_H
#define SAMPLERBOOSTIMPL_H

#include "Sampler_if.h"
//#include <boost/random.hpp>

/*!
 * \brief Alternative sampler implementation intended to rely on Boost RNG/distributions.
 */
class SamplerBoostImpl : public Sampler_if {
public:

	/*! \brief Parameter bundle for Boost-based RNG configuration. */
	struct BoostImplRNG_Parameters : public RNG_Parameters {
		/*! \brief Generic parameter placeholder for Boost-backed configuration. */
		double param;
	};

public:
	/*! \brief Creates a Boost-based sampler instance. */
	SamplerBoostImpl();
	virtual ~SamplerBoostImpl() = default;
public: // probability distributions
	/*! \brief Generates one base uniform pseudo-random number. */
	virtual double random() override;
	/*! \brief Samples Beta in arbitrary interval \p [infLimit, supLimit]. */
    virtual double sampleBeta(double alpha, double beta, double infLimit, double supLimit) override;
	/*! \brief Samples standard Beta in \p [0,1]. */
    virtual double sampleBeta(double alpha, double beta) override;
	/*! \brief Samples Erlang distribution. */
    virtual double sampleErlang(double mean, int M, double offset = 0.0) override;
	/*! \brief Samples Exponential distribution. */
    virtual double sampleExponential(double mean, double offset = 0.0) override;
    //virtual double sampleGamma(double mean, double alpha, double offset=0.0);
	/*! \brief Samples Gamma distribution. */
    virtual double sampleGamma(double alpha, double beta, double offset = 0.0) override;
	/*! \brief Samples Gumbel distribution. */
    virtual double sampleGumbell(double mode, double scale) override;
	/*! \brief Samples Lognormal distribution. */
    virtual double sampleLogNormal(double mean, double stddev, double offset = 0.0) override;
	/*! \brief Samples Normal distribution. */
    virtual double sampleNormal(double mean, double stddev) override;
	/*! \brief Samples Triangular distribution. */
    virtual double sampleTriangular(double min, double mode, double max) override;
	/*! \brief Samples Uniform distribution in \p [min,max]. */
    virtual double sampleUniform(double min, double max) override;
	/*! \brief Samples Weibull distribution. */
    virtual double sampleWeibull(double alpha, double scale) override;
public: // discrete probability distributions
	/*! \brief Samples Binomial distribution. */
    virtual double sampleBinomial(int trials, double p) override;
	/*! \brief Samples Bernoulli distribution. */
    virtual double sampleBernoulli(double p) override;
	/*! \brief Samples arbitrary discrete distribution (variadic interface). */
    virtual double sampleDiscrete(double prob, double value, ...) override;
	/*! \brief Samples arbitrary discrete distribution (array interface). */
    virtual double sampleDiscrete(double *prob, double *value, int size) override;
	/*! \brief Samples Geometric distribution. */
    virtual double sampleGeometric(double p) override;
public:
	/*! \brief Reinitializes RNG state so the pseudo-random sequence restarts. */
	void reset();
public:
	virtual void setRNGparameters(Sampler_if::RNG_Parameters* param) override;
	virtual RNG_Parameters* getRNGparameters() const override;
private:
	//boost::random::mt19937 _gen;
};

#endif /* SAMPLERBOOSTIMPL_H */
