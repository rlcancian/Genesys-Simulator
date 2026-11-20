/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   IntegratorDefaultImpl1.cpp
 * Author: rlcancian
 *
 * Created on 23 de novembro de 2021, 18:36
 */

#include "SolverDefaultImpl1.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

unsigned int normalizedSimpsonStepCount(double min,
                                        double max,
                                        unsigned int configuredSteps,
                                        double precision) {
    unsigned int steps = std::max(2u, configuredSteps);
    const double intervalLength = std::abs(max - min);

    // Preserve the legacy direction of the precision adjustment: when the
    // configured spacing is finer than the requested precision, reduce the
    // number of subintervals. The result is then normalized for Simpson 1/3.
    if (intervalLength > 0.0 && precision > 0.0) {
        const double configuredSpacing = intervalLength / static_cast<double>(steps);
        if (configuredSpacing < precision) {
            const double requestedSteps = std::ceil(intervalLength / precision);
            if (requestedSteps < static_cast<double>(std::numeric_limits<unsigned int>::max())) {
                steps = std::max(2u, static_cast<unsigned int>(requestedSteps));
            }
        }
    }

    // Composite Simpson 1/3 requires a positive even number of subintervals.
    if ((steps % 2u) != 0u) {
        if (steps == std::numeric_limits<unsigned int>::max()) {
            --steps;
        } else {
            ++steps;
        }
    }

    return steps;
}

template<typename Function>
double integrateByCompositeSimpson(double min,
                                   double max,
                                   unsigned int configuredSteps,
                                   double precision,
                                   Function&& evaluate) {
    if (min == max) {
        return 0.0;
    }

    const unsigned int steps = normalizedSimpsonStepCount(
        min, max, configuredSteps, precision);
    const double h = (max - min) / static_cast<double>(steps);

    double sum = evaluate(min) + evaluate(max);
    for (unsigned int i = 1; i < steps; ++i) {
        const double x = min + static_cast<double>(i) * h;
        sum += ((i % 2u) == 0u ? 2.0 : 4.0) * evaluate(x);
    }

    return (h / 3.0) * sum;
}

[[noreturn]] void throwUnsupportedLegacyDerivative() {
    throw std::logic_error(
        "SolverDefaultImpl1::derivate has no defined step-size contract; "
        "use an explicit ODE solver instead");
}

} // namespace

SolverDefaultImpl1::SolverDefaultImpl1(double precision, unsigned int steps)
    : _precision(precision),
      _numSteps(steps),
      _stepSize(0.0) {
}

void SolverDefaultImpl1::setPrecision(double precision) {
    _precision = precision;
}

double SolverDefaultImpl1::getPrecision() {
    return _precision;
}

void SolverDefaultImpl1::setMaxSteps(double steps) {
    _numSteps = steps;
}

double SolverDefaultImpl1::getMaxSteps() {
    return _numSteps;
}

double SolverDefaultImpl1::integrate(double min,
                                     double max,
                                     double (*f)(double, double),
                                     double p2) {
    return integrateByCompositeSimpson(
        min, max, _numSteps, _precision,
        [f, p2](double x) { return f(x, p2); });
}

double SolverDefaultImpl1::integrate(double min,
                                     double max,
                                     double (*f)(double, double, double),
                                     double p2,
                                     double p3) {
    return integrateByCompositeSimpson(
        min, max, _numSteps, _precision,
        [f, p2, p3](double x) { return f(x, p2, p3); });
}

double SolverDefaultImpl1::integrate(double min,
                                     double max,
                                     double (*f)(double, double, double, double),
                                     double p2,
                                     double p3,
                                     double p4) {
    return integrateByCompositeSimpson(
        min, max, _numSteps, _precision,
        [f, p2, p3, p4](double x) { return f(x, p2, p3, p4); });
}

double SolverDefaultImpl1::integrate(
    double min,
    double max,
    double (*f)(double, double, double, double, double),
    double p2,
    double p3,
    double p4,
    double p5) {
    return integrateByCompositeSimpson(
        min, max, _numSteps, _precision,
        [f, p2, p3, p4, p5](double x) { return f(x, p2, p3, p4, p5); });
}

double SolverDefaultImpl1::derivate(double initPoint,
                                    double initValue,
                                    double (*f)(double, double),
                                    double p2) {
    (void)initPoint;
    (void)initValue;
    (void)f;
    (void)p2;
    throwUnsupportedLegacyDerivative();
}

double SolverDefaultImpl1::derivate(double initPoint,
                                    double initValue,
                                    double (*f)(double, double, double),
                                    double p2,
                                    double p3) {
    (void)initPoint;
    (void)initValue;
    (void)f;
    (void)p2;
    (void)p3;
    throwUnsupportedLegacyDerivative();
}

double SolverDefaultImpl1::derivate(
    double initPoint,
    double initValue,
    double (*f)(double, double, double, double),
    double p2,
    double p3,
    double p4) {
    (void)initPoint;
    (void)initValue;
    (void)f;
    (void)p2;
    (void)p3;
    (void)p4;
    throwUnsupportedLegacyDerivative();
}

double SolverDefaultImpl1::derivate(
    double initPoint,
    double initValue,
    double (*f)(double, double, double, double, double),
    double p2,
    double p3,
    double p4,
    double p5) {
    (void)initPoint;
    (void)initValue;
    (void)f;
    (void)p2;
    (void)p3;
    (void)p4;
    (void)p5;
    throwUnsupportedLegacyDerivative();
}
