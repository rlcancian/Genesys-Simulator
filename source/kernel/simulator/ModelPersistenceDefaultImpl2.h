#ifndef MODELPERSISTENCEDEFAULTIMPL_H
#define MODELPERSISTENCEDEFAULTIMPL_H

#include "ModelPersistence_if.h"
#include "Model.h"


class ModelPersistenceDefaultImpl2 : public ModelPersistence_if {
public:
	ModelPersistenceDefaultImpl2(Model* model);

public: // ModelPersistence_if interface
	bool save(std::string filename) override;
	bool load(std::string filename) override;
	bool hasChanged() override;
	bool getOption(ModelPersistence_if::Options option) override;
	void setOption(ModelPersistence_if::Options option, bool value) override;
	std::string getFormatedField(PersistenceRecord *fields) override;

private:
	Model* _model;
	unsigned short _options{0};
	bool _dirty{false};
};

#endif /* MODELPERSISTENCEDEFAULTIMPL_H */
