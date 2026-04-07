#ifndef OBJECTPROPERTYBROWSER_H
#define OBJECTPROPERTYBROWSER_H

#include <map>
#include <vector>
#include <functional>

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMouseEvent>

#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qtpropertymanager.h"
#include "qtpropertybrowser/qteditorfactory.h"

#include "DataComponentProperty.h"
#include "DataComponentEditor.h"
#include "ComboBoxEnum.h"

#include "../../../../kernel/simulator/ModelDataDefinition.h"
#include "../../../../kernel/simulator/PropertyGenesys.h"
#include "../../../../kernel/simulator/GenesysPropertyIntrospection.h"

class ObjectPropertyBrowser : public QtTreePropertyBrowser {
    Q_OBJECT

public:
    explicit ObjectPropertyBrowser(QWidget* parent = nullptr);
    using ModelChangedCallback = std::function<void()>;

    void setActiveObject(
        QObject *obj,
        ModelDataDefinition* mdd = nullptr,
        PropertyEditorGenesys* peg = nullptr,
        std::map<SimulationControl*, DataComponentProperty*>* pl = nullptr,
        std::map<SimulationControl*, DataComponentEditor*>* peUI = nullptr,
        std::map<SimulationControl*, ComboBoxEnum*>* pb = nullptr
        );

    void clearCurrentlyConnectedObject();
    void setModelChangedCallback(ModelChangedCallback callback);

private:
    struct Binding {
        ModelDataDefinition* owner = nullptr;
        SimulationControl* control = nullptr;
        GenesysPropertyDescriptor descriptor;
    };

private:
    void _notifyModelChangeApplied();
    void _clearAll();
    void _populateKernelProperties(ModelDataDefinition* mdd);
    QtProperty* _createProperty(const GenesysPropertyDescriptor& desc);
    QVariant _toVariant(const GenesysPropertyDescriptor& desc) const;
    std::string _fromVariant(const GenesysPropertyDescriptor& desc, const QVariant& value) const;
    QString _objectSummary(const GenesysPropertyDescriptor& desc) const;
    int _enumIndexFor(const GenesysPropertyDescriptor& desc) const;
    QStringList _toQStringList(const std::vector<std::string>& values) const;

    bool _openSpecializedEditorForCurrentItem();
    bool _openSpecializedEditor(QtProperty* property);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QObject* _graphicalObject = nullptr;
    ModelDataDefinition* _modelObject = nullptr;
    PropertyEditorGenesys* _propertyEditor = nullptr;

    std::map<SimulationControl*, DataComponentProperty*>* _propertyList = nullptr;
    std::map<SimulationControl*, DataComponentEditor*>* _propertyEditorUI = nullptr;
    std::map<SimulationControl*, ComboBoxEnum*>* _propertyBox = nullptr;

    QtVariantPropertyManager* _variantManager = nullptr;
    QtGroupPropertyManager* _groupManager = nullptr;
    QtEnumPropertyManager* _enumManager = nullptr;

    QtVariantEditorFactory* _variantFactory = nullptr;
    QtEnumEditorFactory* _enumFactory = nullptr;

    QMap<QtProperty*, Binding> _bindings;
    QMap<QtProperty*, QStringList> _enumNames;
    QMap<QtProperty*, QtVariantProperty*> _objectSummaryProperties;
    ModelChangedCallback _modelChangedCallback;

private slots:
    void valueChanged(QtProperty *property, const QVariant &value);
    void enumValueChanged(QtProperty *property, int value);

public slots:
    void objectUpdated();
};

#endif // OBJECTPROPERTYBROWSER_H
