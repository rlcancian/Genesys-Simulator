#pragma once

#include <string>
#include <sstream>
#include <functional>
#include <list>
#include <stdexcept>
#include <type_traits>
#include "../util/Util.h"
#include "../util/List.h"

//namespace GenesysKernel {


/**
 * @brief Base metadata for kernel-side control/response abstractions.
 *
 * Historically, the experiment layer in GenESyS evolved from the basic
 * ModelSimulation controls toward a more generic mechanism capable of exposing
 * arbitrary getters and setters from model-related classes.
 *
 * Despite the legacy name, this base currently lives in the simulation kernel
 * and supports kernel-side experiment/control abstractions rather than a
 * user-interface property editor.
 */
class PropertyGenesysBase {
public:
    PropertyGenesysBase(std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) {
		_className=className;
		_elementName=elementName;
		_propertyName=propertyName;
		_whatsThis=whatsThis;
		_isList=isList;
        _isClass=isClass;
        _isEnum=isEnum;
	}
	virtual ~PropertyGenesysBase() = default;
public:
	std::string whatsThis() const { return _whatsThis; }
	std::string getClassname() const { return _className; }
	std::string getElementName() const { return _elementName; }
	std::string getName() const { return _propertyName; }
	std::string propertyType() const { return _propertyType; }
    bool getIsList() const { return _isList; }
    bool getIsClass() const { return _isClass; }
    bool getIsEnum() const { return _isEnum; }
	void setWhatsThis(const std::string text);
	void setName(std::string name) { _propertyName = name; }
protected:
	std::string _elementName;
	std::string _className;
	std::string _propertyName;
	std::string _propertyType;
	std::string _whatsThis;
	bool _isList;
    bool _isClass;
    bool _isEnum;
};

