/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SamplerDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 2 de Agosto de 2018, 01:10
 */

#ifndef SAMPLERDEFAULTIMPL1_H
#define SAMPLERDEFAULTIMPL1_H

#include "Sampler_if.h"
//namespace GenesysKernel {

	class SamplerDefaultImpl1 : public Sampler_if {
	public:

		struct DefaultImpl1RNG_Parameters : public RNG_Parameters {
			unsigned int seed = 666;
			unsigned int module = 2147483647;
			unsigned int multiplier = 950706376;
			~DefaultImpl1RNG_Parameters() = default;
		};
	public:
		SamplerDefaultImpl1();
		virtual ~SamplerDefaultImpl1() = default;
	public: // probability distributions
		virtual double random();
		virtual double sampleBeta(double alpha, double beta, double infLimit, double supLimit);
		virtual double sampleDiscrete(double acumProb, double value, ...);
		virtual double sampleErlang(double mean, int M);
		virtual double sampleExponential(double mean);
		virtual double sampleGamma(double mean, double alpha);
		virtual double sampleLogNormal(double mean, double stddev);
		virtual double sampleNormal(double mean, double stddev);
		virtual double sampleTriangular(double min, double mode, double max);
		virtual double sampleUniform(double min, double max);
		virtual double sampleWeibull(double alpha, double scale);
		virtual double sampleGumbellInv(double mode, double scale);
		virtual double sampleGumbell(double mode, double scale);
		virtual double sampleChiSqrt(double degrees);
    virtual double sampleGammaPDF(int alpha, int beta);
    virtual double sampleBetaPDF(int alpha, int beta);
    virtual double sampleBinomial(int trials);
    virtual double sampleGeometric(double k, double p);

	public:
		void reset(); ///< reinitialize seed and other parameters so (pseudo) random number sequence will be generated again.
	public:
		virtual void setRNGparameters(RNG_Parameters* param);
		virtual RNG_Parameters* getRNGparameters() const;
	private:
        double gammaFunction(int n);
        double betaFunction(int x, int y);
		double _gammaJonk(double alpha);
	private:
		RNG_Parameters* _param = new DefaultImpl1RNG_Parameters();
		unsigned int _xi;
		//bool _normalflag;
	};
//namespace\\}
#endif /* SAMPLERDEFAULTIMPL1_H */

