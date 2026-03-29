#ifndef PROPERTY_H
#define PROPERTY_H

#include "../util/Util.h"
//#include "../util/List.h"
#include <string>
#include <functional>

class SeizableItem;
class QueueableItem;

//---------------------------------------------------------------------------

// TODO(genesys|property-system|arquitetura): Revisar a sobreposicao entre PropertyBase/PropertyT e SimulationControl/SimulationResponse.
// Hoje este header define PropertyBase, enquanto SimulationControlAndResponse.h tambem expoe PropertyBase como alias.
// Quando ambos sao incluidos na mesma unidade de traducao, ha conflito nominal. Investigar unificacao ou renomeacao.
class PropertyBase {
public:

	PropertyBase(std::string classname, std::string name, std::string parentName = "") {
		_classename = classname;
		_name = name;
		_parentname = parentName;
	}
	virtual ~PropertyBase() = default;
public:

	std::string show() const {
		return "classname=" + _classename + ", name=" + _name + ", parentname=" + _parentname;
	};

	std::string getClassname() const {
		return _classename;
	};

	std::string getParentName() const {
		return _parentname;
	};

	std::string getName() const {
		return _name;
	};

	std::string getType() const {
		return _type;
	};

	void setName(const std::string&name) {
		_name = name;
	}
public: // hate it

	double getValue() const {
		return -1;
	};

	void setValue(double value) {
	};
	//virtual std::string getStringValue() const=0;
protected:
	std::string _name;
	std::string _parentname;
	std::string _classename;
	std::string _type = "none";
};

template<typename T>
struct Getter {
	typedef std::function<T() > Member;
};

template<typename Class, typename T>
typename Getter<T>::Member DefineGetter(Class * object, T(Class::*function)() const) {
	return std::bind(function, object);
}

template<typename Class, typename T>
typename Getter<T>::Member DefineGetter(Class * object, T(Class::*function)()) {
	return std::bind(function, object);
}

template<typename T>
struct Setter {
	typedef std::function<void(T) > Member;
};

template<typename Class, typename T>
typename Setter<T>::Member DefineSetter(Class * object, void (Class::*function)(T)) {
	return std::bind(function, object, std::placeholders::_1);
}

template<typename T>
class PropertyT : public PropertyBase {
public:

	PropertyT(std::string classname, std::string name, typename Getter<T>::Member getter, typename Setter<T>::Member setter, std::string parentName = "") : PropertyBase(classname, name, parentName) {
		_getter = getter;
		_setter = setter;
	};
public:

	T getValue() {
		return _getter();
	};
	void setValue(T value);
	//virtual std::string getStringValue() const override;
protected:
	typename Getter<T>::Member _getter;
	typename Setter<T>::Member _setter;
	const std::string _type = Util::TypeOf<T>();
};

//template<typename T> std::string PropertyT<T>::getStringValue() const {return std::to_string(_getter());}
//template<> std::string PropertyT<std::string>::getStringValue() const {return _getter();}
//template<> std::string PropertyT<Util::TimeUnit>::getStringValue() const {return Util::StrTimeUnitLong(_getter());}
//template<> std::string PropertyT<List<SeizableItem*>>::getStringValue() const {return "[]size:"+std::to_string(_getter().size());}
//template<> std::string PropertyT<List<QueueableItem*>>::getStringValue() const {return "[]size:"+std::to_string(_getter().size());}

/*
template<>
class PropertyT<std::string>: public PropertyBase {
public:
	PropertyT(std::string classname, std::string name, typename Getter<std::string>::Member getter, typename Setter<std::string>::Member setter, std::string parentName=""):PropertyBase(classname,name, parentName) {
		_getter = getter;
		_setter = setter;
	};
public:
	std::string getValue() {return _getter();};
	void setValue(std::string value){_setter(value);};
	virtual std::string getStringValue() const {return _getter();};
protected:
	typename Getter<std::string>::Member _getter;
	typename Setter<std::string>::Member _setter;
	const std::string _type = Util::TypeOf<std::string>();
};
template<>
class PropertyT<Util::TimeUnit>: public PropertyBase {
public:
	PropertyT(std::string classname, std::string name, typename Getter<Util::TimeUnit>::Member getter, typename Setter<Util::TimeUnit>::Member setter, std::string parentName=""):PropertyBase(classname,name, parentName) {
		_getter = getter;
		_setter = setter;
	};
public:
	Util::TimeUnit getValue() {return _getter();};
	void setValue(Util::TimeUnit value){_setter(value);};
	virtual std::string getStringValue() const {return Util::StrTimeUnitLong(_getter()); };
protected:
	typename Getter<Util::TimeUnit>::Member _getter;
	typename Setter<Util::TimeUnit>::Member _setter;
	const std::string _type = Util::TypeOf<Util::TimeUnit>();
};
 */
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------





