class PropertyGenesys: public PropertyGenesysBase {
public:
	PropertyGenesys(std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", std::string propertyId="", std::string toolTip="", std::string statusTip="")
		: PropertyGenesysBase(className, elementName, propertyName, whatsThis){
		_propertyId=propertyId;
		_toolTip=toolTip;
		_statusTip=statusTip;
	}
	virtual ~PropertyGenesys() = default;
public:
	std::list<PropertyGenesys *> subProperties() const { return _subProperties; };
	std::string toolTip() const { return _toolTip; }
	std::string statusTip() const { return _statusTip; }
	std::string propertyName() const { return _propertyName; }
	std::string propertyId() const { return _propertyId; }
	bool isEnabled() const { return _isEnabled; }
	bool isModified() const { return _isModified; }
	bool isSubProperty()const { return _isSubProperty; }
	bool hasValue() const { return _hasValue; }
	std::string valueText() const { return "?"; } // TODO
	void setToolTip(const std::string text);
	void setStatusTip(const std::string text);
	void setWhatsThis(const std::string text);
	void setPropertyName(const std::string text);
	void setPropertyId(const std::string text);
	void setEnabled(bool enable);
	void setModified(bool modified);
public:
	virtual bool compare(PropertyGenesys* otherProperty)const { return true; } // TODO
public:
	void addSubProperty(PropertyGenesys *property) {}
	void insertSubProperty(PropertyGenesys *property, PropertyGenesys *afterProperty) {}
	void removeSubProperty(PropertyGenesys *property) {}
protected:
	void propertyChanged() {}
protected:
	//friend class QtAbstractPropertyManager;
	std::string _className;
	std::string _propertyName;
	std::string _whatsThis;
	std::string _propertyId;
	std::string _toolTip;
	std::string _statusTip;
	std::list<PropertyGenesys*> _subProperties;
	bool _isEnabled;
	bool _isModified;
	bool _hasValue;
	bool _isSubProperty;
};

/**
 * @brief Read-only kernel-side simulation response abstraction.
 *
 * This is the intended kernel-level base for generic getter-based access to
 * observable model attributes, statistics and experiment responses.
 */
class SimulationResponse: public PropertyGenesysBase {
public:
    SimulationResponse(std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false)
        : PropertyGenesysBase(className, elementName, propertyName, whatsThis, isList, isClass, isEnum) {
    }
    virtual ~SimulationResponse() = default;
public:
	std::string show() const {
		std::string msg = "classname="+_className+ ", elementName="+_elementName+", name=\"" + _propertyName + "\"";
		msg += ", value="+getValue();
		return msg;
	}
	virtual std::string getValue() const = 0;
	virtual List<std::string>* getStrValues() { return nullptr; };
};

/**
 * @brief Read/write kernel-side simulation control abstraction.
 *
 * SimulationControl extends SimulationResponse with setter-based mutation and
 * therefore represents writable kernel-side experiment controls.
 */
class SimulationControl: public SimulationResponse {
public:
    SimulationControl(std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false,  bool isClass=false, bool isEnum=false)
        : SimulationResponse(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
	}
	std::string show() const {
		std::string msg = "classname="+_className+ ", elementName="+_elementName+", name=\""+_propertyName+"\"";
		if (_readonly) {
			msg += " (readonly)";
		}
		//msg += ", type="+_propertyType;
		msg += ", value="+getValue();
		return msg;
	}
	bool isReadOnly() const { return _readonly; }
public:
    virtual void setValue(std::string value, bool remove=false) = 0;
    virtual List<SimulationControl*>* getProperties(int index=0) { return nullptr; };
    virtual bool hasObjectInstance() const { return true; }
    virtual bool ensureObjectInstance() { return hasObjectInstance(); }
    virtual List<SimulationControl*>* getEditableProperties(int index=0) {
        if (getIsClass() && !hasObjectInstance()) {
            if (!ensureObjectInstance()) {
                return nullptr;
            }
        }
        return getProperties(index);
    }
protected:
	void _ensureWritable(const char* operation) const {
		if (_readonly) {
			throw std::logic_error(std::string("Cannot ") + operation + " readonly SimulationControl \"" + _propertyName + "\"");
		}
	}
	bool _readonly = false;
};

// -----------------------------------------------------------

template<typename Class, typename T>
std::function<T()> DefineSimulationGetter(Class* object, T(Class::*function)() const) {
	return std::bind(function, object);
}

template<typename Class, typename T>
std::function<T()> DefineSimulationGetter(Class* object, T(Class::*function)()) {
	return std::bind(function, object);
}

template<typename Class, typename T>
std::function<void(T)> DefineSimulationSetter(Class* object, void (Class::*function)(T)) {
	return std::bind(function, object, std::placeholders::_1);
}

typedef std::function<std::string()> GetterString;
typedef std::function<void(std::string)> SetterString;
class SimulationControlString: public SimulationControl {
public:
//	SimulationControlString(GetterString getter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="") : SimulationControl(className, elementName, propertyName, whatsThis) {
//		SimulationControlString(getter, nullptr, className, propertyName, whatsThis);
//	}
    SimulationControlString(GetterString getter, SetterString setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum) {
		_getter= getter;
		_setter = setter;
		_readonly = setter == nullptr;
		_propertyType = Util::TypeOf<std::string>();
	}
public:
	virtual std::string getValue() const override { return _getter(); }
    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlString setter is not defined");
		}
		_setter(value);
	};
private:
	GetterString _getter;
	SetterString _setter;
};


// -----------------------------------------------------------

typedef std::function<double()> GetterDouble;
typedef std::function<void(double)> SetterDouble;

class SimulationResponseDouble: public SimulationResponse {
public:
    SimulationResponseDouble(GetterDouble getter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false)
        : SimulationResponse(className, elementName, propertyName, whatsThis, isList, isClass, isEnum) {
        _getter = getter;
        _propertyType = Util::TypeOf<double>();
    }
public:
    virtual std::string getValue() const override { return std::to_string(_getter()); }
private:
    GetterDouble _getter;
};

class SimulationControlDouble: public SimulationControl {
public:
//	SimulationControlDouble(GetterDouble getter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="") : SimulationControl(className, elementName, propertyName, whatsThis) {
//		SimulationControlDouble(getter, nullptr, className, propertyName, whatsThis);
//	}
    SimulationControlDouble(GetterDouble getter, SetterDouble setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		_getter= getter;
		_setter = setter;
		_readonly = setter == nullptr;
		_propertyType = Util::TypeOf<double>();
	}
public:
	virtual std::string getValue() const override { return std::to_string(_getter()); }
    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlDouble setter is not defined");
		}
		_setter(std::stod(value));
	};
