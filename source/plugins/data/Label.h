/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   Label.h
 * Author: rlcancian
 *
 * Created on 15 de janeiro de 2022, 10:13
 */

#ifndef LABEL_H
#define LABEL_H
#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/Entity.h"

class Label : public ModelDataDefinition {
public:
	Label(Model* model, std::string name = "");
	virtual ~Label() = default;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	virtual std::string show() override;
	void setLabel(std::string _label);
	std::string getLabel() const;
	void setEnterIntoLabelComponent(ModelComponent* enteringLabelComponent);
	ModelComponent* getEnterIntoLabelComponent() const;
	std::string getEnteringLabelComponentName() const;
	void sendEntityToLabelComponent(Entity* entity, double timeDelay);
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden 
	virtual bool _check(std::string& errorMessage) override;
	//virtual ParserChangesInformation* _getParserChangesInformation();
	//virtual void _initBetweenReplications();
	//virtual void _createInternalAndAttachedData();
private:
	std::string _label;
	std::string _enteringLabelComponentName;
	ModelComponent* _enteringLabelComponent = nullptr;
};

#endif /* LABEL_H */
