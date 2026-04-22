/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SPICECircuit.h
 * Author: bnmfw & joaomai
 *
 * Created on 10 de outubro de 2023, 23:00
 */

#pragma once

#include "kernel/simulator/ModelComponent.h"
#include "plugins/data/ElectronicsSimulation/SPICERunner.h"
#include "plugins/components/ElectronicsSimulation/SPICENode.h"

/*!
 This component ...
 */
class SPICECircuit : public ModelComponent {
public: /// constructors
	SPICECircuit(Model* model, std::string name = "");
	virtual ~SPICECircuit() = default;

public: /// new public user methods for this components
	void setRunner(SPICERunner* runner);
	SPICERunner* getRunner();

public: /// virtual public methods
	virtual std::string show() override;

public: /// static public methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
	// Added
	static std::vector<std::string> split(std::string word, char split_char);

protected: /// virtual protected method that must be overriden
	void BuildCircuit(std::string description, unsigned int id, std::vector<std::string> the_params = {""}, std::string the_model = "", std::string the_model_file = "");
	void UpdateConnections();
	virtual void Build() { id = counter++; build = [this]() {this->Build();};}
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override; ///< This method is only for ModelComponents, not ModelDataElements
	virtual void _createInternalAndAttachedData() override;


protected: /// virtual protected methods that could be overriden by derived classes, if needed
	/*! This method is called by ModelChecker during model check. The component should check itself to verify if user parameters are ok (ex: correct syntax for the parser) and everithing in its parameters allow the model too run without errors in this component */
	// virtual bool _check(std::string& errorMessage);
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	// virtual ParserChangesInformation* _getParserChangesInformation();
	/*! This method is called by ModelSimulation when initianting the replication. The model should set all value for a new replication (Ex: setting back to 0 any internal counter, clearing lists, etc. */
	// virtual void _initBetweenReplications();
	/*! This method is called by ModelChecker and is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	// virtual void _createInternalAndAttachedData(); /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	/*! This method is not used yet. It should be usefull for new UIs */
	// virtual void _addSimulationControl(SimulationControl* property);

private: /// new private user methods
	// Spice subcircuit declaration, if one exists
    std::string spice_template;
    // Name of component
    std::string spice_name;
    // Params of the component, such as resistance or voltage
    std::vector<std::string> params;
    // Pin map of the component
    std::vector<std::string> pins;
    // Instace of the component in spice
    std::string *spice_instance;
    // Model of the component, if one exists (the model name should also be the name of the .cir file)
    std::string model;
    std::string model_file;
    // Spice compiler
    SPICERunner *compiler = nullptr;

	static unsigned int counter;
	bool plain_circuit;
protected:
    unsigned int id;
	std::function<void()> build;

private: /// Attributes that should be loaded or saved with this component (Persistent Fields)

	/// Default values for the attributes. Used on initing, loading and saving
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
	} DEFAULT;
	std::string _someString = DEFAULT.someString;
	unsigned int _someUint = DEFAULT.someUint;

private: /// internal DataElements (Composition)

private: /// attached DataElements (Agrregation)
	// ...
};

class Resistor : public SPICECircuit {
public:
	Resistor(Model* model, std::string name = ""): SPICECircuit(model, name) {
		id = counter++;
		build = [this]() {this->Build();};
	}
	void Build() override;
	void setResistance(float resistance);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float someResistance = 1000;
	} DEFAULT;

    static unsigned int counter;
	float resistance = DEFAULT.someResistance;
};

class Vsource : public SPICECircuit {
public:
	Vsource(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setVoltage(float voltage);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float someVoltage = 5;
	} DEFAULT;

	float voltage = DEFAULT.someVoltage;
    static unsigned int counter;
};

class Vpulse : public SPICECircuit {
public:
	Vpulse(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setVoltage(float voltage);
	void setFreq(float freq);
	void setSlope(float slope);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float someVoltage = 5;
		const float someFreq = 60;
		const float someSlope = 1e-11;
	} DEFAULT;

    static unsigned int counter;
	float voltage = DEFAULT.someVoltage;
	float freq = DEFAULT.someFreq;
	float slope = DEFAULT.someSlope;
};

class Vsine : public SPICECircuit {
public:
	Vsine(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setVoltage(float voltage);
	void setFreq(float freq);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float someVoltage = 5;
		const float someFreq = 60;
	} DEFAULT;

    static unsigned int counter;
	float voltage = DEFAULT.someVoltage;
	float freq = DEFAULT.someFreq;
};

class Capacitor : public SPICECircuit {
public:
	Capacitor(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void setCapacitance(float capacitance);
	void Build() override;

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float someCapacitance = 0.01;
	} DEFAULT;

    static unsigned int counter;
	float capacitance = DEFAULT.someCapacitance;
};