private:
	GetterDouble _getter;
	SetterDouble _setter;
};

// -----------------------------------------------------------

typedef std::function<bool()> GetterBool;
typedef std::function<void(bool)> SetterBool;
class SimulationControlBool: public SimulationControl {
public:
//	SimulationControlBool(GetterBool getter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="") : SimulationControl(className, elementName, propertyName, whatsThis) {
//		SimulationControlBool(getter, nullptr, className, propertyName, whatsThis);
//	}
    SimulationControlBool(GetterBool getter, SetterBool setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		_getter= getter;
		_setter = setter;
		_readonly = setter == nullptr;
		_propertyType = Util::TypeOf<bool>();
	}
public:
	virtual std::string getValue() const override { return std::to_string(_getter()); }
    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlBool setter is not defined");
		}
		std::istringstream auxStr(value);
		auxStr >> std::boolalpha;
		bool boolVal = false;
		if (!(auxStr >> boolVal)) {
			boolVal = std::stoi(value) != 0;
		}
		_setter(boolVal);
	};
private:
	GetterBool _getter;
	SetterBool _setter;
};


// -----------------------------------------------------------

typedef std::function<unsigned int()> GetterUInt;
typedef std::function<void(unsigned int)> SetterUInt;
class SimulationControlUInt: public SimulationControl {
public:
//	SimulationControlUInt(GetterUInt getter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="") : SimulationControl(className, elementName, propertyName, whatsThis) {
//		SimulationControlUInt(getter, nullptr, className, propertyName, whatsThis);
//	}
    SimulationControlUInt(GetterUInt getter, SetterUInt setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		_getter= getter;
		_setter = setter;
		_readonly = setter == nullptr;
		_propertyType = Util::TypeOf<unsigned int>();
	}
public:
	virtual std::string getValue() const override { return std::to_string(_getter()); }
    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlUInt setter is not defined");
		}
		_setter(std::stoul(value));
	};
private:
	GetterUInt _getter;
	SetterUInt _setter;
};

// -----------------------------------------------------------

typedef std::function<unsigned short()> GetterUShort;
typedef std::function<void(unsigned short)> SetterUShort;
class SimulationControlUShort: public SimulationControl {
public:
//	SimulationControlUShort(GetterUShort getter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="") : SimulationControl(className, elementName, propertyName, whatsThis) {
//		SimulationControlUShort(getter, nullptr, className, propertyName, whatsThis);
//	}
	    SimulationControlUShort(GetterUShort getter, SetterUShort setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
			_getter= getter;
			_setter = setter;
			_readonly = setter == nullptr;
			_propertyType = Util::TypeOf<unsigned short>();
		}
public:
	virtual std::string getValue() const override { return std::to_string(_getter()); }
    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlUShort setter is not defined");
		}
		_setter(std::stoul(value));
	};
private:
	GetterUShort _getter;
	SetterUShort _setter;
};


// -----------------------------------------------------------

typedef std::function<int()> GetterInt;
typedef std::function<void(int)> SetterInt;
class SimulationControlInt: public SimulationControl {
public:
//	SimulationControlInt(GetterInt getter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="") : SimulationControl(className, elementName, propertyName, whatsThis) {
//		SimulationControlInt(getter, nullptr, className, propertyName, whatsThis);
//	}
	    SimulationControlInt(GetterInt getter, SetterInt setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
			_getter= getter;
			_setter = setter;
			_readonly = setter == nullptr;
			_propertyType = Util::TypeOf<int>();
		}
public:
	virtual std::string getValue() const override { return std::to_string(_getter()); }
    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlInt setter is not defined");
		}
		_setter(std::stoi(value));
	};
private:
	GetterInt _getter;
	SetterInt _setter;
};

// -----------------------------------------------------------

