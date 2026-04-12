#ifndef OBJECTPROPERTYBROWSER_H
#define OBJECTPROPERTYBROWSER_H

#include <map>
#include <vector>
#include <set>
#include <functional>

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QSet>
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
    bool isCommitPipelineBusy() const;

private:
    struct Binding {
        ModelDataDefinition* owner = nullptr;
        SimulationControl* control = nullptr;
        GenesysPropertyDescriptor descriptor;
        bool isObjectSelector = false;
    };

private:
    // Ensure the property browser infrastructure is created exactly once.
    void _ensureBrowserInfrastructure();
    void _notifyModelChangeApplied();
    void _clearAll();
    void _rebuildProperties();
    void _populateKernelProperties(ModelDataDefinition* mdd);
    void _appendDescriptorRecursively(
        QtProperty* parent,
        SimulationControl* control,
        std::set<const SimulationControl*>& recursionPath,
        int depth = 0
        );

    QtProperty* _createLeafProperty(const GenesysPropertyDescriptor& desc);
    QVariant _toVariant(const GenesysPropertyDescriptor& desc) const;
    std::string _fromVariant(const GenesysPropertyDescriptor& desc, const QVariant& value) const;
    int _enumIndexFor(const GenesysPropertyDescriptor& desc) const;
    QStringList _toQStringList(const std::vector<std::string>& values) const;

    bool _openSpecializedEditorForCurrentItem();
    bool _openSpecializedEditor(QtProperty* property);
    bool _createObjectForProperty(QtProperty* property);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    // Guard against reentrant callback chains during property rebuild and notification.
    bool _isRebuildingProperties = false;
    // Guard against nested model-change notifications triggered by editor callbacks.
    bool _isNotifyingModelChange = false;
    // Keep track of pending rebuild requests raised during guarded execution.
    bool _pendingRebuild = false;
    // Coalesce queued rebuild dispatches while one deferred rebuild request is already scheduled.
    bool _isDeferredRebuildScheduled = false;
    // Coalesce queued model-change callbacks while one deferred dispatch is already scheduled.
    bool _isDeferredModelChangedScheduled = false;

private:
    // Execute property rebuild with reentrancy suppression and deferred retry support.
    void _rebuildPropertiesGuarded();
    // Request a deferred rebuild after current signal/slot stack finishes.
    void _scheduleDeferredRebuild();
    // Request a deferred model-changed callback after current signal/slot stack finishes.
    void _scheduleDeferredModelChangedCallback();
    // Check whether active editor bindings are currently valid before mutating model state.
    bool _hasValidActiveBindingContext(QtProperty* property = nullptr) const;

private:
    // Track the selected graphical object safely in case Qt destroys it during callbacks.
    QPointer<QObject> _graphicalObject = nullptr;
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
    // Track whether factories were already bound to managers in the browser.
    bool _browserInfrastructureBound = false;

    QMap<QtProperty*, Binding> _bindings;
    QMap<QtProperty*, QStringList> _enumNames;
    QSet<QtProperty*> _pendingCommittedProperties;
    QMap<QtProperty*, QVariant> _pendingCommittedValues;
    ModelChangedCallback _modelChangedCallback;

private:
    bool _requiresCommitConfirmation(const Binding& binding) const;
    bool _applyVariantChange(QtProperty* property, const QVariant& value, bool committed);

private slots:
    void valueChanged(QtProperty *property, const QVariant &value);
    void enumValueChanged(QtProperty *property, int value);
    void onVariantEditorCommitted(QtProperty* property);

public slots:
    void objectUpdated();
};

#endif // OBJECTPROPERTYBROWSER_H
