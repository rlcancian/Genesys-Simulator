/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Integrator_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 14 de Agosto de 2018, 13:54
 */

#ifndef SOLVER_IF_H
#define SOLVER_IF_H

/**
 * @brief Legacy compatibility interface for numerical routines.
 *
 * Architectural role:
 * - Historically groups two responsibilities:
 *   1) Numerical quadrature/integration.
 *   2) Numerical advancement/derivation for ODE-like flows.
 *
 * Mathematical meaning:
 * - Function signatures use the first functional parameter as time @c t.
 * - Parameters p2..p5 represent model parameters passed through old-style
 *   function pointers.
 *
 * Limitations:
 * - The p2/p3/p4/p5 model reflects a historical constraint and is not the
 *   preferred long-term abstraction.
 *
 * Planned evolution:
 * - Split into focused interfaces (quadrature, root finding, ODE systems and
 *   ODE solvers) while preserving this API for backward compatibility.
 */
class Solver_if {
public:
	virtual ~Solver_if() = default;
	virtual void setPrecision(double e) = 0;
	virtual double getPrecision() = 0;
	virtual void setMaxSteps(double steps) = 0;
	virtual double getMaxSteps() = 0;
	virtual double integrate(double min, double max, double (*f)(double, double), double p2) = 0;
	virtual double integrate(double min, double max, double (*f)(double, double, double), double p2, double p3) = 0;
	virtual double integrate(double min, double max, double (*f)(double, double, double, double), double p2, double p3, double p4) = 0;
	virtual double integrate(double min, double max, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) = 0;
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double), double p2) = 0;
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double), double p2, double p3) = 0;
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double), double p2, double p3, double p4) = 0;
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) = 0;
};

#endif /* SOLVER_IF_H */