typedef std::function<Util::TimeUnit()> GetterTimeUnit;
typedef std::function<void(Util::TimeUnit)> SetterTimeUnit;
class SimulationControlTimeUnit: public SimulationControl {
public:
//	SimulationControlTimeUnit(GetterTimeUnit getter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="") : SimulationControl(className, elementName, propertyName, whatsThis) {
//		SimulationControlTimeUnit(getter, nullptr, className, propertyName, whatsThis);
//	}
    SimulationControlTimeUnit(GetterTimeUnit getter, SetterTimeUnit setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		_getter= getter;
		_setter = setter;
		_readonly = setter == nullptr;
		_propertyType = Util::TypeOf<Util::TimeUnit>();
	}
public:
	virtual std::string getValue() const override {
		int intVal = static_cast<int>(_getter());
		return std::to_string(intVal);
	}
    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlTimeUnit setter is not defined");
		}
		int intVal = std::stoul(value);
		_setter(static_cast<Util::TimeUnit>(intVal));
    };
private:
	GetterTimeUnit _getter;
	SetterTimeUnit _setter;
};


// TODO(genesys|kernel-controls|migration): This compatibility alias keeps the current
// kernel API working while the codebase migrates from the legacy PropertyBase
// naming toward explicit SimulationResponse/SimulationControl types.
typedef SimulationControl PropertyBase;

// -----------------------------------------------------------

template <typename T>
using GetterGeneric = std::function<T()>;
template <typename T>
using SetterGeneric = std::function<void(T)>;
template <typename T>
using AdderGeneric = std::function<void(T)>;
template <typename T>
using RemoverGeneric = std::function<void(T)>;

template <typename T>
class SimulationControlGeneric: public SimulationControl {
public:
    SimulationControlGeneric(GetterGeneric<T> getter, SetterGeneric<T> setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		_getter= getter;
		_setter = setter;
		_readonly = setter == nullptr;
		_propertyType = Util::TypeOf<T>();
	}
public:
	virtual std::string getValue() const override {
		T tVal = static_cast<T>(_getter());

		std::string strVal;
		std::ostringstream auxStr;
		auxStr << tVal;
		strVal = auxStr.str();

		return strVal;
	}

    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlGeneric setter is not defined");
		}
		std::istringstream auxStr(value);
		T tVal;
		auxStr >> tVal;

		_setter(static_cast<T>(tVal));
    };

private:
	GetterGeneric<T> _getter;
	SetterGeneric<T> _setter;
};

// TODO: remove typename E
template <typename T, typename E>
class SimulationControlGenericEnum: public SimulationControl {
public:
    SimulationControlGenericEnum(GetterGeneric<T> getter, SetterGeneric<T> setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=false, bool isEnum=true) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		static_assert(std::is_enum<T>::value, "SimulationControlGenericEnum requires T to be an enum");
		_getter= getter;
		_setter = setter;
		_readonly = setter == nullptr;
		_propertyType = Util::TypeOf<T>();
        _isEnum=true;
	}
public:
	virtual std::string getValue() const override {
		int intVal = static_cast<int>(_getter());
		int max_i = static_cast<int>(T::num_elements);
		if (intVal < 0 || intVal >= max_i) {
			return "";
		}
		return E::convertEnumToStr(static_cast<T>(intVal));
	}

	    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlGenericEnum setter is not defined");
		}
		int intVal = std::stoul(value);
		_setter(static_cast<T>(intVal));
	};

    virtual List<std::string>* getStrValues() override {
        List<std::string>* strOptions = new List<std::string>();
        int max_i = static_cast<int>(T::num_elements);;
		for (int i=0; i<max_i; i++) {
            std::string value = E::convertEnumToStr((T)i);
            strOptions->insert(value);
        }
        return strOptions;
    }

private:
	GetterGeneric<T> _getter;
	SetterGeneric<T> _setter;
};

