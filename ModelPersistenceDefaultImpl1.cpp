/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelPersistenceDefaultImpl1.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 20 de Maio de 2019, 20:41
 */

#include "ModelPersistenceDefaultImpl1.h"
#include <fstream>
#include <ctime>
#include <regex>
#include <cassert>
#include "ModelComponent.h"
#include "Simulator.h"
#include "Traits.h"
#include "Counter.h"

//using namespace GenesysKernel;

ModelPersistenceDefaultImpl1::ModelPersistenceDefaultImpl1(Model* model) {
	_model = model;
}

std::map<std::string, std::string>* ModelPersistenceDefaultImpl1::_getSimulatorInfoFieldsToSave() {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	fields->emplace("typename", "SimulatorInfo");
	fields->emplace("name", "\"" + _model->getParentSimulator()->name() + "\"");
	fields->emplace("version", "\"" + _model->getParentSimulator()->version() + "\"");
	return fields;
}

bool ModelPersistenceDefaultImpl1::save(std::string filename) {
	_model->getTracer()->trace(Util::TraceLevel::toolInternal, "Saving file \"" + filename + "\"");
	Util::IncIndent();
	std::list<std::string> *simulInfosToSave, *modelInfosToSave, *modelElementsToSave, *modelComponentsToSave;
	{
		//bool res = true;
		std::map<std::string, std::string>* fields;
		fields = _getSimulatorInfoFieldsToSave();
		simulInfosToSave = _adjustFieldsToSave(fields);
		// save model own infos
		fields = _model->getInfos()->saveInstance();
		modelInfosToSave = _adjustFieldsToSave(fields);
		// save infras
		modelElementsToSave = new std::list<std::string>();
		std::list<std::string>* elementTypenames = _model->getElements()->getElementClassnames();
		const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();
		for (std::list<std::string>::iterator itTypenames = elementTypenames->begin(); itTypenames != elementTypenames->end(); itTypenames++) {
			if ((*itTypenames) != Util::TypeOf<StatisticsCollector>() && (*itTypenames) != UtilTypeOfCounter) { // STATISTICSCOLLECTR and COUNTERs do NOT need to be saved
				List<ModelElement*>* infras = _model->getElements()->getElementList((*itTypenames));
				_model->getTracer()->trace(Util::TraceLevel::toolDetailed, "Writing elements of type \"" + (*itTypenames) + "\":");
				Util::IncIndent();
				{
					for (std::list<ModelElement*>::iterator it = infras->list()->begin(); it != infras->list()->end(); it++) {
						_model->getTracer()->trace(Util::TraceLevel::toolDetailed, "Writing " + (*itTypenames) + " \"" + (*it)->getName() + "\"");
						fields = (*it)->SaveInstance((*it));
						Util::IncIndent();
						modelElementsToSave->merge(*_adjustFieldsToSave(fields));
						Util::DecIndent();
					}
				}
				Util::DecIndent();
			}
		}
		// save components
		_model->getTracer()->trace(Util::TraceLevel::toolDetailed, "Writing components\":");
		//List<ModelComponent*>* components = this->_model->getComponents();
		modelComponentsToSave = new std::list<std::string>();
		Util::IncIndent();
		{
			for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
				fields = (*it)->SaveInstance((*it));
				Util::IncIndent();
				modelComponentsToSave->merge(*_adjustFieldsToSave(fields));
				Util::DecIndent();
			}
		}
		Util::DecIndent();
		// SAVE FILE
		_model->getTracer()->trace(Util::TraceLevel::toolDetailed, "Saving file");
		Util::IncIndent();
		{
			// open file
			std::ofstream savefile;
			savefile.open(filename, std::ofstream::out);
			savefile << "# Genesys simulation model " << std::endl;
			time_t now = time(0);
			char* dt = ctime(&now);
			savefile << "# Last saved on " << dt;
			savefile << "# simulator infos" << std::endl;
			_saveContent(simulInfosToSave, &savefile);
			savefile << "# model infos" << std::endl;
			_saveContent(modelInfosToSave, &savefile);
			savefile << "# model elements" << std::endl;
			_saveContent(modelElementsToSave, &savefile);
			savefile << "# model components" << std::endl;
			_saveContent(modelComponentsToSave, &savefile);
			savefile.close();
		}
		Util::DecIndent();
	}
	Util::DecIndent();
	this->_hasChanged = false;
	return true; // \todo: check if save really saved successfully
}

void ModelPersistenceDefaultImpl1::_saveContent(std::list<std::string>* content, std::ofstream* file) {
	for (std::list<std::string>::iterator it = content->begin(); it != content->end(); it++) {
		*file << (*it) << std::endl << std::endl;
	}
}

