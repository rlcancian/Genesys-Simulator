#ifndef COMPARTMENTEXCHANGECOMPONENT_H
#define COMPARTMENTEXCHANGECOMPONENT_H

#include <string>
#include <vector>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/*!
 * \brief Moves metabolite proxies between whole-cell regions.
 *
 * CompartmentExchangeComponent transfers a fraction of a source metabolite pool
 * in WholeCellState into a target pool on each dispatched event. The component
 * can represent didactic transport, exchange, or redistribution policies
 * between global and compartment-scoped pools without embedding that logic
 * directly into examples.
 *
 * Source and target regions follow this convention:
 * - empty region name: global WholeCellState metabolite pool;
 * - non-empty region name: compartment-scoped pool.
 */
class CompartmentExchangeComponent : public ModelComponent {
public:
	struct ExchangeRule {
		std::string label;
		std::string sourceRegion;
		std::string sourceMetaboliteKey;
		std::string targetRegion;
		std::string targetMetaboliteKey;
		double exchangeFraction = 0.0;
		double baselineAmount = 0.0;
		std::string driverPathwayKey;
		double driverScale = 1.0;
		double maxTransferAmount = 0.0;
		bool conserveMass = true;
	};

public:
	CompartmentExchangeComponent(Model* model, std::string name = "");
	virtual ~CompartmentExchangeComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setSourceRegion(std::string region);
	std::string getSourceRegion() const;
	void setSourceMetaboliteKey(std::string key);
	std::string getSourceMetaboliteKey() const;
	void setTargetRegion(std::string region);
	std::string getTargetRegion() const;
	void setTargetMetaboliteKey(std::string key);
	std::string getTargetMetaboliteKey() const;
	void setExchangeFraction(double fraction);
	double getExchangeFraction() const;
	void setBaselineAmount(double amount);
	double getBaselineAmount() const;
	void setDriverPathwayKey(std::string key);
	std::string getDriverPathwayKey() const;
	void setDriverScale(double scale);
	double getDriverScale() const;
	void setMaxTransferAmount(double amount);
	double getMaxTransferAmount() const;
	void setConserveMass(bool conserve);
	bool getConserveMass() const;
	void clearExchangeRules();
	void addExchangeRule(const std::string& label,
	                     const std::string& sourceRegion,
	                     const std::string& sourceMetaboliteKey,
	                     const std::string& targetRegion,
	                     const std::string& targetMetaboliteKey,
	                     double exchangeFraction,
	                     double baselineAmount = 0.0,
	                     const std::string& driverPathwayKey = "",
	                     double driverScale = 1.0,
	                     double maxTransferAmount = 0.0,
	                     bool conserveMass = true);
	unsigned int getExchangeRuleCount() const;
	const std::vector<ExchangeRule>& getExchangeRules() const;
	double getLastTransferAmount() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual void _createEditableDataDefinitions() override;

private:
	ExchangeRule _buildLegacyRule() const;
	std::vector<ExchangeRule> _getEffectiveRules() const;
	bool _checkRule(const ExchangeRule& rule, std::string& errorMessage, unsigned int index) const;
	double _computeTransferAmount(const ExchangeRule& rule) const;
	void _applyRule(const ExchangeRule& rule);
	double _getPoolAmount(const std::string& region, const std::string& key) const;
	void _setPoolAmount(const std::string& region, const std::string& key, double amount);
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName = "";
		std::string sourceRegion = "";
		std::string sourceMetaboliteKey = "";
		std::string targetRegion = "";
		std::string targetMetaboliteKey = "";
		double exchangeFraction = 0.0;
		double baselineAmount = 0.0;
		std::string driverPathwayKey = "";
		double driverScale = 1.0;
		double maxTransferAmount = 0.0;
		bool conserveMass = true;
		double lastTransferAmount = 0.0;
	} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	std::string _sourceRegion = DEFAULT.sourceRegion;
	std::string _sourceMetaboliteKey = DEFAULT.sourceMetaboliteKey;
	std::string _targetRegion = DEFAULT.targetRegion;
	std::string _targetMetaboliteKey = DEFAULT.targetMetaboliteKey;
	double _exchangeFraction = DEFAULT.exchangeFraction;
	double _baselineAmount = DEFAULT.baselineAmount;
	std::string _driverPathwayKey = DEFAULT.driverPathwayKey;
	double _driverScale = DEFAULT.driverScale;
	double _maxTransferAmount = DEFAULT.maxTransferAmount;
	bool _conserveMass = DEFAULT.conserveMass;
	std::vector<ExchangeRule> _exchangeRules;
	double _lastTransferAmount = DEFAULT.lastTransferAmount;
};

#endif /* COMPARTMENTEXCHANGECOMPONENT_H */
