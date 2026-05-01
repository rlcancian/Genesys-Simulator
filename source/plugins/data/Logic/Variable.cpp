/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Variable.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 4 de Setembro de 2018, 18:28
 */

#include "plugins/data/Logic/Variable.h"
#include "kernel/simulator/Model.h"

#include <cctype>
#include <sstream>
#include <stdexcept>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Variable::GetPluginInformation;
}
#endif

namespace {

struct InitialValueNode {
	bool leaf = true;
	double value = 0.0;
	std::vector<InitialValueNode> children;
};

std::string trimCopy(const std::string& text) {
	std::size_t first = 0;
	while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) {
		++first;
	}
	std::size_t last = text.size();
	while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
		--last;
	}
	return text.substr(first, last - first);
}

class InitialValueParser {
public:
	explicit InitialValueParser(std::string text) : _text(std::move(text)) {
	}

	bool parse(InitialValueNode* node, std::string* errorMessage) {
		if (node == nullptr) {
			if (errorMessage != nullptr) {
				*errorMessage = "Initial value parser received a null output node.";
			}
			return false;
		}
		_pos = 0;
		skipSpaces();
		if (!parseNode(node, errorMessage)) {
			return false;
		}
		skipSpaces();
		if (_pos != _text.size()) {
			if (errorMessage != nullptr) {
				*errorMessage = "Unexpected trailing characters in initial values text.";
			}
			return false;
		}
		return true;
	}

private:
	bool parseNode(InitialValueNode* node, std::string* errorMessage) {
		skipSpaces();
		if (_pos >= _text.size()) {
			if (errorMessage != nullptr) {
				*errorMessage = "Unexpected end of initial values text.";
			}
			return false;
		}

		if (_text[_pos] == '[') {
			node->leaf = false;
			node->children.clear();
			++_pos;
			skipSpaces();
			if (_pos < _text.size() && _text[_pos] == ']') {
				++_pos;
				return true;
			}
			while (true) {
				InitialValueNode child;
				if (!parseNode(&child, errorMessage)) {
					return false;
				}
				node->children.push_back(std::move(child));
				skipSpaces();
				if (_pos >= _text.size()) {
					if (errorMessage != nullptr) {
						*errorMessage = "Unterminated list in initial values text.";
					}
					return false;
				}
				if (_text[_pos] == ',') {
					++_pos;
					continue;
				}
				if (_text[_pos] == ']') {
					++_pos;
					break;
				}
				if (errorMessage != nullptr) {
					*errorMessage = "Expected ',' or ']' in initial values text.";
				}
				return false;
			}
			return true;
		}

		const char* begin = _text.c_str() + _pos;
		char* end = nullptr;
		const double value = std::strtod(begin, &end);
		if (end == begin) {
			if (errorMessage != nullptr) {
				*errorMessage = "Expected numeric value in initial values text.";
			}
			return false;
		}
		node->leaf = true;
		node->value = value;
		_pos = static_cast<std::size_t>(end - _text.c_str());
		return true;
	}

	void skipSpaces() {
		while (_pos < _text.size() && std::isspace(static_cast<unsigned char>(_text[_pos]))) {
			++_pos;
		}
	}

	std::string _text;
	std::size_t _pos = 0;
};

bool inferShape(const InitialValueNode& node, std::vector<unsigned int>* shape, std::string* errorMessage) {
	if (node.leaf) {
		return true;
	}
	if (node.children.empty()) {
		if (errorMessage != nullptr) {
			*errorMessage = "Initial values list cannot be empty.";
		}
		return false;
	}

	std::vector<unsigned int> childShape;
	if (!inferShape(node.children.front(), &childShape, errorMessage)) {
		return false;
	}
	for (std::size_t i = 1; i < node.children.size(); ++i) {
		std::vector<unsigned int> otherShape;
		if (!inferShape(node.children.at(i), &otherShape, errorMessage)) {
			return false;
		}
		if (otherShape != childShape) {
			if (errorMessage != nullptr) {
				*errorMessage = "Initial values must form a rectangular array.";
			}
			return false;
		}
	}

	shape->clear();
	shape->push_back(static_cast<unsigned int>(node.children.size()));
	shape->insert(shape->end(), childShape.begin(), childShape.end());
	return true;
}

void writeNodeToStore(const InitialValueNode& node,
                      std::vector<unsigned int>* indexPath,
                      SparseValueStore* store) {
	if (node.leaf) {
		store->setValue(node.value, SparseValueStore::makeIndexKey(*indexPath));
		return;
	}
	for (std::size_t i = 0; i < node.children.size(); ++i) {
		indexPath->push_back(static_cast<unsigned int>(i));
		writeNodeToStore(node.children.at(i), indexPath, store);
		indexPath->pop_back();
	}
}

std::string formatNumber(double value) {
	std::ostringstream out;
	out << value;
	return out.str();
}

std::string serializeNode(const SparseValueStore* store,
                          const std::list<unsigned int>& dimensions,
                          std::vector<unsigned int>* indexPath,
                          std::size_t level) {
	if (level >= dimensions.size()) {
		return formatNumber(store->value(SparseValueStore::makeIndexKey(*indexPath)));
	}

	std::ostringstream out;
	out << "[";
	const unsigned int count = *std::next(dimensions.begin(), static_cast<long>(level));
	for (unsigned int i = 0; i < count; ++i) {
		indexPath->push_back(i);
		if (i > 0) {
			out << ",";
		}
		out << serializeNode(store, dimensions, indexPath, level + 1);
		indexPath->pop_back();
	}
	out << "]";
	return out.str();
}

