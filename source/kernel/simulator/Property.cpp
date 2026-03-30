#include "Property.h"

/*
PropertyGetterBase::PropertyGetterBase(std::string classname, std::string name, std::string parentName){
	_classename = classname;
	_name = name;
	_parentname = parentName;
}

std::string PropertyGetterBase::show() const
{
	return "classname="+_classename+", name="+_name+", parentname="+_parentname;
}

std::string PropertyGetterBase::getClassname() const{
	return _classename;
}

std::string PropertyGetterBase::getParentName() const{
	return _parentname;
}

std::string PropertyGetterBase::getName() const{
	return _name;
}

void PropertyGetterBase::setName(const std::string &name)
{
	_name = name;
}

double PropertyGetterBase::getValue()
{
}

std::string PropertyGetterBase::getType() const
{
	return _type;
}

PropertySetterBase::PropertySetterBase(std::string classname, std::string name, std::string parentName)
	: PropertyGetterBase(classname, name, parentName) {

}

void PropertySetterBase::setValue(double value)
{
}

PropertyGetterUInt::PropertyGetterUInt(std::string classname, std::string name, GetterUInt getter, std::string parentName)
	: PropertyGetterBase(classname, name, parentName)
{
	_getter = getter;
}

unsigned int PropertyGetterUInt::getValue()
{
	return _getter();
}


PropertySetterUInt::PropertySetterUInt(std::string classname, std::string name, GetterUInt getter, SetterUInt setter, std::string parentName)
	: PropertyGetterUInt(classname, name, getter, parentName)
{
	_setter = setter;
}

void PropertySetterUInt::setValue(unsigned int value)
{
	_setter(value);
}


PropertyGetterString::PropertyGetterString(std::string classname, std::string name, GetterString getter, std::string parentName)
	: PropertyGetterBase(classname, name, parentName)
{

}

std::string PropertyGetterString::getValue()
{
	return _getter();
}

PropertySetterString::PropertySetterString(std::string classname, std::string name, GetterString getter, SetterString setter, std::string parentName)
	: PropertyGetterString(classname, name, getter, parentName)
{
	_setter = setter;
}

void PropertySetterString::setValue(std::string value)
{
	_setter(value);
}

PropertyGetterTimeUnit::PropertyGetterTimeUnit(std::string classname, std::string name, GetterTimeUnit getter, std::string parentName)
	: PropertyGetterBase(classname, name, parentName)
{

}

Util::TimeUnit PropertyGetterTimeUnit::getValue()
{
	return _getter();
}

PropertySetterTimeUnit::PropertySetterTimeUnit(std::string classname, std::string name, GetterTimeUnit getter, SetterTimeUnit setter, std::string parentName)
	: PropertyGetterTimeUnit(classname, name, getter, parentName)
{
	_setter = setter;
}

void PropertySetterTimeUnit::setValue(Util::TimeUnit value)
{
	_setter(value);
}

PropertyGetterDouble::PropertyGetterDouble(std::string classname, std::string name, GetterDouble getter, std::string parentName)
	: PropertyGetterBase(classname, name, parentName)
{

}

double PropertyGetterDouble::getValue()
{
	return _getter();
}

PropertySetterDouble::PropertySetterDouble(std::string classname, std::string name, GetterDouble getter, SetterDouble setter, std::string parentName)
	: PropertyGetterDouble(classname, name, getter, parentName)
{
	_setter = setter;
}

void PropertySetterDouble::setValue(double value)
{
	_setter(value);
}

PropertyGetterBool::PropertyGetterBool(std::string classname, std::string name, GetterBool getter, std::string parentName)
	: PropertyGetterBase(classname, name, parentName)
{

}

bool PropertyGetterBool::getValue()
{
	return _getter();
}

PropertySetterBool::PropertySetterBool(std::string classname, std::string name, GetterBool getter, SetterBool setter, std::string parentName)
	: PropertyGetterBool(classname, name, getter, parentName)
{
	_setter = setter;
}

void PropertySetterBool::setValue(bool value)
{
	_setter(value);
}

void LegacyPropertyBase::setName(const std::string&name)
{
	_name = name;
}
 */
