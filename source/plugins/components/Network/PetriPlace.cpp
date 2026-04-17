#include "plugins/components/Network/PetriPlace.h"

PetriPlace::PetriPlace(Model* model, std::string name) : DefaultNode(model, Util::TypeOf<PetriPlace>(), name) {
}

unsigned int PetriPlace::getTokens(std::string color) const {
	auto it = _tokensByColor->find(color);
	if (it == _tokensByColor->end()) {
		return 0;
	}
	return it->second;
}

void PetriPlace::addTokens(unsigned int quantity, std::string color) {
	(*_tokensByColor)[color] += quantity;
}

bool PetriPlace::removeTokens(unsigned int quantity, std::string color) {
	unsigned int tokens = getTokens(color);
	if (tokens < quantity) {
		return false;
	}
	(*_tokensByColor)[color] = tokens - quantity;
	return true;
}

std::map<std::string, unsigned int>* PetriPlace::getAllTokens() {
	return _tokensByColor;
}

PluginInformation* PetriPlace::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<PetriPlace>(), &PetriPlace::LoadInstance, &PetriPlace::NewInstance);
	info->setCategory("Network");
	info->setDescriptionHelp("Petri net place with colored tokens.");
	info->setReceiveTransfer(true); // Petri paces do not need to form a process flow
	info->setSendTransfer(true);
	return info;
}

ModelComponent* PetriPlace::LoadInstance(Model* model, PersistenceRecord *fields) {
	PetriPlace* component = new PetriPlace(model);
	component->_loadInstance(fields);
	return component;
}

ModelDataDefinition* PetriPlace::NewInstance(Model* model, std::string name) {
	return new PetriPlace(model, name);
}

bool PetriPlace::_loadInstance(PersistenceRecord *fields) {
	bool res = DefaultNode::_loadInstance(fields);
	if (res) {
		_tokensByColor->clear();
		unsigned int colors = fields->loadField("petriColors", 0u);
		for (unsigned int i = 0; i < colors; i++) {
			std::string color = fields->loadField("petriColor" + Util::StrIndex(i), "default");
			unsigned int qty = fields->loadField("petriTokens" + Util::StrIndex(i), 0u);
			(*_tokensByColor)[color] = qty;
		}
	}
	return res;
}

void PetriPlace::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	DefaultNode::_saveInstance(fields, saveDefaultValues);
	fields->saveField("petriColors", _tokensByColor->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (const auto& pair : *_tokensByColor) {
		fields->saveField("petriColor" + Util::StrIndex(i), pair.first, "default", saveDefaultValues);
		fields->saveField("petriTokens" + Util::StrIndex(i), pair.second, 0u, saveDefaultValues);
		i++;
	}
}
