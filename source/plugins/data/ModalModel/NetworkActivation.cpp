/*
 * File:   NetworkActivation.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#include "NetworkActivation.h"

NetworkActivationFrame::NetworkActivationFrame(unsigned int numInputs) : _inputs(numInputs) {
}

void NetworkActivationFrame::reset(unsigned int numInputs) {
	_inputs.assign(numInputs, NetworkPortValue());
}

unsigned int NetworkActivationFrame::size() const {
	return static_cast<unsigned int>(_inputs.size());
}

void NetworkActivationFrame::setPresent(unsigned int index, double value) {
	if (index < _inputs.size()) {
		_inputs[index].present = true;
		_inputs[index].value = value;
	}
}

void NetworkActivationFrame::setAbsent(unsigned int index) {
	if (index < _inputs.size()) {
		_inputs[index].present = false;
		_inputs[index].value = 0.0;
	}
}

bool NetworkActivationFrame::isPresent(unsigned int index) const {
	return index < _inputs.size() && _inputs[index].present;
}

double NetworkActivationFrame::getValue(unsigned int index) const {
	return index < _inputs.size() ? _inputs[index].value : 0.0;
}

NetworkActivationResult::NetworkActivationResult(unsigned int numOutputs) : _outputs(numOutputs) {
}

void NetworkActivationResult::reset(unsigned int numOutputs) {
	_outputs.assign(numOutputs, NetworkPortValue());
}

unsigned int NetworkActivationResult::size() const {
	return static_cast<unsigned int>(_outputs.size());
}

void NetworkActivationResult::setPresent(unsigned int index, double value) {
	if (index < _outputs.size()) {
		_outputs[index].present = true;
		_outputs[index].value = value;
	}
}

void NetworkActivationResult::setAbsent(unsigned int index) {
	if (index < _outputs.size()) {
		_outputs[index].present = false;
		_outputs[index].value = 0.0;
	}
}

bool NetworkActivationResult::isPresent(unsigned int index) const {
	return index < _outputs.size() && _outputs[index].present;
}

double NetworkActivationResult::getValue(unsigned int index) const {
	return index < _outputs.size() ? _outputs[index].value : 0.0;
}

unsigned int NetworkActivationResult::countPresent() const {
	unsigned int count = 0;
	for (const NetworkPortValue& portValue : _outputs) {
		if (portValue.present) {
			count++;
		}
	}
	return count;
}
