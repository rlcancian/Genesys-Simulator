/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/*
 * File:   IntegratorDefaultImpl1.h
 * Author: rlcancian
 *
 * Created on 23 de novembro de 2021, 18:36
 */

#ifndef SOLVERDEFAULTIMPL1_IF_H
#define SOLVERDEFAULTIMPL1_IF_H

#include "Solver_if.h"

/**
 * @brief Legacy default implementation of Solver_if.
 *
 * Supported behavior:
 * - Numerical quadrature through composite Simpson 1/3 with a positive even
 *   number of subintervals.
 *
 * Unsupported behavior:
 * - The historical derivate() signatures do not expose a defined step-size
 *   contract and are not used by current repository clients. They fail
 *   explicitly instead of returning silent or undefined numerical results.
 *
 * Planned evolution:
 * - Split the historical interface into focused quadrature and ODE abstractions
 *   while preserving compatibility where a valid contract exists.
 */
class SolverDefaultImpl1 : public Solver_if {
public:
    SolverDefaultImpl1(double precision = 1e-6, unsigned int steps = 1e3);
    virtual ~SolverDefaultImpl1() = default;
public:
    virtual void setPrecision(double e) override;
    virtual double getPrecision() override;
    virtual void setMaxSteps(double steps) override;
    virtual double getMaxSteps() override;
    virtual double integrate(double min, double max, double (*f)(double, double), double p2) override;
    virtual double integrate(double min, double max, double (*f)(double, double, double), double p2, double p3) override;
    virtual double integrate(double min, double max, double (*f)(double, double, double, double), double p2, double p3, double p4) override;
    virtual double integrate(double min, double max, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) override;
    virtual double derivate(double initPoint, double initValue, double (*f)(double, double), double p2) override;
    virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double), double p2, double p3) override;
    virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double), double p2, double p3, double p4) override;
    virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) override;
private:
    double _precision;
    unsigned int _numSteps;
    double _stepSize;
};

#endif /* SOLVERDEFAULTIMPL1_IF_H */
