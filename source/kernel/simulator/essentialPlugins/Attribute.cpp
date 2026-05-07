/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Attribute.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 25 de Setembro de 2018, 16:37
 */

#include "Attribute.h"
#include "../model/Model.h"

#include <cctype>
#include <cstdlib>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>

//using namespace GenesysKernel;

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Attribute::GetPluginInformation;
}
#endif

ModelDataDefinition* Attribute::NewInstance(Model* model, std::string name) {
	return new Attribute(model, name);
}

namespace {

struct InitialValueNode {
	bool leaf = true;
	double value = 0.0;
	std::vector<InitialValueNode> children;
	std::vector<bool> rowBreakAfter;
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
	static bool isValueStart(char ch) {
		return ch == '[' || ch == '+' || ch == '-' || ch == '.' || std::isdigit(static_cast<unsigned char>(ch));
	}

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
			node->rowBreakAfter.clear();
			++_pos;
			skipSpaces();
			if (_pos < _text.size() && _text[_pos] == ']') {
				if (errorMessage != nullptr) {
					*errorMessage = "Initial values list cannot be empty.";
				}
				return false;
			}
			while (true) {
				InitialValueNode child;
				if (!parseNode(&child, errorMessage)) {
					return false;
				}
				node->children.push_back(std::move(child));
				node->rowBreakAfter.push_back(false);
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
				if (_text[_pos] == ';') {
					++_pos;
					node->rowBreakAfter.back() = true;
					continue;
				}
				if (_text[_pos] == ']') {
					++_pos;
					break;
				}
				if (isValueStart(_text[_pos])) {
					continue;
				}
				if (errorMessage != nullptr) {
					*errorMessage = "Expected ',', ';' or ']' in initial values text.";
				}
				return false;
			}
			normalizeLegacyRows(node);
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

	void normalizeLegacyRows(InitialValueNode* node) {
		if (node == nullptr || node->leaf || node->children.size() <= 1) {
			return;
		}
		const bool hasExplicitRowBreak = std::any_of(node->rowBreakAfter.begin(), node->rowBreakAfter.end(), [](bool value) {
			return value;
		});
		if (hasExplicitRowBreak) {
			return;
		}
		const bool allChildrenAreArrays = std::all_of(node->children.begin(), node->children.end(), [](const InitialValueNode& child) {
			return !child.leaf;
		});
		if (!allChildrenAreArrays) {
			return;
		}
		for (std::size_t i = 0; i + 1 < node->rowBreakAfter.size(); ++i) {
			node->rowBreakAfter[i] = true;
		}
	}

	void skipSpaces() {
		while (_pos < _text.size() && std::isspace(static_cast<unsigned char>(_text[_pos]))) {
			++_pos;
		}
	}

