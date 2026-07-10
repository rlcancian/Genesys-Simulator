#ifndef MODELPERSISTENCEDEFAULTIMPL_H
#define MODELPERSISTENCEDEFAULTIMPL_H

#include "Persistence_if.h"
#include "../model/Model.h"

class PersistenceDefaultImpl2 : public Persistence_if {
public:
	PersistenceDefaultImpl2(Model* model);

public: // Persistence_if interface
	bool save(std::string filename) override;
	bool load(std::string filename) override;
	bool hasChanged() override;
	void setHasChanged(bool hasChanged) override;
	bool getOption(Persistence_if::Options option) override;
	void setOption(Persistence_if::Options option, bool value) override;
	std::string getFormatedField(PersistenceRecord *fields) override;

private:
	Model* _model;
	unsigned short _options{0};
	bool _dirty{false};
};

#endif /* MODELPERSISTENCEDEFAULTIMPL_H */
