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
#include <iterator>

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
	SaveField(fields, "typename", "SimulatorInfo");
	SaveField(fields, "name", _model->getParentSimulator()->getName());
	SaveField(fields, "versionNumber", _model->getParentSimulator()->getVersionNumber());
	//SaveField(fields, "version", _model->getParentSimulator()->getVersion());
	return fields;
}

bool ModelPersistenceDefaultImpl1::save(std::string filename) {
	_model->getTracer()->trace(Util::TraceLevel::L6_internal, "Saving file \"" + filename + "\"");
	Util::IncIndent();
	std::list<std::string> *simulatorInfosToSave, *simulationInfosToSave, *modelInfosToSave, *modelElementsToSave, *modelComponentsToSave;
	{
		//bool res = true;
		std::map<std::string, std::string>* fields;
		fields = _getSimulatorInfoFieldsToSave();
		simulatorInfosToSave = _adjustFieldsToSave(fields);
		// save model own infos
		fields = _model->getInfos()->saveInstance();
		modelInfosToSave = _adjustFieldsToSave(fields);
		// save model own infos
		// \todo save modelSimulation fields (breakpoints)
		fields = _model->getSimulation()->saveInstance();
		simulationInfosToSave = _adjustFieldsToSave(fields);
		// save infras
		modelElementsToSave = new std::list<std::string>();
		std::list<std::string>* elementTypenames = _model->getElements()->getElementClassnames();
		const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();
		for (std::list<std::string>::iterator itTypenames = elementTypenames->begin(); itTypenames != elementTypenames->end(); itTypenames++) {
			if ((*itTypenames) != Util::TypeOf<StatisticsCollector>() && (*itTypenames) != UtilTypeOfCounter) { // STATISTICSCOLLECTR and COUNTERs do NOT need to be saved
				List<ModelElement*>* infras = _model->getElements()->getElementList((*itTypenames));
				_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Writing elements of type \"" + (*itTypenames) + "\":");
				Util::IncIndent();
				{
					for (std::list<ModelElement*>::iterator it = infras->list()->begin(); it != infras->list()->end(); it++) {
						_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Writing " + (*itTypenames) + " \"" + (*it)->getName() + "\"");
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
		_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Writing components\":");
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
		_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Saving file");
		Util::IncIndent();
		{
			// open file
			std::ofstream savefile;
			savefile.open(filename, std::ofstream::out);
			//savefile << "# Genesys simulation model " << std::endl;
			time_t now = time(0);
			char* dt = ctime(&now);
			//savefile << "# Last saved on " << dt;
			//savefile << "# simulator infos" << std::endl;
			_saveContent(simulatorInfosToSave, &savefile);
			//savefile << "# model infos" << std::endl;
			_saveContent(modelInfosToSave, &savefile);
			//savefile << "# simulation infos / experimental design" << std::endl;
			_saveContent(simulationInfosToSave, &savefile);
			//savefile << "#" << std::endl;
			//savefile << "# model elements" << std::endl;
			_saveContent(modelElementsToSave, &savefile);
			//savefile << "#" << std::endl;
			//savefile << "# model components" << std::endl;
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
		*file << (*it) << std::endl; // << std::endl;
	}
}

bool ModelPersistenceDefaultImpl1::_loadFields(std::string line) {
	//std::regex regex{R"([=]+)"}; // split on space R"([\s]+)" \todo: HOW SEPARATOR WITH MORE THAN ONE CHAR
	_model->getTracer()->trace(Util::TraceLevel::L8_mostDetailed, line);
	bool res = true;
	// replaces every "quoted" string by {stringX}
	std::regex regexQuoted("\"([^\"]*)\"");
	auto matches_begin = std::sregex_iterator(line.begin(), line.end(), regexQuoted);
	auto matches_end = std::sregex_iterator();
	std::list<std::string>* strings = new std::list<std::string>();
	for (std::sregex_iterator it = matches_begin; it != matches_end; it++) {
		std::string match_str = (*it).str();
		strings->insert(strings->end(), match_str);
	}
	unsigned short i = 0;
	for (std::list<std::string>::iterator it = strings->begin(); it != strings->end(); it++, i++) {
		std::string match_str = (*it);
		unsigned int pos = line.find(match_str, 0);
		line.replace(pos, match_str.length(), "{string" + std::to_string(i) + "}");
	}
	//
	//i = 0;
	//for (std::list<std::string>::iterator it = strings->begin(); it != strings->end(); it++, i++) {
	//	std::cout << "{string" << std::to_string(i) << "}=" << (*it) << std::endl;
	//}
	std::regex regex{R"([\s]+)"}; // split on " " \todo Should be _lineseparator
	std::sregex_token_iterator tit{line.begin(), line.end(), regex, -1};
	std::list<std::string> lstfields{tit,{}};
	// for each field, separate key and value and form a map
	try {
		std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
		regex = {R"([=]+)"};
		std::vector<std::string> veckeyval; //{it,{}};
		unsigned int i = 0;
		for (std::list<std::string>::iterator it = lstfields.begin(); it != lstfields.end(); it++, i++) {
			//std::cout << (*it) << std::endl;
			tit = {(*it).begin(), (*it).end(), regex, -1};
			veckeyval = {tit,{}};
			veckeyval[0] = trim((veckeyval[0]));
			if (veckeyval[0] != "") { // it should always be, rigth? \todo case for assert?
				if (veckeyval.size() > 1) {
					veckeyval[1] = trim((veckeyval[1]));
					if (veckeyval[1].substr(0, 1) == "\"" && veckeyval[1].substr(veckeyval[1].length() - 1, 1) == "\"") { // remove ""
						veckeyval[1] = veckeyval[1].substr(1, veckeyval[1].length() - 2);
					}
					veckeyval[1] = this->_convertLineseparatorReplacementBacktoLineseparator(veckeyval[1]);
					fields->emplace(veckeyval[0], veckeyval[1]);
				} else {
					if (i == 0) {
						fields->emplace("id", veckeyval[0]);
					} else if (i == 1) {
						fields->emplace("typename", veckeyval[0]);
					} else if (i == 2) {
						///if (veckeyval[0].substr(0, 1) == "\"" && veckeyval[0].substr(veckeyval[0].length() - 1, 1) == "\"") { // remove ""
						//	veckeyval[0] = veckeyval[0].substr(1, veckeyval[0].length() - 2);
						//}
						//veckeyval[0] = this->_convertLineseparatorReplacementBacktoLineseparator(veckeyval[0]);
						fields->emplace("name", veckeyval[0]);
					} else {
						fields->emplace(veckeyval[0], "");
					}
				}
			}
		}
		// replaces back {stringX} by the strings themselves
		i = 0;
		std::map<std::string, std::string>::iterator fieldsIt = fields->begin();
		for (std::list<std::string>::iterator it = strings->begin(); it != strings->end(); it++, i++) {
			while ((*fieldsIt).second != "{string" + std::to_string(i) + "}") {
				fieldsIt++;
			}
			(*fieldsIt).second = (*it).substr(1, (*it).length() - 2);
			fieldsIt = fields->begin();
		}

		// now the map<str,str> is ready. Look for the right class to load it
		Util::IncIndent();
		{
			std::string thistypename = (*fields->find("typename")).second;
			_model->getTracer()->trace(Util::TraceLevel::L6_internal, "loading " + thistypename + "");
			if (thistypename == "SimulatorInfo") {
				this->_loadSimulatorInfoFields(fields);
			} else if (thistypename == "ModelInfo") {
				_model->getInfos()->loadInstance(fields);
			} else if (thistypename == "ModelSimulation") {
				_model->getSimulation()->loadInstance(fields);
			} else {
				// this should be a ModelComponent or ModelElement.
				ModelElement* newUselessElement = ModelElement::LoadInstance(_model, fields, false);
				if (newUselessElement != nullptr) {
					// \todo how free newUselessElement without invoking destructor?
					////newUselessElement->~ModelElement(false);
					Plugin* plugin = this->_model->getParentSimulator()->getPlugins()->find(thistypename);
					if (plugin != nullptr) {
						res = plugin->loadAndInsertNew(_model, fields);
						// save fields for components, in order to allow to connect components after all of them have been loaded
						if (res && plugin->getPluginInfo()->isComponent()) {
							_componentFields->insert(_componentFields->end(), fields);
						}
					} else {
						_model->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Error loading file: Could not identity typename \"" + thistypename + "\"");
						res = false;
					}
				} else {
					_model->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Error loading file: Could not identity typename \"" + thistypename + "\"");
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
	unsigned int savedVersionNumber = LoadField(fields, "versionNumber", 0);
	unsigned int simulatorVersionNumber = _model->getParentSimulator()->getVersionNumber();
	if (savedVersionNumber != simulatorVersionNumber) {
		_model->getTracer()->trace("WARNING: The version of the saved model differs from the simulator. Loading may not be possible", Util::TraceLevel::L2_errorRecover);
	}
}

bool ModelPersistenceDefaultImpl1::load(std::string filename) {
	bool res = true;
	_model->getTracer()->trace(Util::TraceLevel::L6_internal, "Loading file \"" + filename + "\"");
	Util::IncIndent();
	_componentFields->clear();
	{
		std::ifstream modelFile;
		std::string inputLine;
		try {
			modelFile.open(filename);
			while (getline(modelFile, inputLine) && res) {
				inputLine = trim(inputLine);
				if (inputLine.substr(0, 1) != "#" && !inputLine.empty()) {
					//Util::IncIndent();
					res &= _loadFields(inputLine);
					//Util::DecIndent();
				}
			}
			modelFile.close();
		} catch (...) {
			_model->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Error loading file \"" + filename + "\"");
		}
	}
	// check if something was loaded
	res &= (_model->getComponents()->getNumberOfComponents() > 0) & (_model->getElements()->getNumberOfElements() > 0);
	if (res) {
		//
		// CONNECT LOADED COMPONENTS (must wait for all components to be loaded so they can be connected)
		//
		ComponentManager* cm = _model->getComponents();
		_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Connecting loaded components");
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
				if (fields->find("nextSize") != fields->end()) { // found nextSize
					nextSize = std::stoi((*fields->find("nextSize")).second);
				}
				for (unsigned short i = 0; i < nextSize; i++) {
					Util::identification nextId;
					if (nextSize == 1) {
						if (fields->find("nextId") != fields->end()) {
							nextId = std::stoi((*fields->find("nextId")).second);
						} else {
							nextId = std::stoi((*fields->find("nextId" + std::to_string(i))).second);
						}
					} else {
						nextId = std::stoi((*fields->find("nextId" + std::to_string(i))).second);
					}
					unsigned short nextInputNumber = 0; // default value if it is not found bellow
					if (fields->find("nextInputNumber" + std::to_string(i)) != fields->end())
						nextInputNumber = std::stoi((*fields->find("nextInputNumber" + std::to_string(i))).second);
					ModelComponent* nextComponent = nullptr;
					for (std::list<ModelComponent*>::iterator itcomp = cm->begin(); itcomp != cm->end(); itcomp++) {// connect the components
						if ((*itcomp)->getId() == nextId) { // connect the components
							nextComponent = (*itcomp);
							thisComponent->getNextComponents()->insert(nextComponent, nextInputNumber);
							_model->getTracer()->trace(Util::TraceLevel::L7_detailed, thisComponent->getName() + "<" + std::to_string(i) + ">" + " --> " + nextComponent->getName() + "<" + std::to_string(nextInputNumber) + ">");
							break;
						}
					}
					assert(nextComponent != nullptr);
				}

			}
		}
		Util::DecIndent();
		_model->getTracer()->trace(Util::TraceLevel::L6_internal, "File successfully loaded with " + std::to_string(_model->getComponents()->getNumberOfComponents()) + " components and " + std::to_string(_model->getElements()->getNumberOfElements()) + " elements");
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
	std::string newStr, strV2003, idV2003, typenameV2003, nameV2003, nextIDV2004; //, strV210329;
	idV2003 = "0";
	std::string attrValue;
	for (std::map<std::string, std::string>::iterator it = fields->begin(); it != fields->end(); it++) {
		//newStr += (*it).first + "=" + (*it).second + this->_linefieldseparator;
		if ((*it).first == "id")
			idV2003 = (*it).second;
		else if ((*it).first == "typename") {
			if ((*it).second.substr(0, 1) == "\"" && (*it).second.substr((*it).second.length() - 1, 1) == "\"")
				typenameV2003 = (*it).second.substr(1, (*it).second.length() - 2); // remove "" on the bounders
			else
				typenameV2003 = (*it).second;
		} else if ((*it).first == "name")
			nameV2003 = (*it).second; //_convertLineseparatorToLineseparatorReplacement((*it).second);
		else if ((*it).first.find("nextId", 0) != std::string::npos)
			nextIDV2004 += (*it).first + "=" + (*it).second + _fieldseparator;
		else if ((*it).first.find("nextInputNumber", 0) != std::string::npos)
			nextIDV2004 += (*it).first + "=" + (*it).second + _fieldseparator;
		else if ((*it).first.find("nextSize", 0) != std::string::npos)
			nextIDV2004 = (*it).first + "=" + (*it).second + _fieldseparator + nextIDV2004;
		else {
			// version V210329: (*it).second should NEVER contain _linefieldseparator. So, replace it by _linefieldseparatorReplacement
			attrValue = (*it).second; // _convertLineseparatorToLineseparatorReplacement((*it).second);
			strV2003 += (*it).first + "=" + attrValue + _fieldseparator;
		}
	}
	while (idV2003.length() < 3)
		idV2003 += _fieldseparator;
	while (typenameV2003.length() < 10)
		typenameV2003 += _fieldseparator;
	strV2003 = idV2003 + _fieldseparator + typenameV2003 + _fieldseparator + nameV2003 + _fieldseparator + strV2003 + nextIDV2004;
	_model->getTracer()->trace(Util::TraceLevel::L7_detailed, strV2003); //newStr
	newList->push_back(strV2003); //newStr
	return newList;
}

bool ModelPersistenceDefaultImpl1::hasChanged() {
	return _hasChanged;
}
