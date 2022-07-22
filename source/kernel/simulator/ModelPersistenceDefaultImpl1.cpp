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
#include <memory>

#include "ModelComponent.h"
#include "Simulator.h"
#include "../TraitsKernel.h"
#include "Counter.h"

//using namespace GenesysKernel;

ModelPersistenceDefaultImpl1::ModelPersistenceDefaultImpl1(Model* model) {
    _model = model;
}

void ModelPersistenceDefaultImpl1::_getSimulatorInfoFieldsToSave(PersistenceRecord *fields) {
    fields->saveField("typename", "Simulator");
    fields->saveField("name", _model->getParentSimulator()->getName());
    fields->saveField("versionNumber", _model->getParentSimulator()->getVersionNumber());
}

bool ModelPersistenceDefaultImpl1::save(std::string filename) {
    _model->getTracer()->trace(TraceManager::Level::L7_internal, "Saving file \"" + filename + "\"");
    Util::IncIndent();
    // get options
    bool saveDefaults = (_options && static_cast<int> (Options::SAVEDEFAULTS)) > 0;
    //
    std::list<std::string> simulatorInfosToSave{}, simulationInfosToSave{}, modelInfosToSave{}, modelElementsToSave{}, modelComponentsToSave{};
    {
        //bool res = true;
        auto fields = std::make_unique<PersistenceRecord>(this);
        _getSimulatorInfoFieldsToSave(fields.get());
        simulatorInfosToSave = _adjustFieldsToSave(fields.get());
        // save model own infos
        fields->clear();
        _model->getInfos()->saveInstance(fields.get());
        modelInfosToSave = _adjustFieldsToSave(fields.get());
        // save model own infos
        // @TODO save modelSimulation fields (breakpoints and other stuff included?)
        fields->clear();
        _model->getSimulation()->saveInstance(fields.get(), saveDefaults);
        simulationInfosToSave = _adjustFieldsToSave(fields.get());
        unsigned int modelLevel = 0; // @TODO How get the level of the model?  _model->get
        // save infras
        std::list<std::string>* datadefinitionTypenames = _model->getDataManager()->getDataDefinitionClassnames();
        const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();
        for (std::string thisTypename : *datadefinitionTypenames) {
            if (thisTypename != Util::TypeOf<StatisticsCollector>() && thisTypename != UtilTypeOfCounter) { // STATISTICSCOLLECTR and COUNTERs do NOT need to be saved
                List<ModelDataDefinition*>* infras = _model->getDataManager()->getDataDefinitionList(thisTypename);
                _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing elements of type \"" + thisTypename + "\":");
                Util::IncIndent();
                {
                    for (ModelDataDefinition* data : *infras->list()) {
                        if (data->getLevel() == modelLevel) {
                            _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing " + thisTypename + " \"" + data->getName() + "\"");
                            fields->clear();
                            data->SaveInstance(fields.get(), data);
                            Util::IncIndent();
                            modelElementsToSave.merge(_adjustFieldsToSave(fields.get()));
                            Util::DecIndent();
                        }
                    }
                }
                Util::DecIndent();
            }
        }
        // save components
        _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing components\":");
        Util::IncIndent();
        {
            for (ModelComponent* component : *_model->getComponents()) {
                if (component->getLevel() == modelLevel) {
                    fields->clear();
                    component->SaveInstance(fields.get(), component);
                    Util::IncIndent();
                    modelComponentsToSave.merge(_adjustFieldsToSave(fields.get()));
                    Util::DecIndent();
                }
            }
        }
        Util::DecIndent();
        // SAVE FILE
        _model->getTracer()->trace(TraceManager::Level::L7_internal, "Saving file");
        Util::IncIndent();
        {
            // open file
            std::ofstream savefile;
            savefile.open(filename, std::ofstream::out);
            savefile << "# Genesys Simulation Model " << std::endl;
            ////time_t now = time(0);
            ////char* dt = ctime(&now);
            //savefile << "# Last saved on " << dt;
            savefile << "# Simulator, Model and Simulation infos" << std::endl;
            _saveContent(simulatorInfosToSave, &savefile);
            //savefile << "# model infos" << std::endl;
            _saveContent(modelInfosToSave, &savefile);
            //savefile << "# simulation infos / experimental design" << std::endl;
            _saveContent(simulationInfosToSave, &savefile);
            //savefile << "#" << std::endl;
            savefile << "\n# Model Data Definitions" << std::endl;
            _saveContent(modelElementsToSave, &savefile);
            //savefile << "#" << std::endl;
            savefile << "\n# Model Components" << std::endl;
            _saveContent(modelComponentsToSave, &savefile);
            savefile.close();
        }
        Util::DecIndent();
    }
    Util::DecIndent();
    this->_hasChanged = false;
    return true; // @TODO: check if save really saved successfully
}

