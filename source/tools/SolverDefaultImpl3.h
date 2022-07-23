#ifndef SOLVERDEFAULTIMPL3_IF_H
#define SOLVERDEFAULTIMPL3_IF_H

#include "solver_if.h"
#include <string>
#include <map>

class SolverDefaultImpl3 : public Solver_if {
public:
	SolverDefaultImpl3(double precision = 1e-6, unsigned int steps = 1e3);
	virtual ~SolverDefaultImpl3() = default;
public:
	virtual void setPrecision(double e);
	virtual double getPrecision();
	virtual void setMaxSteps(double steps);
	virtual double getMaxSteps();
	virtual double integrate(double min, double max, double (*f)(double, double), double p2);
	virtual double integrate(double min, double max, double (*f)(double, double, double), double p2, double p3);
	virtual double integrate(double min, double max, double (*f)(double, double, double, double), double p2, double p3, double p4);
	virtual double integrate(double min, double max, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5);
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double), double p2);
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double), double p2, double p3);
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double), double p2, double p3, double p4);
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5);
private:
    std::map<std::string, double> cachedResults;
	double _precision;
	unsigned int _numSteps;
	double _stepSize;
	//Variable* _variables;
	//Variable* _timeVariable;
};

#endif