bool parseInitialValuesText(const std::string& valuesText,
                            SparseValueStore* initialValues,
                            std::string* errorMessage) {
	if (initialValues == nullptr) {
		if (errorMessage != nullptr) {
			*errorMessage = "Initial values store is null.";
		}
		return false;
	}

	const std::string trimmed = trimCopy(valuesText);
	InitialValueParser parser(trimmed);
	InitialValueNode root;
	if (!parser.parse(&root, errorMessage)) {
		return false;
	}

	std::vector<unsigned int> shape;
	if (!inferShape(root, &shape, errorMessage)) {
		return false;
	}

	SparseValueStore parsedStore;
	for (unsigned int dimension : shape) {
		parsedStore.insertDimensionSize(dimension);
	}
	std::vector<unsigned int> indexPath;
	writeNodeToStore(root, &indexPath, &parsedStore);
	*initialValues = parsedStore;
	return true;
}

} // namespace

ModelDataDefinition* Variable::NewInstance(Model* model, std::string name) {
	return new Variable(model, name);
}

Variable::Variable(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Variable>(), name) {
	setName(name);
	SimulationControlString* propInitialValue = new SimulationControlString(
			std::bind(&Variable::getInitialValuesText, this),
			std::bind(&Variable::setInitialValuesText, this, std::placeholders::_1),
			Util::TypeOf<Variable>(), getName(), "InitialValue", "",
			false, false, false,
			[](const std::string& value, std::string& errorMessage) {
				SparseValueStore scratch;
				return parseInitialValuesText(value, &scratch, &errorMessage);
			});
	// this Control was infered from getters and setters
	SimulationControlGenericClass<ModelDataDefinition*, Model*, ModelDataDefinition>* propScope =
			new SimulationControlGenericClass<ModelDataDefinition*, Model*, ModelDataDefinition>(
					_parentModel,
					std::bind(&Variable::get_scope, this),
					std::bind(&Variable::set_scope, this, std::placeholders::_1),
					Util::TypeOf<Variable>(), getName(), "Scope", "");
	_parentModel->getControls()->insert(propInitialValue);
	_parentModel->getControls()->insert(propScope);
	_addSimulationControl(propInitialValue);
	_addSimulationControl(propScope);
}

Variable::~Variable() {
	delete _values;
	_values = nullptr;
	delete _initialValues;
	_initialValues = nullptr;
}

std::string Variable::show() {
	return ModelDataDefinition::show() + ", values:" + this->_values->showValues();
}

PluginInformation* Variable::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Variable>(), &Variable::LoadInstance, &Variable::NewInstance);
	return info;
}

//double Variable::getValue() {
//	return getValue("");
//}

double Variable::getValue(std::string index) {
	return _values->value(index);
}

//void Variable::setValue(double value) {
//	setValue("", value);
//}

void Variable::setValue(double value,std::string index) {
	_values->setValue(value, index);
}

std::string Variable::getInitialValuesText() const {
	const std::list<unsigned int>& dimensions = *_initialValues->dimensionSizes();
	if (dimensions.empty()) {
		return formatNumber(_initialValues->value(""));
	}
	std::vector<unsigned int> indexPath;
	return serializeNode(_initialValues, dimensions, &indexPath, 0);
}

void Variable::setInitialValuesText(std::string valuesText) {
	SparseValueStore parsedStore;
	std::string errorMessage;
	if (!parseInitialValuesText(valuesText, &parsedStore, &errorMessage)) {
		throw std::invalid_argument(errorMessage.empty() ? "Invalid initial values text." : errorMessage);
	}
	*_initialValues = parsedStore;
	*_values = *_initialValues;
}

/*
double Variable::getInitialValue() {
	return getInitialValue("");
}

void Variable::setInitialValue(double value) {
	setInitialValue("", value);
}
*/

double Variable::getInitialValue(std::string index) {
	return _initialValues->value(index);
}

void Variable::setInitialValue(double value, std::string index) {
	_initialValues->setValue(value, index);
}

void Variable::setInitialValues(const std::vector<std::pair<std::string, double>> values) {
	for(std::pair<std::string,double> pair: values) {
		setInitialValue(pair.second, pair.first);
	}
}

void Variable::insertDimentionSize(unsigned int size) {
	// Dimension metadata applies both to initial values and runtime values.
	_initialValues->insertDimensionSize(size);
	_values->insertDimensionSize(size);
}

std::list<unsigned int>* Variable::getDimensionSizes() const {
	return _initialValues->dimensionSizes();
}

ModelDataDefinition* Variable::LoadInstance(Model* model, PersistenceRecord *fields) {
	Variable* newElement = new Variable(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Variable::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		// Preserve the legacy persistence layout while centralizing sparse parsing.
		this->_initialValues->loadDimensions(fields);
		this->_initialValues->loadValues(fields, "values", "valuePos", "value");
		*this->_values = *this->_initialValues;
	}
	return res;
}

void Variable::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	_initialValues->saveDimensions(fields, saveDefaultValues);
	_initialValues->saveValues(fields, "values", "valuePos", "value", saveDefaultValues);
}

bool Variable::_check(std::string& errorMessage) {
	errorMessage += "";
	return true;
}

void Variable::_initBetweenReplications() {
	*this->_values = *this->_initialValues;
}

ModelDataDefinition* Variable::get_scope() {
	return scope;
}

void Variable::set_scope(ModelDataDefinition* const scope) {
	this->scope = scope;
}

std::map<std::string, double> *Variable::getValues() const {
	return _values->values();
}

SparseValueStore* Variable::getValueStore() {
	return _values;
}

SparseValueStore* Variable::getInitialValueStore() {
	return _initialValues;
}

// void Variable::_createInternalStatisticReporters() { }

// void Variable::_createEditableDataDefinitions() { }

// void Variable::_createAttachedAttributes() { }