// TODO: remove typename C
template <typename T, typename M, typename C>
class SimulationControlGenericClass: public SimulationControl {
public:
    SimulationControlGenericClass(M model, GetterGeneric<T> getter, SetterGeneric<T> setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=true, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		static_assert(std::is_pointer<T>::value, "SimulationControlGenericClass requires pointer type T");
		_model = model;
		_getter= getter;
		_setter = setter;
		_readonly = setter == nullptr;
		_propertyType = Util::TypeOf<C>();
	}
public:
	virtual std::string getValue() const override {
		T tVal = static_cast<T>(_getter());
        std::string strVal;

        if (tVal != nullptr) {
            strVal = tVal->getName();
        } else {
			strVal = "";
        };

		return strVal;
	}

    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlGenericClass setter is not defined");
		}
		bool exists = false;
        // value.pop_back();
		T newVal;

        for (auto modeldata : *_model->getDataManager()->getDataDefinitionList(_propertyType)->list()) {
            if (modeldata->getName() == value) {
                exists = true;
                newVal = static_cast<T>(modeldata);
                break;
            };
        };

		if (!exists) {
			newVal = new C(_model, value);
		 	_model->getDataManager()->insert(newVal);
		};

		_setter(newVal);
	};

    virtual List<SimulationControl*>* getProperties(int index=0) override {
        T tVal = static_cast<T>(_getter());

        if (tVal) {
            return tVal->getProperties();
        } else {
            return nullptr;
        }
    }

private:
	M _model;
	GetterGeneric<T> _getter;
	SetterGeneric<T> _setter;
};

// TODO: remove typename C
template <typename T, typename M, typename C>
class SimulationControlGenericClassNotDC: public SimulationControl {
public:
    using Creator = std::function<T(M)>;
    SimulationControlGenericClassNotDC(M model, GetterGeneric<T> getter, SetterGeneric<T> setter, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=false, bool isClass=true, bool isEnum=false, Creator creator=nullptr) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		static_assert(std::is_pointer<T>::value, "SimulationControlGenericClassNotDC requires pointer type T");
        _model = model;
        _getter= getter;
        _setter = setter;
        _creator = creator;
        _readonly = setter == nullptr;
        _propertyType = Util::TypeOf<C>();
    }
public:
    virtual std::string getValue() const override {
        T tVal = static_cast<T>(_getter());
        std::string strVal;

        if (tVal != nullptr) {
            strVal = tVal->getName();
        } else {
            strVal = "";
        };

        return strVal;
    }

    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable("set value of");
		if (!_setter) {
			throw std::logic_error("SimulationControlGenericClassNotDC setter is not defined");
		}
        T newVal;

        // TODO: criar apenas se já não estiver definido?
        newVal = new C(_model, value);

        _setter(newVal);
    };

    virtual bool hasObjectInstance() const override {
        return static_cast<T>(_getter()) != nullptr;
    }

    virtual bool ensureObjectInstance() override {
		_ensureWritable("ensure instance of");
		if (!_setter) {
			throw std::logic_error("SimulationControlGenericClassNotDC setter is not defined");
		}

        T current = static_cast<T>(_getter());
        if (current != nullptr) {
            return true;
        }

        T newVal;
        if (_creator != nullptr) {
            newVal = _creator(_model);
        } else {
            newVal = new C(_model, "");
        }
        _setter(newVal);
        return static_cast<T>(_getter()) != nullptr;
    }

    virtual List<SimulationControl*>* getProperties(int index=0) override {
        T tVal = static_cast<T>(_getter());

        if (tVal) {
            return tVal->getProperties();
        } else {
            return nullptr;
        }
    }

private:
    M _model;
    GetterGeneric<T> _getter;
    SetterGeneric<T> _setter;
    Creator _creator;
};

template <typename T, typename M, typename C>
class SimulationControlGenericList: public SimulationControl {
public:
    SimulationControlGenericList(M model, GetterGeneric<List<T>*> getter, AdderGeneric<T> adder, RemoverGeneric<T> remover, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=true, bool isClass=false, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		_model = model;
		_getter= getter;
		_adder = adder;
        _remover = remover;
		_readonly = adder == nullptr;
		_propertyType = Util::TypeOf<C>();
	}
public:
	virtual std::string getValue() const override {
		List<T>* tVal = static_cast<List<T>*>(_getter());
		std::string strVal = std::to_string(tVal->size()) + " Items";

		return strVal;
	}

    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable(remove ? "remove value from" : "add value to");
        T newVal;

