/*
 * File:   BacteriaSignalGrid.cpp
 * Author: GRO
 *
 * Created on 28 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/BacteriaSignalGrid.h"
#include "kernel/simulator/Model.h"

#include <cctype>
#include <sstream>
#include <stdexcept>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BacteriaSignalGrid::GetPluginInformation;
}
#endif

namespace {

std::string trim(const std::string& text) {
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

std::vector<std::string> splitSignalValues(const std::string& text) {
	std::vector<std::string> tokens;
	std::string current;
	for (char character : text) {
		if (character == ',' || character == ';' || std::isspace(static_cast<unsigned char>(character))) {
			const std::string token = trim(current);
			if (!token.empty()) {
				tokens.push_back(token);
			}
			current.clear();
			continue;
		}
		current.push_back(character);
	}

	const std::string trailing = trim(current);
	if (!trailing.empty()) {
		tokens.push_back(trailing);
	}
	return tokens;
}

} // namespace

ModelDataDefinition* BacteriaSignalGrid::NewInstance(Model* model, std::string name) {
	return new BacteriaSignalGrid(model, name);
}

BacteriaSignalGrid::BacteriaSignalGrid(Model* model, std::string name) :
		ModelDataDefinition(model, Util::TypeOf<BacteriaSignalGrid>(), name) {
	auto* propWidth = new SimulationControlGeneric<unsigned int>(
			std::bind(&BacteriaSignalGrid::getWidth, this),
			std::bind(&BacteriaSignalGrid::setWidth, this, std::placeholders::_1),
			Util::TypeOf<BacteriaSignalGrid>(), getName(), "Width",
			"Discrete signal-grid width");
	auto* propHeight = new SimulationControlGeneric<unsigned int>(
			std::bind(&BacteriaSignalGrid::getHeight, this),
			std::bind(&BacteriaSignalGrid::setHeight, this, std::placeholders::_1),
			Util::TypeOf<BacteriaSignalGrid>(), getName(), "Height",
			"Discrete signal-grid height");
	auto* propInitialSignal = new SimulationControlDouble(
			std::bind(&BacteriaSignalGrid::getInitialSignal, this),
			std::bind(&BacteriaSignalGrid::setInitialSignal, this, std::placeholders::_1),
			Util::TypeOf<BacteriaSignalGrid>(), getName(), "InitialSignal",
			"Default initial signal value used when no explicit cell values are provided");
	auto* propDiffusionRate = new SimulationControlDouble(
			std::bind(&BacteriaSignalGrid::getDiffusionRate, this),
			std::bind(&BacteriaSignalGrid::setDiffusionRate, this, std::placeholders::_1),
			Util::TypeOf<BacteriaSignalGrid>(), getName(), "DiffusionRate",
			"Per-step diffusion relaxation factor in the [0,1] interval");
	auto* propDecayRate = new SimulationControlDouble(
			std::bind(&BacteriaSignalGrid::getDecayRate, this),
			std::bind(&BacteriaSignalGrid::setDecayRate, this, std::placeholders::_1),
			Util::TypeOf<BacteriaSignalGrid>(), getName(), "DecayRate",
			"Per-step decay factor in the [0,1] interval");
	auto* propInitialValues = new SimulationControlString(
			std::bind(&BacteriaSignalGrid::getInitialValues, this),
			std::bind(&BacteriaSignalGrid::setInitialValues, this, std::placeholders::_1),
			Util::TypeOf<BacteriaSignalGrid>(), getName(), "InitialValues",
			"Optional comma-separated row-major initial values for every cell");

	_parentModel->getControls()->insert(propWidth);
	_parentModel->getControls()->insert(propHeight);
	_parentModel->getControls()->insert(propInitialSignal);
	_parentModel->getControls()->insert(propDiffusionRate);
	_parentModel->getControls()->insert(propDecayRate);
	_parentModel->getControls()->insert(propInitialValues);

	_addSimulationControl(propWidth);
	_addSimulationControl(propHeight);
	_addSimulationControl(propInitialSignal);
	_addSimulationControl(propDiffusionRate);
	_addSimulationControl(propDecayRate);
	_addSimulationControl(propInitialValues);
}

std::string BacteriaSignalGrid::show() {
	return ModelDataDefinition::show() +
	       ",width=" + std::to_string(_width) +
	       ",height=" + std::to_string(_height) +
	       ",initialSignal=" + Util::StrTruncIfInt(std::to_string(_initialSignal)) +
	       ",diffusionRate=" + Util::StrTruncIfInt(std::to_string(_diffusionRate)) +
	       ",decayRate=" + Util::StrTruncIfInt(std::to_string(_decayRate));
}

PluginInformation* BacteriaSignalGrid::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BacteriaSignalGrid>(),
	                                                &BacteriaSignalGrid::LoadInstance,
	                                                &BacteriaSignalGrid::NewInstance);
	info->setCategory("BiologicalModeling");
	info->setDescriptionHelp("Stores the reusable spatial configuration of a discrete bacteria signal field, "
	                         "including dimensions, initial values, and simple diffusion/decay parameters.");
	return info;
}

ModelDataDefinition* BacteriaSignalGrid::LoadInstance(Model* model, PersistenceRecord* fields) {
	BacteriaSignalGrid* newElement = new BacteriaSignalGrid(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

void BacteriaSignalGrid::setWidth(unsigned int width) {
	_width = width;
}

unsigned int BacteriaSignalGrid::getWidth() const {
	return _width;
}

void BacteriaSignalGrid::setHeight(unsigned int height) {
	_height = height;
}

unsigned int BacteriaSignalGrid::getHeight() const {
	return _height;
}

void BacteriaSignalGrid::setInitialSignal(double initialSignal) {
	_initialSignal = initialSignal;
}

double BacteriaSignalGrid::getInitialSignal() const {
	return _initialSignal;
}

void BacteriaSignalGrid::setDiffusionRate(double diffusionRate) {
	_diffusionRate = diffusionRate;
}

double BacteriaSignalGrid::getDiffusionRate() const {
	return _diffusionRate;
}

void BacteriaSignalGrid::setDecayRate(double decayRate) {
	_decayRate = decayRate;
}

double BacteriaSignalGrid::getDecayRate() const {
	return _decayRate;
}

void BacteriaSignalGrid::setInitialValues(std::string initialValues) {
	_initialValues = std::move(initialValues);
}

std::string BacteriaSignalGrid::getInitialValues() const {
	return _initialValues;
}

unsigned int BacteriaSignalGrid::getCellCount() const {
	return _width * _height;
}

bool BacteriaSignalGrid::buildInitialField(std::vector<double>& values, std::string& errorMessage) const {
	values.assign(getCellCount(), _initialSignal);
	if (_initialValues.empty()) {
		return true;
	}

	const std::vector<std::string> tokens = splitSignalValues(_initialValues);
	if (tokens.size() != values.size()) {
		errorMessage += "BacteriaSignalGrid explicit initial values count must match width*height. ";
		return false;
	}

	// Explicit values let the model define spatial heterogeneity without turning
	// the runtime field itself into persisted mutable simulator state.
	for (std::size_t index = 0; index < tokens.size(); ++index) {
		try {
			values[index] = std::stod(tokens[index]);
		} catch (const std::exception&) {
			errorMessage += "BacteriaSignalGrid could not parse explicit initial value \"" + tokens[index] + "\". ";
			return false;
		}
	}
	return true;
}

bool BacteriaSignalGrid::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_width = fields->loadField("width", DEFAULT.width);
		_height = fields->loadField("height", DEFAULT.height);
		_initialSignal = fields->loadField("initialSignal", DEFAULT.initialSignal);
		_diffusionRate = fields->loadField("diffusionRate", DEFAULT.diffusionRate);
		_decayRate = fields->loadField("decayRate", DEFAULT.decayRate);
		_initialValues = fields->loadField("initialValues", DEFAULT.initialValues);
	}
	return res;
}

void BacteriaSignalGrid::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("width", _width, DEFAULT.width, saveDefaultValues);
	fields->saveField("height", _height, DEFAULT.height, saveDefaultValues);
	fields->saveField("initialSignal", _initialSignal, DEFAULT.initialSignal, saveDefaultValues);
	fields->saveField("diffusionRate", _diffusionRate, DEFAULT.diffusionRate, saveDefaultValues);
	fields->saveField("decayRate", _decayRate, DEFAULT.decayRate, saveDefaultValues);
	fields->saveField("initialValues", _initialValues, DEFAULT.initialValues, saveDefaultValues);
}

bool BacteriaSignalGrid::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_width == 0 || _height == 0) {
		errorMessage += "BacteriaSignalGrid dimensions must be greater than zero. ";
		resultAll = false;
	}
	if (_diffusionRate < 0.0 || _diffusionRate > 1.0) {
		errorMessage += "BacteriaSignalGrid diffusion rate must stay in the [0,1] interval. ";
		resultAll = false;
	}
	if (_decayRate < 0.0 || _decayRate > 1.0) {
		errorMessage += "BacteriaSignalGrid decay rate must stay in the [0,1] interval. ";
		resultAll = false;
	}

	std::vector<double> values;
	resultAll &= buildInitialField(values, errorMessage);
	return resultAll;
}

void BacteriaSignalGrid::_createReportStatisticsDataDefinitions() {
}

void BacteriaSignalGrid::_createEditableDataDefinitions() {
}

void BacteriaSignalGrid::_createOthersDataDefinitions() {
}