void ModelPersistenceDefaultImpl1::_saveContent(const std::list<std::string>& content, std::ofstream* file) {
    for (auto it = content.begin(); it != content.end(); it++) {
        *file << (*it) << std::endl;
    }
}

bool ModelPersistenceDefaultImpl1::_loadFields(std::list<std::map<std::string, std::string>>& componentFields, std::string line) {
    //std::regex regex{R"([=]+)"}; // split on space R"([\s]+)" @TODO: HOW SEPARATOR WITH MORE THAN ONE CHAR
    _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, line);
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
    std::regex regex{R"([\s]+)"}; // split on " " @TODO Should be _lineseparator
    std::sregex_token_iterator tit{line.begin(), line.end(), regex, -1};
    std::list<std::string> lstfields{tit,{}};
    // for each field, separate key and value and form a map
    try {
        std::map<std::string, std::string> fields{};
        regex = {R"([=]+)"};
        std::vector<std::string> veckeyval; //{it,{}};
        unsigned int i = 0;
        for (std::list<std::string>::iterator it = lstfields.begin(); it != lstfields.end(); it++, i++) {
            //std::cout << (*it) << std::endl;
            tit = {(*it).begin(), (*it).end(), regex, -1};
            veckeyval = {tit,{}};
            veckeyval[0] = trim((veckeyval[0]));
            if (veckeyval[0] != "") { // it should always be, rigth? @TODO case for assert?
                if (veckeyval.size() > 1) {
                    veckeyval[1] = trim((veckeyval[1]));
                    if (veckeyval[1].substr(0, 1) == "\"" && veckeyval[1].substr(veckeyval[1].length() - 1, 1) == "\"") { // remove ""
                        veckeyval[1] = veckeyval[1].substr(1, veckeyval[1].length() - 2);
                    }
                    veckeyval[1] = this->_convertLineseparatorReplacementBacktoLineseparator(veckeyval[1]);
                    fields.emplace(veckeyval[0], veckeyval[1]);
                } else {
                    if (i == 0) {
                        fields.emplace("id", veckeyval[0]);
                    } else if (i == 1) {
                        fields.emplace("typename", veckeyval[0]);
                    } else if (i == 2) {
                        ///if (veckeyval[0].substr(0, 1) == "\"" && veckeyval[0].substr(veckeyval[0].length() - 1, 1) == "\"") { // remove ""
                        //	veckeyval[0] = veckeyval[0].substr(1, veckeyval[0].length() - 2);
                        //}
                        //veckeyval[0] = this->_convertLineseparatorReplacementBacktoLineseparator(veckeyval[0]);
                        fields.emplace("name", veckeyval[0]);
                    } else {
                        fields.emplace(veckeyval[0], "");
                    }
                }
            }
        }
        // replaces back {stringX} by the strings themselves
        i = 0;
        std::map<std::string, std::string>::iterator fieldsIt = fields.begin();
        for (std::list<std::string>::iterator it = strings->begin(); it != strings->end(); it++, i++) {
            while ((*fieldsIt).second != "{string" + std::to_string(i) + "}") {
                fieldsIt++;
            }
            (*fieldsIt).second = (*it).substr(1, (*it).length() - 2);
            fieldsIt = fields.begin();
        }

        // now the map<str,str> is ready. Look for the right class to load it
        auto persistence = std::make_unique<PersistenceRecord>(this);
        persistence->insert(fields.begin(), fields.end());
        Util::IncIndent();
        {
            std::string thistypename = fields.find("typename")->second;
            _model->getTracer()->trace(TraceManager::Level::L7_internal, "loading " + thistypename + "");
            if (thistypename == "SimulatorInfo" || thistypename == "Simulator") {
                this->_loadSimulatorInfoFields(persistence.get());
            } else if (thistypename == "ModelInfo") {
                _model->getInfos()->loadInstance(persistence.get());
            } else if (thistypename == "ModelSimulation") {
                _model->getSimulation()->loadInstance(persistence.get());
            } else {
                // this should be a ModelComponent or ModelDataDefinition.
                //ModelDataDefinition* newUselessElement = ModelDataDefinition::LoadInstance(_model, fields, false);
                //if (newUselessElement != nullptr) {
                // @TODO how free newUselessElement without invoking destructor?
                ////newUselessElement->~ModelDataDefinition(false);
                Plugin* plugin = this->_model->getParentSimulator()->getPlugins()->find(thistypename);
                if (plugin != nullptr) {
                    res = plugin->loadAndInsertNew(_model, persistence.get());
                    // save fields for components, in order to allow to connect components after all of them have been loaded
                    if (res && plugin->getPluginInfo()->isComponent()) {
                        componentFields.push_back(fields);
                    }
                } else {
                    _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Error loading file: Could not identity typename \"" + thistypename + "\"");
                    res = false;
                }
            }
        }
        Util::DecIndent();
    } catch (...) {

    }
    return res;
}