class Diode : public SPICECircuit {
public:
	Diode(Model* model, std::string name = ""): SPICECircuit(model, name) {
		id = counter++; build = [this]() {this->Build();};
	}
	void Build() override;
	void setElectricalModel(std::string electricalModel);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const std::string someElectricalModel = "diode";
	} DEFAULT;

	static unsigned int counter;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class PMOS: public SPICECircuit {
public:
	PMOS(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setWidth(float width);
	void setLength(float length);
	void setElectricalModel(std::string electricalModel);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float someWidth = 140e-9;
		const float someLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float width = DEFAULT.someWidth;
	float length = DEFAULT.someLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class NMOS: public SPICECircuit {
public:
	NMOS(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setWidth(float width);
	void setLength(float length);
	void setElectricalModel(std::string electricalModel);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float someWidth = 70e-9;
		const float someLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float width = DEFAULT.someWidth;
	float length = DEFAULT.someLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class NOT : public SPICECircuit {
public:
	NOT(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setPMOSWidth(float PMOSWidth);
	void setPMOSLength(float length);
	void setNMOSWidth(float NMOSWidth);
	void setNMOSLength(float NMOSLength);
	void setElectricalModel(std::string electricalModel);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float somePMOSWidth = 140e-9;
		const float somePMOSLength = 32e-9;
		const float someNMOSWidth = 70e-9;
		const float someNMOSLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float PMOSWidth = DEFAULT.somePMOSWidth;
	float PMOSLength = DEFAULT.somePMOSLength;
	float NMOSWidth = DEFAULT.someNMOSWidth;
	float NMOSLength = DEFAULT.someNMOSLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class NAND : public SPICECircuit {
public:
	NAND(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setPMOSWidth(float PMOSWidth);
	void setPMOSLength(float length);
	void setNMOSWidth(float NMOSWidth);
	void setNMOSLength(float NMOSLength);
	void setElectricalModel(std::string electricalModel);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
    const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float somePMOSWidth = 140e-9;
		const float somePMOSLength = 32e-9;
		const float someNMOSWidth = 70e-9;
		const float someNMOSLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float PMOSWidth = DEFAULT.somePMOSWidth;
	float PMOSLength = DEFAULT.somePMOSLength;
	float NMOSWidth = DEFAULT.someNMOSWidth;
	float NMOSLength = DEFAULT.someNMOSLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class AND : public SPICECircuit {
public:
	AND(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setPMOSWidth(float PMOSWidth);
	void setPMOSLength(float length);
	void setNMOSWidth(float NMOSWidth);
	void setNMOSLength(float NMOSLength);
	void setElectricalModel(std::string electricalModel);
	
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
    const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float somePMOSWidth = 140e-9;
		const float somePMOSLength = 32e-9;
		const float someNMOSWidth = 70e-9;
		const float someNMOSLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float PMOSWidth = DEFAULT.somePMOSWidth;
	float PMOSLength = DEFAULT.somePMOSLength;
	float NMOSWidth = DEFAULT.someNMOSWidth;
	float NMOSLength = DEFAULT.someNMOSLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class NOR : public SPICECircuit {
public:
	NOR(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setPMOSWidth(float PMOSWidth);
	void setPMOSLength(float length);
	void setNMOSWidth(float NMOSWidth);
	void setNMOSLength(float NMOSLength);
	void setElectricalModel(std::string electricalModel);
	
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
    const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float somePMOSWidth = 140e-9;
		const float somePMOSLength = 32e-9;
		const float someNMOSWidth = 70e-9;
		const float someNMOSLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float PMOSWidth = DEFAULT.somePMOSWidth;
	float PMOSLength = DEFAULT.somePMOSLength;
	float NMOSWidth = DEFAULT.someNMOSWidth;
	float NMOSLength = DEFAULT.someNMOSLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class OR : public SPICECircuit {
public:
	OR(Model* model, std::string name = ""): SPICECircuit(model, name)  { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setPMOSWidth(float PMOSWidth);
	void setPMOSLength(float length);
	void setNMOSWidth(float NMOSWidth);
	void setNMOSLength(float NMOSLength);
	void setElectricalModel(std::string electricalModel);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
    const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float somePMOSWidth = 140e-9;
		const float somePMOSLength = 32e-9;
		const float someNMOSWidth = 70e-9;
		const float someNMOSLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float PMOSWidth = DEFAULT.somePMOSWidth;
	float PMOSLength = DEFAULT.somePMOSLength;
	float NMOSWidth = DEFAULT.someNMOSWidth;
	float NMOSLength = DEFAULT.someNMOSLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class XOR : public SPICECircuit {
public:
	XOR(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setPMOSWidth(float PMOSWidth);
	void setPMOSLength(float length);
	void setNMOSWidth(float NMOSWidth);
	void setNMOSLength(float NMOSLength);
	void setElectricalModel(std::string electricalModel);
	
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
    const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float somePMOSWidth = 140e-9;
		const float somePMOSLength = 32e-9;
		const float someNMOSWidth = 70e-9;
		const float someNMOSLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float PMOSWidth = DEFAULT.somePMOSWidth;
	float PMOSLength = DEFAULT.somePMOSLength;
	float NMOSWidth = DEFAULT.someNMOSWidth;
	float NMOSLength = DEFAULT.someNMOSLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

class XNOR : public SPICECircuit {
public:
	XNOR(Model* model, std::string name = ""): SPICECircuit(model, name) { id = counter++; build = [this]() {this->Build();};}
	void Build() override;
	void setPMOSWidth(float PMOSWidth);
	void setPMOSLength(float length);
	void setNMOSWidth(float NMOSWidth);
	void setNMOSLength(float NMOSLength);
	void setElectricalModel(std::string electricalModel);

	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	
private:
    const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
		const float somePMOSWidth = 140e-9;
		const float somePMOSLength = 32e-9;
		const float someNMOSWidth = 70e-9;
		const float someNMOSLength = 32e-9;
		const std::string someElectricalModel = "32nmCMOS";
	} DEFAULT;

    static unsigned int counter;
	float PMOSWidth = DEFAULT.somePMOSWidth;
	float PMOSLength = DEFAULT.somePMOSLength;
	float NMOSWidth = DEFAULT.someNMOSWidth;
	float NMOSLength = DEFAULT.someNMOSLength;
	std::string electricalModel = DEFAULT.someElectricalModel;
};

