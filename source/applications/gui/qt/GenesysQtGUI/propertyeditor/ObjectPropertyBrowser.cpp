#include "ObjectPropertyBrowser.h"
#include <QDebug>
//#include <QProperty>
#include <iostream>

ObjectPropertyBrowser::ObjectPropertyBrowser(QWidget* parent) : QtTreePropertyBrowser(parent) {
	variantManager = new QtVariantPropertyManager(this);
	setFactoryForManager(variantManager, new QtVariantEditorFactory);
	this->setResizeMode(QtTreePropertyBrowser::Interactive);
	this->setAccessibleDescription("Editor for model elements");
	this->setHeaderVisible(true);
	this->setIndentation(10);
    this->setRootIsDecorated(true);
}

void ObjectPropertyBrowser::clearCurrentlyConnectedObject() {
    currentlyConnectedObject = nullptr;
}

void ObjectPropertyBrowser::setActiveObject(QObject *obj, ModelDataDefinition* mdd, PropertyEditorGenesys* peg, std::map<SimulationControl*, DataComponentProperty*>* dcp, std::map<SimulationControl*, DataComponentEditor*>* dce, std::map<SimulationControl*, ComboBoxEnum*>* pb) {
	// TODO : initialize in other part of the code
    propertyEditor = peg;
    propertyList = dcp;
    propertyEditorUI = dce;
    propertyBox = pb;

	clear();
	variantManager->clear();
	propertyMap.clear();
	if(currentlyConnectedObject) currentlyConnectedObject->disconnect(this);
	currentlyConnectedObject = obj;
	if(!obj) return;

	obj->setObjectName(QString::fromStdString(std::to_string(mdd->getId())));

	std::map<std::string, QtVariantProperty*> propGroupMap;
	QtVariantProperty *groupProperty = nullptr;
	QtVariantProperty *property = nullptr;

	if (mdd != nullptr) {
		std::string className;
		for (PropertyBase* prop: *mdd->getProperties()->list()) {
            className = prop->getClassname(); //@TODO Era para ser getClassName();
            std::map<std::string, QtVariantProperty*>::iterator it;
            if ((it = propGroupMap.find(className))==propGroupMap.end()) {
                groupProperty = variantManager->addProperty(QVariant::Char, className.c_str());
                propGroupMap.insert({className, groupProperty});
            } else {
                groupProperty = (*it).second;
            }
            this->addProperty(groupProperty);
            obj->setProperty(prop->getName().c_str(), 1);
            property = variantManager->addProperty(QVariant::String, prop->getName().c_str());
            if (property != nullptr) {
                property->setEnabled(true);//mp.isWritable());
                propertyMap[property] = prop->getName().c_str();
                groupProperty->addSubProperty(property);
                //this->addProperty(property);

                // if (prop->getIsEnum()) {
                    // property->setValue(QString::fromStdString(prop->getEnumValues()));
                // } else {
                    property->setValue(QString::fromStdString(prop->getValue()));
                // }
            }
        }
	}

	groupProperty = variantManager->addProperty(QVariant::Char,"Graphical");
	this->addProperty(groupProperty);
	propGroupMap.insert({"Graphical", groupProperty});

	// add properties based on meta-object
	for(int i=1; i< obj->metaObject()->propertyCount();i++) {
		QMetaProperty mp = obj->metaObject()->property(i);
		property =variantManager->addProperty(mp.type(),mp.name());
		if (property != nullptr) {
			property->setEnabled(mp.isWritable());
			propertyMap[property] = mp.name();
			groupProperty->addSubProperty(property);
			//this->addProperty(property);
		}
	}
	/*
	for(QByteArray name :obj->dynamicPropertyNames()) {
		//QMetaProperty mp = obj->metaObject()->property(i);
		QVariant variant = obj->property(name);
		QtVariantProperty *property = variantManager->addProperty(2, name);//addProperty(mp.type(),mp.name());
		if (property != nullptr) {
			property->setEnabled(true);//mp.isWritable());
			propertyMap[property] = name;//mp.name();
			addProperty(property);
		}
	}
	*/

	/*
	if (mdd != nullptr) {
		std::string className;
		for (PropertyG* prop: *mdd->getProperties()->list()) {
			className = prop->getClassname();
			std::map<std::string, QtVariantProperty*>::iterator it;
			//if ((it = propGroupMap.find(className))==propGroupMap.end()) {
			//	groupProperty = variantManager->addProperty(2, className.c_str());
			//	propGroupMap.insert({className, groupProperty});
			//} else {
			//	groupProperty = (*it).second;
			//}
			QtVariantProperty *property = variantManager->addProperty(1, prop->getName().c_str());
			if (property != nullptr) {
				property->setEnabled(true);//mp.isWritable());
				propertyMap[property] = prop->getName().c_str();
				//groupProperty->addSubProperty(property);
				addProperty(property);
			}
		}
	}

    */

	// connect(obj, SIGNAL(propertyChanged()), this, SLOT(objectUpdated()));
	objectUpdated();
}

void ObjectPropertyBrowser::valueChanged(QtProperty *property, const QVariant &value)
{
    std::string stdValue = value.toString().toStdString();

	if (stdValue.back() == ' ') {
		stdValue.pop_back();
		SimulationControl* prop = propertyEditor->findProperty(currentlyConnectedObject->objectName().toStdString(), property->propertyName().toStdString());

        if (prop->getIsList()) {
            (*(propertyList))[prop]->open_window();
        } else { if (prop->getIsEnum()) {
                    (*(propertyBox))[prop]->open_box();
            } else {
                    propertyEditor->changeProperty(prop, stdValue);
                    currentlyConnectedObject->setProperty(property->propertyName().toStdString().c_str(), value);

                    if (prop->getIsClass()) {
                        (*(propertyEditorUI))[prop]->open_window(prop);
                    }
            }
        }
	}

	objectUpdated();
}

void ObjectPropertyBrowser::objectUpdated()
{
	disconnect(variantManager, SIGNAL(valueChanged(QtProperty*, QVariant)), this, SLOT(valueChanged(QtProperty*, QVariant)));
	QMapIterator<QtProperty*, const char*> i(propertyMap);
	while(i.hasNext()) {
		i.next();
		variantManager->setValue(i.key(), currentlyConnectedObject->property(i.value()));
	}
    connect(variantManager, SIGNAL(valueChanged(QtProperty*, QVariant)), this, SLOT(valueChanged(QtProperty*, QVariant)));
}