void ModelPersistenceDefaultImpl1::_loadSimulatorInfoFields(PersistenceRecord *fields) {
    unsigned int savedVersionNumber = fields->loadField("versionNumber", 0);
    unsigned int simulatorVersionNumber = _model->getParentSimulator()->getVersionNumber();
    if (savedVersionNumber != simulatorVersionNumber) {
        _model->getTracer()->trace("WARNING: The version of the saved model differs from the simulator. Loading may not be possible", TraceManager::Level::L3_errorRecover);
    }
}

bool ModelPersistenceDefaultImpl1::load(std::string filename) {
    bool res = true;
    _model->getTracer()->trace(TraceManager::Level::L7_internal, "Loading file \"" + filename + "\"");
    Util::IncIndent(); 
    std::list<std::map<std::string, std::string>> componentFields{};
    {
        std::ifstream modelFile;
        std::string inputLine;
        try {
            modelFile.open(filename);
            while (getline(modelFile, inputLine) && res) {
                inputLine = trim(inputLine);
                if (inputLine.substr(0, 1) != "#" && !inputLine.empty()) {
                    res &= _loadFields(componentFields, inputLine);
                }
            }
            modelFile.close();
        } catch (...) {
            _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Error loading file \"" + filename + "\"");
        }
    }
    // check if something was loaded
    //res &= (_model->getComponents()->getNumberOfComponents() > 0) & (_model->getDataManager()->getNumberOfDataDefinitions() > 0);
    if (res) {
        //
        // CONNECT LOADED COMPONENTS (must wait for all components to be loaded so they can be connected)
        //
        ComponentManager* cm = _model->getComponents();
        _model->getTracer()->trace(TraceManager::Level::L8_detailed, "Connecting loaded components");
        Util::IncIndent();
        {
            for (auto& fields : componentFields) {
                // find the component
                ModelComponent* thisComponent = nullptr;
                Util::identification thisId = std::stoi(fields.find("id")->second);
                for (std::list<ModelComponent*>::iterator itcomp = cm->begin(); itcomp != cm->end(); itcomp++) {
                    if ((*itcomp)->getId() == thisId) {
                        thisComponent = (*itcomp);
                        break; // end inner for loop
                    }
                }
                assert(thisComponent != nullptr);

                // find the next components connected with this one
                unsigned short nextSize = 1;
                if (fields.find("nexts") != fields.end()) { // found nextSize
                    nextSize = std::stoi(fields.find("nexts")->second);
                }
                for (unsigned short i = 0; i < nextSize; i++) {
                    Util::identification nextId;
                    if (nextSize == 1) {
                        if (fields.find("nextId") != fields.end()) {
                            nextId = std::stoi(fields.find("nextId")->second);
                        } else {
                            nextId = std::stoi(fields.find("nextId" + strIndex(i))->second);
                        }
                    } else {
                        nextId = std::stoi(fields.find("nextId" + strIndex(i))->second);
                    }
                    unsigned short nextinputPortNumber = 0; // default value if it is not found bellow
                    if (fields.find("nextinputPortNumber" + std::to_string(i)) != fields.end())
                        nextinputPortNumber = std::stoi(fields.find("nextinputPortNumber" + std::to_string(i))->second);
                    ModelComponent* nextComponent = nullptr;
                    for (std::list<ModelComponent*>::iterator itcomp = cm->begin(); itcomp != cm->end(); itcomp++) {// connect the components
                        if ((*itcomp)->getId() == nextId) { // connect the components
                            nextComponent = (*itcomp);
                            thisComponent->getConnections()->insert(nextComponent, nextinputPortNumber);
                            _model->getTracer()->trace(TraceManager::Level::L8_detailed, thisComponent->getName() + "<" + std::to_string(i) + ">" + " --> " + nextComponent->getName() + "<" + std::to_string(nextinputPortNumber) + ">");
                            break;
                        }
                    }
                    assert(nextComponent != nullptr);
                }

            }
        }
        Util::DecIndent();
        _model->getTracer()->trace(TraceManager::Level::L7_internal, "File successfully loaded with " + std::to_string(_model->getComponents()->getNumberOfComponents()) + " components and " + std::to_string(_model->getDataManager()->getNumberOfDataDefinitions()) + " elements");
    }
    Util::DecIndent();
    if (res) {
        _hasChanged = false;
    }
    return res;
}