        std::istringstream auxStr(value);
        auxStr >> newVal;
        newVal = static_cast<T>(newVal);

	        if (remove) {
				if (!_remover) {
					throw std::logic_error("SimulationControlGenericList remover is not defined");
				}
	            _remover(newVal);
	        } else {
				if (!_adder) {
					throw std::logic_error("SimulationControlGenericList adder is not defined");
				}
	            bool exists = false;
				List<T>* tVal = static_cast<List<T>*>(_getter());
	            for (auto element : *tVal->list()) {
					std::ostringstream auxElement;
					auxElement << element;
	                if (value == auxElement.str()) {
	                    exists = true;
	                    break;
	                }
	            }

            if (!exists) {
                _adder(newVal);
            }
        }
    };

    virtual List<std::string>* getStrValues() override {
        List<T>* tVal = static_cast<List<T>*>(_getter());
        List<std::string>* strOptions = new List<std::string>();

        for (auto element : *tVal->list()) {
            std::string strVal;
            std::ostringstream auxStr;
            auxStr << element;
            strVal = auxStr.str();

            strOptions->insert(strVal);
        }
        return strOptions;
    }

private:
	M _model;
	GetterGeneric<List<T>*> _getter;
	AdderGeneric<T> _adder;
    RemoverGeneric<T> _remover;
};

template <typename T, typename M, typename C>
class SimulationControlGenericListPointer: public SimulationControl {
public:
    SimulationControlGenericListPointer(M model, GetterGeneric<List<T>*> getter, AdderGeneric<T> adder, RemoverGeneric<T> remover, std::string className, std::string elementName, std::string propertyName, std::string whatsThis="", bool isList=true, bool isClass=true, bool isEnum=false) : SimulationControl(className, elementName, propertyName, whatsThis, isList, isClass, isEnum){
		static_assert(std::is_pointer<T>::value, "SimulationControlGenericListPointer requires pointer type T");
        _model = model;
        _getter= getter;
        _adder = adder;
        _remover = remover;
        _readonly = adder == nullptr;
        _propertyType = Util::TypeOf<C>();
    }
public:
    virtual std::string getValue() const override {
        List<T>* tVal = static_cast<List<T>*>(_getter());
        std::string strVal = std::to_string(tVal->size()) + " Items";

        return strVal;
    }

	    virtual void setValue(std::string value, bool remove=false) override {
		_ensureWritable(remove ? "remove value from" : "add value to");
		List<T>* tVal = static_cast<List<T>*>(_getter());
		T existingVal = nullptr;
		for (auto element : *tVal->list()) {
			if (element != nullptr && element->getName() == value) {
				existingVal = element;
				break;
			}
		}

	        if (remove) {
				if (!_remover) {
					throw std::logic_error("SimulationControlGenericListPointer remover is not defined");
				}
				if (existingVal != nullptr) {
					_remover(existingVal);
				}
	        } else {
				if (!_adder) {
					throw std::logic_error("SimulationControlGenericListPointer adder is not defined");
				}
	            if (existingVal == nullptr) {
					T newVal = new C(_model, value);
	                _adder(newVal);
            }
        }
    };

	    virtual List<SimulationControl*>* getProperties(int index=0) override {
	        List<T>* tVal = static_cast<List<T>*>(_getter());
	        T selectedElement = nullptr;

        int current_index = 0;
        for (auto element : *tVal->list()) {
            if (current_index == index) {
                selectedElement = element;
            }
            current_index++;
        }

        if (selectedElement) {
            return selectedElement->getProperties();
        } else {
            return nullptr;
        }
    }

    virtual List<std::string>* getStrValues() override {
        List<T>* tVal = static_cast<List<T>*>(_getter());
        List<std::string>* strOptions = new List<std::string>();

        for (auto element : *tVal->list()) {
            std::string strVal;
            if (element != nullptr) {
                strVal = element->getName();
            } else {
                strVal = "";
            };

            strOptions->insert(strVal);
        }
        return strOptions;
    }

private:
    M _model;
    GetterGeneric<List<T>*> _getter;
    AdderGeneric<T> _adder;
    RemoverGeneric<T> _remover;
};

//namespace\\}
