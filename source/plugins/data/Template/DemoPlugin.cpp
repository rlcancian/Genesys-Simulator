/*
 * File:   DemoPlugin.cpp
 * Author: genesys
 *
 * Created on 3 de Junho de 2026
 */

#include "plugins/data/Template/DemoPlugin.h"
#include "kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DemoPlugin::GetPluginInformation;
}
#endif

ModelDataDefinition* DemoPlugin::NewInstance(Model* model, std::string name) {
	return new DemoPlugin(model, name);
}

ModelDataDefinition* DemoPlugin::LoadInstance(Model* model, PersistenceRecord *fields) {
	DemoPlugin* newElement = new DemoPlugin(model, "");
	try {
		newElement->_loadInstance(fields);
	} catch (...) {
	}
	return newElement;
}

PluginInformation* DemoPlugin::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DemoPlugin>(), &DemoPlugin::LoadInstance, &DemoPlugin::NewInstance);
	info->setCategory("Template");
	info->setDescriptionHelp("Demo plugin for dynamic parser testing. Adds a demo(x) function that returns x + 1.");
	return info;
}

DemoPlugin::DemoPlugin(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<DemoPlugin>(), name) {
}

std::string DemoPlugin::show() {
	return ModelDataDefinition::show();
}

bool DemoPlugin::_check(std::string& errorMessage) {
	return ModelDataDefinition::_check(errorMessage);
}

void DemoPlugin::_initBetweenReplications() {
}

ParserChangesInformation* DemoPlugin::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	changes->setTokens("%token <obj_t> fDEMO\n");
	changes->setFunctionProdutions(
		"    | fDEMO \"(\" expression \")\" { $$.valor = $3.valor + 1; }\n"
	);
	changes->setLexicalRules(
		"[dD][eE][mM][oO] {return yy::genesyspp_parser::make_fDEMO(obj_t(0, std::string(yytext)), loc);}\n"
	);
	return changes;
}
