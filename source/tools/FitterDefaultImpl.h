#ifndef FITTERDEFAULTIMPL_H
#define FITTERDEFAULTIMPL_H

#include "Fitter_if.h"

/**
 * @brief Planned default concrete fitter implementation.
 *
 * Implementation status:
 * - Structural placeholder only.
 * - Functional fitting logic is intentionally deferred to a later phase.
 *
 * Failure behavior:
 * - Methods currently return controlled placeholder outputs that indicate
 *   unavailable fitting results.
 */
class FitterDefaultImpl : public Fitter_if {
public:
	FitterDefaultImpl() = default;
	virtual ~FitterDefaultImpl() = default;

public:
	virtual bool isNormalDistributed(double confidencelevel) override {
		(void) confidencelevel;
		return false;
	}

	virtual void fitUniform(double *sqrerror, double *min, double *max) override {
		_placeHolder3(sqrerror, min, max);
	}

	virtual void fitTriangular(double *sqrerror, double *min, double *mo, double *max) override {
		_placeHolder4(sqrerror, min, mo, max);
	}

	virtual void fitNormal(double *sqrerror, double *avg, double *stddev) override {
		_placeHolder3(sqrerror, avg, stddev);
	}

	virtual void fitExpo(double *sqrerror, double *avg1) override {
		_placeHolder2(sqrerror, avg1);
	}

	virtual void fitErlang(double *sqrerror, double *avg, double *m) override {
		_placeHolder3(sqrerror, avg, m);
	}

	virtual void fitBeta(double *sqrerror, double *alpha, double *beta, double *infLimit, double *supLimit) override {
		_placeHolder5(sqrerror, alpha, beta, infLimit, supLimit);
	}

	virtual void fitWeibull(double *sqrerror, double *alpha, double *scale) override {
		_placeHolder3(sqrerror, alpha, scale);
	}

	virtual void fitAll(double *sqrerror, std::string *name) override {
		if (sqrerror != nullptr) {
			*sqrerror = -1.0;
		}
		if (name != nullptr) {
			*name = "not-implemented";
		}
	}

	virtual void setDataFilename(std::string dataFilename) override {
		_dataFilename = dataFilename;
	}

	virtual std::string getDataFilename() override {
		return _dataFilename;
	}

private:
	void _placeHolder2(double* value1, double* value2) {
		if (value1 != nullptr) {
			*value1 = -1.0;
		}
		if (value2 != nullptr) {
			*value2 = 0.0;
		}
	}

	void _placeHolder3(double* value1, double* value2, double* value3) {
		_placeHolder2(value1, value2);
		if (value3 != nullptr) {
			*value3 = 0.0;
		}
	}

	void _placeHolder4(double* value1, double* value2, double* value3, double* value4) {
		_placeHolder3(value1, value2, value3);
		if (value4 != nullptr) {
			*value4 = 0.0;
		}
	}

	void _placeHolder5(double* value1, double* value2, double* value3, double* value4, double* value5) {
		_placeHolder4(value1, value2, value3, value4);
		if (value5 != nullptr) {
			*value5 = 0.0;
		}
	}

private:
	std::string _dataFilename = "";
};

#endif /* FITTERDEFAULTIMPL_H */
