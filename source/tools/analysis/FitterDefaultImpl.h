#ifndef FITTERDEFAULTIMPL_H
#define FITTERDEFAULTIMPL_H

#include "DatasetLoader.h"
#include "Fitter_if.h"

#include <functional>
#include <string>
#include <vector>

class FitterDefaultImpl : public Fitter_if {
public:
	FitterDefaultImpl() = default;
	virtual ~FitterDefaultImpl() = default;

	virtual bool isNormalDistributed(double confidencelevel) override;
	virtual void fitUniform(double* sqrerror, double* min, double* max) override;
	virtual void fitTriangular(double* sqrerror, double* min, double* mo, double* max) override;
	virtual void fitNormal(double* sqrerror, double* avg, double* stddev) override;
	virtual void fitExpo(double* sqrerror, double* avg1) override;
	virtual void fitErlang(double* sqrerror, double* avg, double* m) override;
	virtual void fitBeta(double* sqrerror, double* alpha, double* beta, double* infLimit, double* supLimit) override;
	virtual void fitWeibull(double* sqrerror, double* alpha, double* scale) override;
	virtual void fitAll(double* sqrerror, std::string* name) override;
	virtual FitSummary fitAllSummary() override;
	virtual void setDataFilename(std::string dataFilename) override;
	virtual bool setData(const std::vector<double>& data) override;
	virtual std::string getDataFilename() override;

private:
	bool _hasUsableDataset() const;
	void _setSse(double* sqrerror, const std::function<double(double)>& cdf);
	static double _betaPdfSafe(double x, double alpha, double beta);
	double _betaCdfScaled(double x, double alpha, double beta, double infLimit, double supLimit) const;
	double _weibullCdf(double x, double alpha, double scale) const;
	bool _estimateScaledBetaMoments(double* alpha, double* beta, double* infLimit, double* supLimit);
	bool _estimateWeibullShapeFromCv(double cv, double* shape) const;

private:
	std::string _dataFilename;
	DatasetLoader _dataset;
};

#endif /* FITTERDEFAULTIMPL_H */