/*


class PropertyGetterBase {
public:
	PropertyGetterBase(std::string classname, std::string name, std::string parentName="");
public:
	std::string show() const;
	std::string getClassname() const;
	std::string getParentName() const;
	std::string getName() const;
	void setName(const std::string &name);
	std::string getType() const;
public:
	double getValue();
protected:
	std::string _name;
	std::string _parentname;
	std::string _classename;
	const std::string _type = "void";
};

class PropertySetterBase: public PropertyGetterBase {
public:
	PropertySetterBase(std::string classname, std::string name, std::string parentName="");
public:
	void setValue(double value);
};


//
// unsigned int
//
typedef std::function<unsigned int()> GetterUInt;
template<typename Class>
GetterUInt DefineGetterUInt(Class * object, unsigned int (Class::*function)() const) {
	return std::bind(function, object);
}
class PropertyGetterUInt: public PropertyGetterBase {
public:
	PropertyGetterUInt(std::string classname, std::string name, GetterUInt getter, std::string parentName="");
public:
	unsigned int getValue();

protected:
	GetterUInt _getter;
	const std::string _type = "unsigned int";
};

typedef std::function<void(unsigned int) > SetterUInt;
template<typename Class>
SetterUInt DefineSetterUInt(Class * object, void (Class::*function)(unsigned int)) {
	return std::bind(function, object, std::placeholders::_1);
}
class PropertySetterUInt: public PropertyGetterUInt {
public:
	PropertySetterUInt(std::string classname, std::string name, GetterUInt getter, SetterUInt setter, std::string parentName="");
public:
	void setValue(unsigned int value);
protected:
	SetterUInt _setter;
};

//
// double
//
typedef std::function<double()> GetterDouble;
template<typename Class>
GetterDouble DefineGetterDouble(Class * object, double (Class::*function)() const) {
	return std::bind(function, object);
}
class PropertyGetterDouble: public PropertyGetterBase {
public:
	PropertyGetterDouble(std::string classname, std::string name, GetterDouble getter, std::string parentName="");
public:
	double getValue();
protected:
	GetterDouble _getter;
	const std::string _type = "double";
};

typedef std::function<void(double) > SetterDouble;
template<typename Class>
SetterDouble DefineSetterDouble(Class * object, void (Class::*function)(double)) {
	return std::bind(function, object, std::placeholders::_1);
}
class PropertySetterDouble: public PropertyGetterDouble {
public:
	PropertySetterDouble(std::string classname, std::string name, GetterDouble getter, SetterDouble setter, std::string parentName="");
public:
	void setValue(double value);
protected:
	SetterDouble _setter;
};


//
// string
//
typedef std::function<std::string()> GetterString;
template<typename Class>
GetterString DefineGetterString(Class * object, std::string (Class::*function)() const) {
	return std::bind(function, object);
}
class PropertyGetterString: public PropertyGetterBase {
public:
	PropertyGetterString(std::string classname, std::string name, GetterString getter, std::string parentName="");
public:
	std::string getValue();
protected:
	GetterString _getter;
	const std::string _type = "string";
};

typedef std::function<void(std::string) > SetterString;
template<typename Class>
SetterString DefineSetterString(Class * object, void (Class::*function)(std::string)) {
	return std::bind(function, object, std::placeholders::_1);
}
class PropertySetterString: public PropertyGetterString {
public:
	PropertySetterString(std::string classname, std::string name, GetterString getter, SetterString setter, std::string parentName="");
public:
	void setValue(std::string value);
protected:
	SetterString _setter;
};



//
// time unit
//
typedef std::function<Util::TimeUnit()> GetterTimeUnit;
template<typename Class>
GetterTimeUnit DefineGetterTimeUnit(Class * object, Util::TimeUnit (Class::*function)() const) {
	return std::bind(function, object);
}
class PropertyGetterTimeUnit: public PropertyGetterBase {
public:
	PropertyGetterTimeUnit(std::string classname, std::string name, GetterTimeUnit getter, std::string parentName="");
public:
	Util::TimeUnit getValue();
protected:
	GetterTimeUnit _getter;
	const std::string _type = "TimeUnit";
};

typedef std::function<void(Util::TimeUnit) > SetterTimeUnit;
template<typename Class>
SetterTimeUnit DefineSetterTimeUnit(Class * object, void (Class::*function)(Util::TimeUnit)) {
	return std::bind(function, object, std::placeholders::_1);
}
class PropertySetterTimeUnit: public PropertyGetterTimeUnit {
public:
	PropertySetterTimeUnit(std::string classname, std::string name, GetterTimeUnit getter, SetterTimeUnit setter, std::string parentName="");
public:
	void setValue(Util::TimeUnit value);
protected:
	SetterTimeUnit _setter;
};




//
// bool
//
typedef std::function<bool()> GetterBool;
template<typename Class>
GetterBool DefineGetterBool(Class * object, bool (Class::*function)() const) {
	return std::bind(function, object);
}
class PropertyGetterBool: public PropertyGetterBase {
public:
	PropertyGetterBool(std::string classname, std::string name, GetterBool getter, std::string parentName="");
public:
	bool getValue();
protected:
	GetterBool _getter;
	const std::string _type = "bool";
};

typedef std::function<void(bool) > SetterBool;
template<typename Class>
SetterBool DefineSetterBool(Class * object, void (Class::*function)(bool)) {
	return std::bind(function, object, std::placeholders::_1);
}
class PropertySetterBool: public PropertyGetterBool {
public:
	PropertySetterBool(std::string classname, std::string name, GetterBool getter, SetterBool setter, std::string parentName="");
public:
	void setValue(bool value);
protected:
	SetterBool _setter;
};


 */

#endif // PROPERTY_H