bool ModelPersistenceDefaultImpl1::_loadFields(std::string line) {
	//std::regex regex{R"([=]+)"}; // split on space R"([\s]+)" \todo: HOW SEPARATOR WITH MORE THAN ONE CHAR
	_model->getTracer()->trace(Util::TraceLevel::everythingMostDetailed, line);
	bool res = true;
	std::regex regex{R"([\s]+)"}; // split on " " \todo Should be _lineseparator
	std::sregex_token_iterator tit{line.begin(), line.end(), regex, -1};
	std::list<std::string> lstfields{tit,{}};
	// for each field, separate key and value and form a map
	try {
		std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
		regex = {R"([=]+)"};
		std::vector<std::string> veckeyval; //{it,{}};
		unsigned int i = 0;
		for (std::list<std::string>::iterator it = lstfields.begin(); it != lstfields.end(); it++) {
			//std::cout << (*it) << std::endl;
			tit = {(*it).begin(), (*it).end(), regex, -1};
			veckeyval = {tit,{}};
			trim((veckeyval[0]));
			if (veckeyval[0] != "") { // it should always be, rigth? \todo case for assert?
				if (veckeyval.size() > 1) {
					trim((veckeyval[1]));
					veckeyval[1] = this->_convertLineseparatorReplacementBacktoLineseparator(veckeyval[1]);
					fields->emplace(veckeyval[0], veckeyval[1]);
				} else {
					if (i == 0) {
						fields->emplace("id", veckeyval[0]);
					} else if (i == 1) {
						fields->emplace("typename", veckeyval[0]);
					} else {
						fields->emplace(veckeyval[0], "");
					}
				}
			}
			i++;
		}
		// now the map<str,str> is ready. Look for the right class to load it
		Util::IncIndent();
		{
			std::string thistypename = (*fields->find("typename")).second;
			_model->getTracer()->trace(Util::TraceLevel::toolInternal, "loading " + thistypename + "");
			if (thistypename == "SimulatorInfo") {
				this->_loadSimulatorInfoFields(fields);
			} else if (thistypename == "ModelInfo") {
				_model->getInfos()->loadInstance(fields);
			} else {
				// this should be a ModelComponent or ModelElement.
				//std::string thistypename = (*fields->find("typename")).second;
				ModelElement* newTemUselessElement = ModelElement::LoadInstance(_model, fields, false);
				if (newTemUselessElement != nullptr) {
					newTemUselessElement->~ModelElement();
					Plugin* plugin = this->_model->getParentSimulator()->getPlugins()->find(thistypename);
					if (plugin != nullptr) {
						res = plugin->loadAndInsertNew(_model, fields);
						// save fields for components, in order to allow to connect components after all of them have been loaded
						if (res && plugin->getPluginInfo()->isComponent()) {
							_componentFields->insert(_componentFields->end(), fields);
							//_model->getTraceManager()->trace(Util::TraceLevel::errors, "Inserindo fields do componente "+plugin->getPluginInfo()->getPluginTypename());
						}
					} else {
						_model->getTracer()->trace(Util::TraceLevel::errorFatal, "Error loading file: Could not identity typename \"" + thistypename + "\"");
						res = false;
					}
				} else {
					_model->getTracer()->trace(Util::TraceLevel::errorFatal, "Error loading file: Could not identity typename \"" + thistypename + "\"");
					res = false;
				}

			}
		}
		Util::DecIndent();
	} catch (...) {

	}
	return res;
}

void ModelPersistenceDefaultImpl1::_loadSimulatorInfoFields(std::map<std::string, std::string>* fields) {
}