bool ModelPersistenceDefaultImpl1::getOption(ModelPersistence_if::Options option) {
    return (_options & static_cast<int> (option)) > 0;
}

void ModelPersistenceDefaultImpl1::setOption(ModelPersistence_if::Options option, bool value) {
    if (value) {
        _options |= static_cast<int> (option);
    } else {
        _options &= ~(static_cast<int> (option));
    }
}

std::string ModelPersistenceDefaultImpl1::getFormatedField(PersistenceRecord *fields) {
    std::list<std::string> formatedList = this->_adjustFieldsToSave(fields);
    std::string txt = "";
    for (std::string formated : formatedList) {
        txt += formated + " ";
    }
    return txt.substr(0, txt.length() - 1);
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

std::list<std::string> ModelPersistenceDefaultImpl1::_adjustFieldsToSave(PersistenceRecord *fields) {
    std::list<std::string> newList;
    std::string newStr, strV2003, idV2003, typenameV2003, nameV2003, nextIDV2004; //, strV210329;
    idV2003 = "0";
    std::string attrValue;
    for (auto it = fields->begin(); it != fields->end(); it++) {
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
        else if ((*it).first.find("nextinputPortNumber", 0) != std::string::npos)
            nextIDV2004 += (*it).first + "=" + (*it).second + _fieldseparator;
        else if ((*it).first.find("nexts", 0) != std::string::npos)
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
    _model->getTracer()->trace(TraceManager::Level::L8_detailed, strV2003); //newStr
    newList.push_back(strV2003); //newStr
    return newList;
}

bool ModelPersistenceDefaultImpl1::hasChanged() {
    return _hasChanged;
}
