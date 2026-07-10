#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <QTreeWidget>
#include <QObject>

class ModelDataDefinition;

class PropertyEditor : public QTreeWidget {
public:
	PropertyEditor(QWidget *parent = nullptr);
public:
	void setModelBlock(ModelDataDefinition* modelblock);
};

#endif // PROPERTYEDITOR_H