bool ModelPersistenceDefaultImpl1::load(std::string filename) {
	//std::list<Plugin*> plugins = this->_model->getParent()->getPlugins();
	//plugins->front()->
	//return false;
	bool res = true;
	_model->getTracer()->trace(Util::TraceLevel::toolInternal, "Loading file \"" + filename + "\"");
	Util::IncIndent();
	_componentFields->clear();
	{
		std::ifstream modelFile;
		std::string inputLine;
		try {
			modelFile.open(filename);
			while (getline(modelFile, inputLine) && res) {
				//trim(&inputLine);
				if (inputLine.substr(0, 1) != "#" && !inputLine.empty()) {
					//Util::IncIndent();
					res &= _loadFields(inputLine);
					//Util::DecIndent();
				}
			}
			modelFile.close();
		} catch (...) {
			_model->getTracer()->trace(Util::TraceLevel::errorFatal, "Error loading file \"" + filename + "\"");
		}
	}
	// check if something was loaded
	res &= (_model->getComponents()->getNumberOfComponents() > 0) & (_model->getElements()->getNumberOfElements() > 0);
	if (res) {
		//
		// CONNECT LOADED COMPONENTS (must wait for all components to be loaded so they can be connected)
		//
		ComponentManager* cm = _model->getComponents();
		_model->getTracer()->trace(Util::TraceLevel::simulatorDetailed, "Connecting loaded components");
		Util::IncIndent();
		{
			for (std::list<std::map<std::string, std::string>*>::iterator it = _componentFields->begin(); it != _componentFields->end(); it++) {
				std::map<std::string, std::string>* fields = (*it);
				// find the component
				ModelComponent* thisComponent = nullptr;
				Util::identification thisId = std::stoi((*fields->find("id")).second);
				for (std::list<ModelComponent*>::iterator itcomp = cm->begin(); itcomp != cm->end(); itcomp++) {
					if ((*itcomp)->getId() == thisId) {
						thisComponent = (*itcomp);
						break; // end inner for loop
					}
				}
				assert(thisComponent != nullptr);

				// find the next components connected with this one
				unsigned short nextSize = 1;
				if (*fields->find("nextSize") != *fields->end()) { // found nextSize
					nextSize = std::stoi((*fields->find("nextSize")).second);
				}
				for (unsigned short i = 0; i < nextSize; i++) {
					Util::identification nextId = std::stoi((*fields->find("nextId" + std::to_string(i))).second);
					unsigned short nextInputNumber = 0; // default value if it is not found bellow
					if (fields->find("nextInputNumber" + std::to_string(i)) != fields->end())
						nextInputNumber = std::stoi((*fields->find("nextInputNumber" + std::to_string(i))).second);
					ModelComponent* nextComponent = nullptr;
					for (std::list<ModelComponent*>::iterator itcomp = cm->begin(); itcomp != cm->end(); itcomp++) {// connect the components
						if ((*itcomp)->getId() == nextId) { // connect the components
							nextComponent = (*itcomp);
							thisComponent->getNextComponents()->insert(nextComponent, nextInputNumber);
							_model->getTracer()->trace(Util::TraceLevel::toolDetailed, thisComponent->getName() + "<" + std::to_string(i) + ">" + " -> " + nextComponent->getName() + "<" + std::to_string(nextInputNumber) + ">");
							break;
						}
					}
					assert(nextComponent != nullptr);
				}

			}
		}
		Util::DecIndent();
		_model->getTracer()->trace(Util::TraceLevel::simulatorInternal, "File successfully loaded with " + std::to_string(_model->getComponents()->getNumberOfComponents()) + " components and " + std::to_string(_model->getElements()->getNumberOfElements()) + " elements");
	}
	Util::DecIndent();
	if (res) {
		_hasChanged = false;
	}
	return res;
}

std::string ModelPersistenceDefaultImpl1::_convertLineseparatorReplacementBacktoLineseparator(std::string str) {
	size_t index = str.find(_fieldseparatorReplacement, 0);
	while (index != std::string::npos) {
		str.replace(index, _fieldseparatorReplacement.length(), _fieldseparator);
		/* Advance index forward so the next iteration doesn't pick it up as well. */
		index = str.find(_fieldseparatorReplacement, index + _fieldseparator.length());
	}
	return str;
}

std::string ModelPersistenceDefaultImpl1::_convertLineseparatorToLineseparatorReplacement(std::string str) {
	size_t index = str.find(_fieldseparator, 0);
	while (index != std::string::npos) {
		str.replace(index, _fieldseparator.length(), _fieldseparatorReplacement);
		/* Advance index forward so the next iteration doesn't pick it up as well. */
		index = str.find(_fieldseparator, index + _fieldseparatorReplacement.length());
	}
	return str;
}

std::list<std::string>* ModelPersistenceDefaultImpl1::_adjustFieldsToSave(std::map<std::string, std::string>* fields) {
	std::list<std::string>* newList = new std::list<std::string>();
	std::string newStr, strV2003, idV2003, typenameV2003; //, strV210329;
	idV2003 = "0";
	std::string attrValue;
	for (std::map<std::string, std::string>::iterator it = fields->begin(); it != fields->end(); it++) {
		//newStr += (*it).first + "=" + (*it).second + this->_linefieldseparator;
		if ((*it).first == "id")
			idV2003 = (*it).second;
		else if ((*it).first == "typename")
			typenameV2003 = (*it).second;
		else {
			// version V210329: (*it).second should NEVER contain _linefieldseparator. So, replace it by _linefieldseparatorReplacement
			attrValue = _convertLineseparatorToLineseparatorReplacement((*it).second);
			strV2003 += (*it).first + "=" + attrValue + _fieldseparator;
		}
	}
	while (idV2003.length() < 3)
		idV2003 += _fieldseparator;
	while (typenameV2003.length() < 15)
		typenameV2003 += _fieldseparator;
	strV2003 = idV2003 + _fieldseparator + typenameV2003 + _fieldseparator + strV2003;
	_model->getTracer()->trace(Util::TraceLevel::toolDetailed, strV2003); //newStr
	newList->push_back(strV2003); //newStr
	return newList;
}

bool ModelPersistenceDefaultImpl1::hasChanged() {
	return _hasChanged;
}