	std::string _text;
	std::size_t _pos = 0;
};

std::vector<std::vector<const InitialValueNode*>> splitRows(const InitialValueNode& node) {
	std::vector<std::vector<const InitialValueNode*>> rows;
	if (node.leaf) {
		return rows;
	}

	if (node.children.empty()) {
		return rows;
	}

	const bool hasExplicitRowBreak = std::any_of(node.rowBreakAfter.begin(), node.rowBreakAfter.end(), [](bool value) {
		return value;
	});
	const bool allChildrenAreArrays = std::all_of(node.children.begin(), node.children.end(), [](const InitialValueNode& child) {
		return !child.leaf;
	});
	if (!hasExplicitRowBreak && allChildrenAreArrays && node.children.size() > 1) {
		rows.reserve(node.children.size());
		for (const InitialValueNode& child : node.children) {
			rows.push_back({&child});
		}
		return rows;
	}

	std::vector<const InitialValueNode*> currentRow;
	currentRow.reserve(node.children.size());
	for (std::size_t i = 0; i < node.children.size(); ++i) {
		currentRow.push_back(&node.children.at(i));
		if (i < node.rowBreakAfter.size() && node.rowBreakAfter.at(i)) {
			rows.push_back(currentRow);
			currentRow.clear();
		}
	}
	if (!currentRow.empty()) {
		rows.push_back(currentRow);
	}
	return rows;
}

bool inferShape(const InitialValueNode& node, std::vector<unsigned int>* shape, std::string* errorMessage);

bool inferRowShape(const std::vector<const InitialValueNode*>& row,
                   std::vector<unsigned int>* shape,
                   std::string* errorMessage) {
	if (row.empty()) {
		if (errorMessage != nullptr) {
			*errorMessage = "Initial values row cannot be empty.";
		}
		return false;
	}

	const bool allLeaf = std::all_of(row.begin(), row.end(), [](const InitialValueNode* child) {
		return child != nullptr && child->leaf;
	});
	const bool allArray = std::all_of(row.begin(), row.end(), [](const InitialValueNode* child) {
		return child != nullptr && !child->leaf;
	});
	if (!allLeaf && !allArray) {
		if (errorMessage != nullptr) {
			*errorMessage = "Initial values row must contain only scalars or only arrays.";
		}
		return false;
	}

	if (allLeaf) {
		shape->clear();
		shape->push_back(static_cast<unsigned int>(row.size()));
		return true;
	}

	std::vector<unsigned int> childShape;
	if (!inferShape(*row.front(), &childShape, errorMessage)) {
		return false;
	}
	for (std::size_t i = 1; i < row.size(); ++i) {
		std::vector<unsigned int> otherShape;
		if (!inferShape(*row.at(i), &otherShape, errorMessage)) {
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
	if (row.size() == 1) {
		shape->insert(shape->end(), childShape.begin(), childShape.end());
		return true;
	}
	shape->push_back(static_cast<unsigned int>(row.size()));
	shape->insert(shape->end(), childShape.begin(), childShape.end());
	return true;
}

bool inferShape(const InitialValueNode& node, std::vector<unsigned int>* shape, std::string* errorMessage) {
	if (node.leaf) {
		shape->clear();
		return true;
	}

	const std::vector<std::vector<const InitialValueNode*>> rows = splitRows(node);
	if (rows.empty()) {
		if (errorMessage != nullptr) {
			*errorMessage = "Initial values list cannot be empty.";
		}
		return false;
	}
	if (rows.size() == 1) {
		return inferRowShape(rows.front(), shape, errorMessage);
	}

	std::vector<unsigned int> rowShape;
	if (!inferRowShape(rows.front(), &rowShape, errorMessage)) {
		return false;
	}
	for (std::size_t i = 1; i < rows.size(); ++i) {
		std::vector<unsigned int> otherRowShape;
		if (!inferRowShape(rows.at(i), &otherRowShape, errorMessage)) {
			return false;
		}
		if (otherRowShape != rowShape) {
			if (errorMessage != nullptr) {
				*errorMessage = "Initial values must form a rectangular array.";
			}
			return false;
		}
	}

	shape->clear();
	shape->push_back(static_cast<unsigned int>(rows.size()));
	shape->insert(shape->end(), rowShape.begin(), rowShape.end());
	return true;
}

void writeNodeToStore(const InitialValueNode& node,
                      std::vector<unsigned int>* indexPath,
                      SparseValueStore* store) {
	if (node.leaf) {
		store->setValue(node.value, SparseValueStore::makeIndexKey(*indexPath));
		return;
	}

	const std::vector<std::vector<const InitialValueNode*>> rows = splitRows(node);
	if (rows.size() == 1) {
		const std::vector<const InitialValueNode*>& row = rows.front();
		if (!row.empty() && std::all_of(row.begin(), row.end(), [](const InitialValueNode* child) {
			return child != nullptr && child->leaf;
		})) {
			for (std::size_t columnIndex = 0; columnIndex < row.size(); ++columnIndex) {
				indexPath->push_back(static_cast<unsigned int>(columnIndex));
				store->setValue(row.at(columnIndex)->value, SparseValueStore::makeIndexKey(*indexPath));
				indexPath->pop_back();
			}
			return;
		}
		if (row.size() == 1 && row.front() != nullptr) {
			writeNodeToStore(*row.front(), indexPath, store);
			return;
		}
		for (std::size_t elementIndex = 0; elementIndex < row.size(); ++elementIndex) {
			indexPath->push_back(static_cast<unsigned int>(elementIndex));
			writeNodeToStore(*row.at(elementIndex), indexPath, store);
			indexPath->pop_back();
		}
		return;
	}

	for (std::size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
		indexPath->push_back(static_cast<unsigned int>(rowIndex));
		const std::vector<const InitialValueNode*>& row = rows.at(rowIndex);
		if (!row.empty() && std::all_of(row.begin(), row.end(), [](const InitialValueNode* child) {
			return child != nullptr && child->leaf;
		})) {
			for (std::size_t columnIndex = 0; columnIndex < row.size(); ++columnIndex) {
				indexPath->push_back(static_cast<unsigned int>(columnIndex));
				store->setValue(row.at(columnIndex)->value, SparseValueStore::makeIndexKey(*indexPath));
				indexPath->pop_back();
			}
		} else if (row.size() == 1 && row.front() != nullptr) {
			writeNodeToStore(*row.front(), indexPath, store);
		} else {
			for (std::size_t elementIndex = 0; elementIndex < row.size(); ++elementIndex) {
				indexPath->push_back(static_cast<unsigned int>(elementIndex));
				writeNodeToStore(*row.at(elementIndex), indexPath, store);
				indexPath->pop_back();
			}
		}
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

Attribute::Attribute(Model* model, std::string name, std::string dataDefinitionTypename)
		: ModelDataDefinition(model, dataDefinitionTypename, name) {
	if (_parentModel != nullptr) {
		SimulationControlString* propInitialValue = new SimulationControlString(
				std::bind(&Attribute::getInitialValuesText, this),
				std::bind(&Attribute::setInitialValuesText, this, std::placeholders::_1),
				dataDefinitionTypename, getName(), "InitialValue", "",
				false, false, false,
				[this](const std::string& value, std::string& errorMessage) {
					SparseValueStore parsedStore;
					return parseInitialValuesText(value, &parsedStore, &errorMessage);
				});
		propInitialValue->setPreferredEditorHint(SimulationControlEditorHint::MultiLineText);
		_parentModel->getControls()->insert(propInitialValue);
		_addSimulationControl(propInitialValue);
	}
	_syncInitialValuesTextFromStore();
}

Attribute::~Attribute() {
	delete _initialValues;
	_initialValues = nullptr;
}

std::string Attribute::show() {
	return ModelDataDefinition::show() + ", initialValues:" + _initialValues->showValues();
}

std::string Attribute::getInitialValuesText() const {
	return _initialValuesText;
}

void Attribute::setInitialValuesText(std::string valuesText) {
	SparseValueStore parsedStore;
	std::string errorMessage;
	if (!parseInitialValuesText(valuesText, &parsedStore, &errorMessage)) {
		_initialValuesText = valuesText;
		_initialValuesTextValid = false;
		_initialValuesTextErrorMessage = errorMessage.empty() ? "Invalid initial values text: \""+valuesText+"\"" : errorMessage;
		return;
	}
	*_initialValues = parsedStore;
	_initialValuesTextValid = true;
	_initialValuesTextErrorMessage.clear();
	_syncInitialValuesTextFromStore();
}

bool Attribute::_loadInstance(PersistenceRecord* fields) {
	const bool result = ModelDataDefinition::_loadInstance(fields);
	if (result) {
		// Attribute uses the same sparse persistence shape as Variable.
		_initialValues->loadDimensions(fields);
		_initialValues->loadValues(fields, "values", "valuePos", "value");
		_initialValuesTextValid = true;
		_initialValuesTextErrorMessage.clear();
		_syncInitialValuesTextFromStore();
	}
	return result;
}

PluginInformation* Attribute::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Attribute>(), &Attribute::LoadInstance,
	                                                &Attribute::NewInstance);
	// @ToDo: (pequena alteração): Add Attribute description help
	info->setDescriptionHelp("");
	return info;
}

ModelDataDefinition* Attribute::LoadInstance(Model* model, PersistenceRecord* fields) {
	Attribute* newElement = new Attribute(model);
	try {
		newElement->_loadInstance(fields);
	}
	catch (const std::exception& e) {
	}
	return newElement;
}

void Attribute::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	_initialValues->saveDimensions(fields, saveDefaultValues);
	_initialValues->saveValues(fields, "values", "valuePos", "value", saveDefaultValues);
}

bool Attribute::_check(std::string& errorMessage) {
	if (!_initialValuesTextValid) {
		errorMessage += _initialValuesTextErrorMessage.empty()
			? "InitialValue contains an invalid expression."
			: _initialValuesTextErrorMessage;
		return false;
	}
	return true;
}

double Attribute::getInitialValue(std::string index) {
	return _initialValues->value(index);
}

void Attribute::setInitialValue(double value, std::string index) {
	_initialValues->setValue(value, index);
	_initialValuesTextValid = true;
	_initialValuesTextErrorMessage.clear();
	_syncInitialValuesTextFromStore();
}

void Attribute::setInitialValues(const std::vector<std::pair<std::string, double>> values) {
	for (const std::pair<std::string, double>& pair : values) {
		setInitialValue(pair.second, pair.first);
	}
}

void Attribute::insertDimentionSize(unsigned int size) {
	_initialValues->insertDimensionSize(size);
	_initialValuesTextValid = true;
	_initialValuesTextErrorMessage.clear();
	_syncInitialValuesTextFromStore();
}

std::list<unsigned int>* Attribute::getDimensionSizes() const {
	return _initialValues->dimensionSizes();
}

std::map<std::string, double>* Attribute::getInitialValues() const {
	return _initialValues->values();
}

SparseValueStore* Attribute::getInitialValueStore() {
	return _initialValues;
}

bool Attribute::isInitialValuesTextValid() const {
	return _initialValuesTextValid;
}

void Attribute::_syncInitialValuesTextFromStore() {
	const std::list<unsigned int>& dimensions = *_initialValues->dimensionSizes();
	if (dimensions.empty()) {
		_initialValuesText = formatNumber(_initialValues->value(""));
		return;
	}
	std::vector<unsigned int> indexPath;
	_initialValuesText = serializeNode(_initialValues, dimensions, &indexPath, 0);
}
