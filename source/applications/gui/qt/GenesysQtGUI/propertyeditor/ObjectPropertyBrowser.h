#ifndef OBJECTPROPERTYBROWSER_H
#define OBJECTPROPERTYBROWSER_H


#include <QObject>
#include <QPushButton>
#include <QMap>
#include <QMetaProperty>
#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "qtpropertybrowser/qtvariantproperty.h"


#include "DataComponentProperty.h"
#include "DataComponentEditor.h"
#include "ComboBoxEnum.h"

#include "../../../../kernel/simulator/ModelDataDefinition.h"
#include "../../../../kernel/simulator/PropertyGenesys.h"

class ObjectPropertyBrowser : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
    ObjectPropertyBrowser(QWidget* parent);
    void setActiveObject(QObject *obj, ModelDataDefinition* mdd = nullptr, PropertyEditorGenesys* peg = nullptr, std::map<SimulationControl*, DataComponentProperty*>* pl = nullptr, std::map<SimulationControl*, DataComponentEditor*>* peUI = nullptr, std::map<SimulationControl*, ComboBoxEnum*>* pb = nullptr);
    void clearCurrentlyConnectedObject();
    
private:
    QtVariantPropertyManager *variantManager;
    QObject *currentlyConnectedObject = nullptr;
    QMap<QtProperty *, const char*> propertyMap;

    PropertyEditorGenesys* propertyEditor;
    std::map<SimulationControl*, DataComponentProperty*>* propertyList;
    std::map<SimulationControl*, DataComponentEditor*>* propertyEditorUI;
    std::map<SimulationControl*, ComboBoxEnum*>* propertyBox;

private slots:
    void valueChanged(QtProperty *property, const QVariant &value);

public slots:
    void objectUpdated();
};

#endif // OBJECTPROPERTYBROWSER_H
