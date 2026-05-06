/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelDataDefinitionAssociations.h
 * Author: Codex
 *
 * Created on 6 de Maio de 2026
 */

#ifndef MODELDATADEFINITIONASSOCIATIONS_H
#define MODELDATADEFINITIONASSOCIATIONS_H

#include <map>
#include <string>
#include <vector>

class Attribute;
class Model;
class ModelDataDefinition;

/*!
 * Owns the internal/attached registries and the new lifecycle buckets for a ModelDataDefinition.
 *
 * The class starts as a compatibility layer around the current internal/attached storage so the
 * kernel can move bucket by bucket without breaking direct callers that still inspect the legacy
 * aggregate maps.
 */
class ModelDataDefinitionAssociations {
public:
	explicit ModelDataDefinitionAssociations(ModelDataDefinition* owner);
	~ModelDataDefinitionAssociations();

public:
	std::map<std::string, ModelDataDefinition*>* getInternalData() const;
	std::map<std::string, ModelDataDefinition*>* getAttachedData() const;
	ModelDataDefinition* getInternalData(const std::string& key) const;

	void clearAll();
	void clearInternalData();
	void clearAttachedData();

	void internalDataInsert(const std::string& key, ModelDataDefinition* data);
	void internalDataRemove(const std::string& key);
	void attachedDataInsert(const std::string& key, ModelDataDefinition* data);
	void attachedDataRemove(const std::string& key);
	void attachedAttributesInsert(const std::vector<std::string>& neededNames);
	void checkCreateAttachedReferencedDataDefinition(const std::string& expression);

	void createInternalAndAttachedData();
	void createInternalStatisticReporters();
	void createAttachedAttributes();
	void createNonEditableDataDefinitions();
	void createEditableDataDefinitions();

	void insertStatisticReporter(const std::string& key, ModelDataDefinition* data);
	void removeStatisticReporter(const std::string& key);
	void clearStatisticReporters();

	void insertMandatoryAttachedAttribute(const std::string& key, ModelDataDefinition* data);
	void removeMandatoryAttachedAttribute(const std::string& key);
	void clearMandatoryAttachedAttributes();

	void insertMandatoryEditableDataDefinition(const std::string& key, ModelDataDefinition* data);
	void removeMandatoryEditableDataDefinition(const std::string& key);
	void clearMandatoryEditableDataDefinitions();

	void insertOptionalEditableDataDefinition(const std::string& key, ModelDataDefinition* data);
	void removeOptionalEditableDataDefinition(const std::string& key);
	void clearOptionalEditableDataDefinitions();

	void insertMandatoryNonEditableDataDefinition(const std::string& key, ModelDataDefinition* data);
	void removeMandatoryNonEditableDataDefinition(const std::string& key);
	void clearMandatoryNonEditableDataDefinitions();

	void insertOptionalNonEditableDataDefinition(const std::string& key, ModelDataDefinition* data);
	void removeOptionalNonEditableDataDefinition(const std::string& key);
	void clearOptionalNonEditableDataDefinitions();

private:
	void _deleteOwnedInternalData(ModelDataDefinition* data);
	void _rebuildLegacyViews();

private:
	ModelDataDefinition* _owner = nullptr;
	std::map<std::string, ModelDataDefinition*> _internalData;
	std::map<std::string, ModelDataDefinition*> _attachedData;
	std::map<std::string, ModelDataDefinition*> _statisticReporters;
	std::map<std::string, ModelDataDefinition*> _mandatoryAttachedAttributes;
	std::map<std::string, ModelDataDefinition*> _mandatoryEditableDataDefinitions;
	std::map<std::string, ModelDataDefinition*> _optionalEditableDataDefinitions;
	std::map<std::string, ModelDataDefinition*> _mandatoryNonEditableDataDefinitions;
	std::map<std::string, ModelDataDefinition*> _optionalNonEditableDataDefinitions;
};

#endif /* MODELDATADEFINITIONASSOCIATIONS_H */
