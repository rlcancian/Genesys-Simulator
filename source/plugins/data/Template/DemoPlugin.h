/*
 * File:   DemoPlugin.h
 * Author: genesys
 *
 * Created on 3 de Junho de 2026
 */

#ifndef DEMOPLUGIN_H
#define DEMOPLUGIN_H

#include "kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/ParserChangesInformation.h"

class DemoPlugin : public ModelDataDefinition {
public:
	DemoPlugin(Model* model, std::string name = "");
	virtual ~DemoPlugin() = default;
public:
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected:
	virtual ParserChangesInformation* _getParserChangesInformation() override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
};

#endif /* DEMOPLUGIN_H */
