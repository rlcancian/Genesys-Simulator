#ifndef MODELPERSISTENCEPARTIALLOADIMPL1_H
#define MODELPERSISTENCEPARTIALLOADIMPL1_H

#include "PersistenceDefaultImpl2.h"
#include "../model/Model.h"

class PersistencePartialLoadImpl1 : public PersistenceDefaultImpl2 {
public:
	explicit PersistencePartialLoadImpl1(Model* model);

public:
	bool load(std::string filename) override;

private:
	Model* _model;
};

#endif /* MODELPERSISTENCEPARTIALLOADIMPL1_H */
