#ifndef MODELPERSISTENCEPARTIALLOADIMPL1_H
#define MODELPERSISTENCEPARTIALLOADIMPL1_H

#include "ModelPersistenceDefaultImpl2.h"
#include "Model.h"

class ModelPersistencePartialLoadImpl1 : public ModelPersistenceDefaultImpl2 {
public:
	explicit ModelPersistencePartialLoadImpl1(Model* model);

public:
	bool load(std::string filename) override;

private:
	Model* _model;
};

#endif /* MODELPERSISTENCEPARTIALLOADIMPL1_H */
