#include "ObjectPropertyBrowser.h"

#include <map>
#include <set>
#include <sstream>
#include <exception>
#include <utility>

#include <QSignalBlocker>
#include <QString>
#include <QVariant>
#include <QMenu>
#include <QDebug>
#include <QMetaObject>
#include <QLineEdit>
#include <QAbstractSpinBox>

namespace {
class CommitAwareVariantEditorFactory final : public QtVariantEditorFactory {
public:
    using CommitCallback = std::function<void(QtProperty*)>;

    explicit CommitAwareVariantEditorFactory(QObject* parent = nullptr)
        : QtVariantEditorFactory(parent) {}

    void setCommitCallback(CommitCallback callback) {
        _commitCallback = std::move(callback);
    }

protected:
    QWidget* createEditor(QtVariantPropertyManager* manager, QtProperty* property, QWidget* parent) override {
        QWidget* editor = QtVariantEditorFactory::createEditor(manager, property, parent);
        if (editor == nullptr || _commitCallback == nullptr) {
            return editor;
        }

        if (QLineEdit* lineEdit = editor->findChild<QLineEdit*>()) {
            QObject::connect(lineEdit, &QLineEdit::editingFinished, editor, [callback = _commitCallback, property]() {
                callback(property);
            });
            return editor;
        }

        if (QAbstractSpinBox* spinBox = editor->findChild<QAbstractSpinBox*>()) {
            QObject::connect(spinBox, &QAbstractSpinBox::editingFinished, editor, [callback = _commitCallback, property]() {
                callback(property);
            });
        }

        return editor;
    }

private:
    CommitCallback _commitCallback;
};
} // namespace

ObjectPropertyBrowser::ObjectPropertyBrowser(QWidget* parent)
    : QtTreePropertyBrowser(parent) {

    _clearAll();

    setResizeMode(QtTreePropertyBrowser::Interactive);
    setAccessibleDescription("Editor for model elements");
    setHeaderVisible(true);
    setIndentation(10);
    setRootIsDecorated(true);
    setAlternatingRowColors(true);
}

void ObjectPropertyBrowser::_clearAll() {
    clear();
    _bindings.clear();
    _enumNames.clear();
    _pendingCommittedProperties.clear();
    _pendingCommittedValues.clear();

    delete _variantFactory;
    delete _enumFactory;
    delete _variantManager;
    delete _groupManager;
    delete _enumManager;

    _variantFactory = nullptr;
    _enumFactory = nullptr;
    _variantManager = nullptr;
    _groupManager = nullptr;
    _enumManager = nullptr;

    _variantManager = new QtVariantPropertyManager(this);
    _groupManager = new QtGroupPropertyManager(this);
    _enumManager = new QtEnumPropertyManager(this);

    auto* commitFactory = new CommitAwareVariantEditorFactory(this);
    commitFactory->setCommitCallback([this](QtProperty* property) {
        onVariantEditorCommitted(property);
    });
    _variantFactory = commitFactory;
    _enumFactory = new QtEnumEditorFactory(this);

    setFactoryForManager(_variantManager, _variantFactory);
    setFactoryForManager(_enumManager, _enumFactory);

    connect(
        _variantManager,
        SIGNAL(valueChanged(QtProperty*,QVariant)),
        this,
        SLOT(valueChanged(QtProperty*,QVariant))
        );

    connect(
        _enumManager,
        SIGNAL(valueChanged(QtProperty*,int)),
        this,
        SLOT(enumValueChanged(QtProperty*,int))
        );
}

void ObjectPropertyBrowser::clearCurrentlyConnectedObject() {
    // Fully detach object/editor pointers before clearing UI bindings.
    _graphicalObject = nullptr;
    _modelObject = nullptr;
    _propertyEditor = nullptr;
    _isRebuildingProperties = false;
    _isNotifyingModelChange = false;
    _pendingRebuild = false;
    _isDeferredRebuildScheduled = false;
    _isDeferredModelChangedScheduled = false;
    _clearAll();
}

void ObjectPropertyBrowser::setModelChangedCallback(ModelChangedCallback callback) {
    _modelChangedCallback = std::move(callback);
}

bool ObjectPropertyBrowser::isCommitPipelineBusy() const {
    return _isRebuildingProperties
        || _isNotifyingModelChange
        || _pendingRebuild
        || _isDeferredRebuildScheduled
        || _isDeferredModelChangedScheduled
        || !_pendingCommittedProperties.isEmpty();
}

void ObjectPropertyBrowser::setActiveObject(
    QObject *obj,
    ModelDataDefinition* mdd,
    PropertyEditorGenesys* peg,
    std::map<SimulationControl*, DataComponentProperty*>* pl,
    std::map<SimulationControl*, DataComponentEditor*>* peUI,
    std::map<SimulationControl*, ComboBoxEnum*>* pb
    ) {
    // Always detach stale bindings first to avoid stale-pointer use during rebinding.
    clearCurrentlyConnectedObject();

    // Bind the new active object and editor dependencies for the next safe rebuild.
    _graphicalObject = obj;
    _modelObject = mdd;
    _propertyEditor = peg;
    _propertyList = pl;
    _propertyEditorUI = peUI;
    _propertyBox = pb;

    // Rebuild once with guard logic so nested signals cannot recurse unsafely.
    _scheduleDeferredRebuild();
}

// Rebuild properties with explicit suppression of nested recursive rebuild execution.
void ObjectPropertyBrowser::_rebuildPropertiesGuarded() {
    qInfo() << "[PropertyEditor] _rebuildPropertiesGuarded enter. rebuilding=" << _isRebuildingProperties
            << " notifying=" << _isNotifyingModelChange << " pending=" << _pendingRebuild;
    if (_isRebuildingProperties) {
        _pendingRebuild = true;
        qInfo() << "[PropertyEditor] _rebuildPropertiesGuarded deferred due to active rebuild";
        return;
    }

    _isRebuildingProperties = true;
    do {
        _pendingRebuild = false;
        _rebuildProperties();
    } while (_pendingRebuild);
    _isRebuildingProperties = false;
    qInfo() << "[PropertyEditor] _rebuildPropertiesGuarded exit";
}

void ObjectPropertyBrowser::_scheduleDeferredRebuild() {
    qInfo() << "[PropertyEditor] schedule deferred rebuild request. alreadyScheduled=" << _isDeferredRebuildScheduled
            << " rebuilding=" << _isRebuildingProperties << " notifying=" << _isNotifyingModelChange;
    if (_isRebuildingProperties || _isNotifyingModelChange) {
        _pendingRebuild = true;
    }
    if (_isDeferredRebuildScheduled) {
        return;
    }

    _isDeferredRebuildScheduled = true;
    QMetaObject::invokeMethod(this, [this]() {
        qInfo() << "[PropertyEditor] executing deferred rebuild";
        _isDeferredRebuildScheduled = false;
        if (!_hasValidActiveBindingContext()) {
            qWarning() << "[PropertyEditor] Deferred rebuild canceled due to invalid binding context";
            return;
        }
        _rebuildPropertiesGuarded();
    }, Qt::QueuedConnection);
}

void ObjectPropertyBrowser::_scheduleDeferredModelChangedCallback() {
    qInfo() << "[PropertyEditor] schedule deferred model-changed callback. alreadyScheduled="
            << _isDeferredModelChangedScheduled;
    if (_isDeferredModelChangedScheduled) {
        return;
    }

    _isDeferredModelChangedScheduled = true;
    QMetaObject::invokeMethod(this, [this]() {
        qInfo() << "[PropertyEditor] executing deferred model-changed callback. rebuildScheduled="
                << _isDeferredRebuildScheduled << " rebuilding=" << _isRebuildingProperties
                << " pendingRebuild=" << _pendingRebuild;
        _isDeferredModelChangedScheduled = false;
        if (_isRebuildingProperties || _pendingRebuild || _isDeferredRebuildScheduled) {
            qInfo() << "[PropertyEditor] deferring model-changed callback until rebuild stabilizes";
            _scheduleDeferredModelChangedCallback();
            return;
        }
        if (!_hasValidActiveBindingContext()) {
            qWarning() << "[PropertyEditor] Deferred model-changed callback canceled due to invalid binding context";
            return;
        }
        QMetaObject::invokeMethod(this, [this]() {
            if (_isRebuildingProperties || _pendingRebuild || _isDeferredRebuildScheduled) {
                qInfo() << "[PropertyEditor] model-changed callback postponed one more turn due to pending rebuild";
                _scheduleDeferredModelChangedCallback();
                return;
            }
            if (_modelChangedCallback) {
                qInfo() << "[PropertyEditor] invoking model-changed callback after local rebuild stabilization";
                _modelChangedCallback();
            }
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

// Validate that active objects are still attached before applying property edits.
bool ObjectPropertyBrowser::_hasValidActiveBindingContext(QtProperty* property) const {
    if (_modelObject == nullptr) {
        return false;
    }
    if (_graphicalObject.isNull()) {
        return false;
    }
    if (property != nullptr) {
        auto it = _bindings.constFind(property);
        if (it == _bindings.constEnd()) {
            return false;
        }
        const Binding& binding = it.value();
        if (binding.control == nullptr) {
            return false;
        }
        if (binding.owner != _modelObject) {
            return false;
        }
    }
    return true;
}

void ObjectPropertyBrowser::_rebuildProperties() {
    qInfo() << "[PropertyEditor] _rebuildProperties enter";
    // Clear existing browser state first so stale bindings cannot survive across rebuilds.
    _clearAll();

    if (_modelObject == nullptr) {
        qInfo() << "Skipping property rebuild because model object is null";
        return;
    }

    _populateKernelProperties(_modelObject);
    qInfo() << "[PropertyEditor] _rebuildProperties exit";
}

QStringList ObjectPropertyBrowser::_toQStringList(const std::vector<std::string>& values) const {
    QStringList result;
    for (const std::string& value : values) {
        result << QString::fromStdString(value);
    }
    return result;
}

int ObjectPropertyBrowser::_enumIndexFor(const GenesysPropertyDescriptor& desc) const {
    for (int i = 0; i < static_cast<int>(desc.choices.size()); ++i) {
        if (desc.choices[static_cast<std::size_t>(i)] == desc.currentValue) {
            return i;
        }
    }

    try {
        return std::stoi(desc.currentValue);
    } catch (...) {
        return 0;
    }
}

QVariant ObjectPropertyBrowser::_toVariant(const GenesysPropertyDescriptor& desc) const {
    switch (desc.kind) {
    case GenesysPropertyKind::Boolean: {
        const bool value = (desc.currentValue == "true" || desc.currentValue == "1");
        return QVariant(value);
    }
    case GenesysPropertyKind::Integer:
    case GenesysPropertyKind::UnsignedInteger:
    case GenesysPropertyKind::UnsignedShort: {
        return QVariant(QString::fromStdString(desc.currentValue).toInt());
    }
    case GenesysPropertyKind::Double: {
        return QVariant(QString::fromStdString(desc.currentValue).toDouble());
    }
    case GenesysPropertyKind::String:
    case GenesysPropertyKind::Object:
    case GenesysPropertyKind::List:
    case GenesysPropertyKind::Unknown:
    case GenesysPropertyKind::Enum:
    case GenesysPropertyKind::TimeUnit:
    default: {
        return QVariant(QString::fromStdString(desc.currentValue));
    }
    }
}

std::string ObjectPropertyBrowser::_fromVariant(const GenesysPropertyDescriptor& desc, const QVariant& value) const {
    switch (desc.kind) {
    case GenesysPropertyKind::Boolean:
        return value.toBool() ? "true" : "false";

    case GenesysPropertyKind::Integer:
    case GenesysPropertyKind::UnsignedInteger:
    case GenesysPropertyKind::UnsignedShort:
        return std::to_string(value.toInt());

    case GenesysPropertyKind::Double: {
        std::ostringstream os;
        os.precision(15);
        os << value.toDouble();
        return os.str();
    }

    case GenesysPropertyKind::String:
    case GenesysPropertyKind::Unknown:
    default:
        return value.toString().toStdString();
    }
}

QtProperty* ObjectPropertyBrowser::_createLeafProperty(const GenesysPropertyDescriptor& desc) {
    const QString name = QString::fromStdString(desc.displayName);

    if ((desc.kind == GenesysPropertyKind::Enum || desc.kind == GenesysPropertyKind::TimeUnit)
        && !desc.choices.empty()) {

        QtProperty* property = _enumManager->addProperty(name);
        _enumNames[property] = _toQStringList(desc.choices);
        _enumManager->setEnumNames(property, _enumNames[property]);
        _enumManager->setValue(property, _enumIndexFor(desc));
        property->setEnabled(!desc.readOnly);

        Binding binding;
        binding.owner = _modelObject;
        binding.control = desc.control;
        binding.descriptor = desc;
        _bindings[property] = binding;

        return property;
    }

    int variantType = QVariant::String;

    switch (desc.kind) {
    case GenesysPropertyKind::Boolean:
        variantType = QVariant::Bool;
        break;
    case GenesysPropertyKind::Integer:
    case GenesysPropertyKind::UnsignedInteger:
    case GenesysPropertyKind::UnsignedShort:
        variantType = QVariant::Int;
        break;
    case GenesysPropertyKind::Double:
        variantType = QVariant::Double;
        break;
    default:
        variantType = QVariant::String;
        break;
    }

    QtVariantProperty* property = _variantManager->addProperty(variantType, name);
    property->setValue(_toVariant(desc));

    // This block enables direct inline editing only for scalar-like properties handled by variant editors.
    const bool editableInline =
        !desc.readOnly &&
        !desc.supportsListEditor &&
        !desc.supportsInlineExpansion &&
        !(desc.kind == GenesysPropertyKind::Enum || desc.kind == GenesysPropertyKind::TimeUnit);

    property->setEnabled(editableInline);

    // This block configures the list-editor hint based on explicit contract metadata instead of heuristics.
    if (desc.supportsListEditor) {
        property->setToolTip("List property. Use double click, Enter or context menu to open list editor.");
        property->setStatusTip("List editor");
    }

    Binding binding;
    binding.owner = _modelObject;
    binding.control = desc.control;
    binding.descriptor = desc;
    _bindings[property] = binding;

    return property;
}

void ObjectPropertyBrowser::_appendDescriptorRecursively(
    QtProperty* parent,
    SimulationControl* control,
    std::set<const SimulationControl*>& recursionPath,
    int depth
    ) {
    if (parent == nullptr || control == nullptr) {
        return;
    }

    GenesysPropertyDescriptor desc = GenesysPropertyIntrospection::describe(control);

    // This block uses explicit inline-expansion support metadata to decide recursion.
    if (!desc.supportsInlineExpansion) {
        QtProperty* leaf = _createLeafProperty(desc);
        if (leaf != nullptr) {
            parent->addSubProperty(leaf);
        }
        return;
    }

    QtProperty* group = _groupManager->addProperty(QString::fromStdString(desc.displayName));
    parent->addSubProperty(group);

    Binding groupBinding;
    groupBinding.owner = _modelObject;
    groupBinding.control = control;
    groupBinding.descriptor = desc;
    _bindings[group] = groupBinding;

    if (recursionPath.find(control) != recursionPath.end()) {
        QtVariantProperty* cycleNode = _variantManager->addProperty(QVariant::String, "Cycle");
        cycleNode->setEnabled(false);
        cycleNode->setValue("Recursion stopped: object already visited in this branch");
        group->addSubProperty(cycleNode);
        return;
    }

    if (depth > 10) {
        QtVariantProperty* depthNode = _variantManager->addProperty(QVariant::String, "Depth");
        depthNode->setEnabled(false);
        depthNode->setValue("Recursion depth limit reached");
        group->addSubProperty(depthNode);
        return;
    }

    // This block renders object selection when the contract declares selection among existing objects.
    if (desc.supportsExistingObjectSelection && !desc.choices.empty()) {
        QtProperty* refProperty = _enumManager->addProperty("Reference");
        _enumNames[refProperty] = _toQStringList(desc.choices);
        _enumManager->setEnumNames(refProperty, _enumNames[refProperty]);
        _enumManager->setValue(refProperty, _enumIndexFor(desc));
        refProperty->setEnabled(!desc.readOnly);

        Binding refBinding = groupBinding;
        refBinding.isObjectSelector = true;
        _bindings[refProperty] = refBinding;
        group->addSubProperty(refProperty);
    }

    List<SimulationControl*>* childrenList = control->getEditableProperties();
    if (childrenList == nullptr) {
        QtVariantProperty* emptyNode = _variantManager->addProperty(QVariant::String, "Info");
        emptyNode->setEnabled(false);
        if (desc.supportsObjectCreation && !desc.readOnly) {
            emptyNode->setValue("Object not available yet. Select or create an object reference to continue.");
        } else {
            emptyNode->setValue(desc.readOnly ? "Read-only object" : "Object not available");
        }
        group->addSubProperty(emptyNode);
        return;
    }

    std::vector<SimulationControl*> children;
    for (SimulationControl* child : *childrenList->list()) {
        children.push_back(child);
    }

    recursionPath.insert(control);
    for (SimulationControl* child : children) {
        _appendDescriptorRecursively(group, child, recursionPath, depth + 1);
    }
    recursionPath.erase(control);
}

void ObjectPropertyBrowser::_populateKernelProperties(ModelDataDefinition* mdd) {
    if (mdd == nullptr) {
        return;
    }

    const std::vector<GenesysPropertyDescriptor> properties =
        GenesysPropertyIntrospection::describe(mdd->getProperties());

    std::map<std::string, QtProperty*> groups;

    for (const GenesysPropertyDescriptor& desc : properties) {
        std::string groupName = desc.ownerClassName.empty() ? "General" : desc.ownerClassName;

        auto found = groups.find(groupName);
        QtProperty* group = nullptr;

        if (found == groups.end()) {
            group = _groupManager->addProperty(QString::fromStdString(groupName));
            groups[groupName] = group;
            addProperty(group);
        } else {
            group = found->second;
        }

        std::set<const SimulationControl*> recursionPath;
        _appendDescriptorRecursively(group, desc.control, recursionPath);
    }
}

bool ObjectPropertyBrowser::_openSpecializedEditor(QtProperty* property) {
    if (!_hasValidActiveBindingContext(property)) {
        qWarning() << "[PropertyEditor] openSpecializedEditor aborted due to invalid binding context";
        return false;
    }

    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        return false;
    }

    const Binding binding = it.value();
    SimulationControl* control = binding.control;
    if (control == nullptr || _propertyEditor == nullptr) {
        return false;
    }

    auto refresh = [this]() {
        this->_notifyModelChangeApplied();
    };

    // This block opens the specialized list editor only when the explicit list-editor contract is enabled.
    if (binding.descriptor.supportsListEditor) {
        if (_propertyList != nullptr) {
            auto found = _propertyList->find(control);
            if (found == _propertyList->end() || found->second == nullptr) {
                (*_propertyList)[control] =
                    new DataComponentProperty(_propertyEditor, control, true, refresh);
            }
            (*_propertyList)[control]->open_window();
        } else {
            auto* editor = new DataComponentProperty(_propertyEditor, control, true, refresh);
            editor->open_window();
        }
        return true;
    }

    return false;
}

bool ObjectPropertyBrowser::_openSpecializedEditorForCurrentItem() {
    QtBrowserItem* item = currentItem();
    if (item == nullptr || item->property() == nullptr) {
        return false;
    }
    if (!_hasValidActiveBindingContext(item->property())) {
        qWarning() << "[PropertyEditor] openSpecializedEditorForCurrentItem aborted due to invalid binding context";
        return false;
    }
    return _openSpecializedEditor(item->property());
}

bool ObjectPropertyBrowser::_createObjectForProperty(QtProperty* property) {
    if (!_hasValidActiveBindingContext(property)) {
        qWarning() << "[PropertyEditor] createObjectForProperty aborted due to invalid binding context";
        return false;
    }

    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        return false;
    }

    const Binding binding = it.value();
    if (binding.control == nullptr) {
        return false;
    }
    if (!binding.descriptor.supportsObjectCreation || binding.control->hasObjectInstance()) {
        return false;
    }

    bool created = false;
    try {
        created = binding.control->createObjectInstance();
    } catch (...) {
        created = false;
    }

    if (!created) {
        return false;
    }

    _notifyModelChangeApplied();
    return true;
}


bool ObjectPropertyBrowser::_requiresCommitConfirmation(const Binding& binding) const {
    if (binding.descriptor.supportsInlineExpansion || binding.descriptor.supportsListEditor) {
        return false;
    }

    if (binding.descriptor.kind == GenesysPropertyKind::Enum
        || binding.descriptor.kind == GenesysPropertyKind::TimeUnit
        || binding.descriptor.kind == GenesysPropertyKind::Boolean) {
        return false;
    }

    return binding.descriptor.kind == GenesysPropertyKind::String
        || binding.descriptor.kind == GenesysPropertyKind::Integer
        || binding.descriptor.kind == GenesysPropertyKind::UnsignedInteger
        || binding.descriptor.kind == GenesysPropertyKind::UnsignedShort
        || binding.descriptor.kind == GenesysPropertyKind::Double
        || binding.descriptor.kind == GenesysPropertyKind::Unknown;
}

bool ObjectPropertyBrowser::_applyVariantChange(QtProperty* property, const QVariant& value, bool committed) {
    qInfo() << "[PropertyEditor] _applyVariantChange property="
            << (property != nullptr ? property->propertyName() : QString("<null>"))
            << " committed=" << committed;
    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        qInfo() << "[PropertyEditor] variant apply ignored due to missing binding";
        return false;
    }

    Binding& binding = it.value();
    if (binding.control == nullptr) {
        qWarning() << "[PropertyEditor] variant apply ignored due to null binding control";
        return false;
    }

    if (!_hasValidActiveBindingContext(property)) {
        qWarning() << "Ignoring variant apply because active binding context became invalid";
        return false;
    }

    if (binding.descriptor.supportsInlineExpansion || binding.descriptor.supportsListEditor
        || binding.descriptor.kind == GenesysPropertyKind::Enum
        || binding.descriptor.kind == GenesysPropertyKind::TimeUnit) {
        return false;
    }

    if (_requiresCommitConfirmation(binding) && !committed) {
        qInfo() << "[PropertyEditor] transient value change captured for" << property->propertyName();
        return false;
    }

    const std::string newValue = _fromVariant(binding.descriptor, value);
    if (newValue == binding.descriptor.currentValue) {
        qInfo() << "[PropertyEditor] variant apply ignored because value did not change";
        return false;
    }

    std::string errorMessage;
    const bool ok = GenesysPropertyIntrospection::setValue(
        binding.control,
        newValue,
        false,
        &errorMessage
        );

    if (!ok) {
        _scheduleDeferredRebuild();
        qWarning() << "[PropertyEditor] variant apply failed to apply value. Scheduled deferred rebuild";
        return false;
    }

    binding.descriptor.currentValue = newValue;
    qInfo() << "[PropertyEditor] applied committed variant change for" << property->propertyName();
    _notifyModelChangeApplied();
    return true;
}

void ObjectPropertyBrowser::onVariantEditorCommitted(QtProperty* property) {
    if (property == nullptr) {
        return;
    }

    if (_isRebuildingProperties) {
        qInfo() << "[PropertyEditor] editor commit ignored because rebuild is active";
        return;
    }

    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        return;
    }

    const Binding& binding = it.value();
    if (!_requiresCommitConfirmation(binding)) {
        return;
    }

    const QVariant pendingValue = _variantManager->value(property);
    _pendingCommittedProperties.insert(property);
    _pendingCommittedValues.insert(property, pendingValue);
    qInfo() << "[PropertyEditor] editor commit received for" << property->propertyName();
    QMetaObject::invokeMethod(this, [this, property]() {
        if (!_pendingCommittedProperties.contains(property)) {
            return;
        }
        if (!_hasValidActiveBindingContext(property)) {
            qWarning() << "[PropertyEditor] queued commit aborted due to invalid binding context";
            _pendingCommittedProperties.remove(property);
            _pendingCommittedValues.remove(property);
            return;
        }
        const QVariant committedValue = _pendingCommittedValues.value(property, _variantManager->value(property));
        qInfo() << "[PropertyEditor] queued commit apply for" << property->propertyName();
        _pendingCommittedProperties.remove(property);
        _pendingCommittedValues.remove(property);
        _applyVariantChange(property, committedValue, true);
    }, Qt::QueuedConnection);
}

void ObjectPropertyBrowser::valueChanged(QtProperty *property, const QVariant &value) {
    qInfo() << "[PropertyEditor] valueChanged enter";
    // Drop edits while a guarded rebuild is in progress to avoid reentrant mutation.
    if (_isRebuildingProperties) {
        qInfo() << "[PropertyEditor] valueChanged ignored because rebuild is active";
        return;
    }

    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        qInfo() << "[PropertyEditor] valueChanged ignored due to missing binding";
        return;
    }

    const Binding& binding = it.value();
    const bool requiresCommit = _requiresCommitConfirmation(binding);
    const bool committed = _pendingCommittedProperties.contains(property);

    if (requiresCommit && !committed) {
        qInfo() << "[PropertyEditor] valueChanged treated as transient for" << property->propertyName();
        return;
    }

    if (committed) {
        const QVariant pendingValue = _pendingCommittedValues.value(property);
        if (pendingValue.isValid() && pendingValue != value) {
            qInfo() << "[PropertyEditor] valueChanged waiting for committed value for" << property->propertyName();
            return;
        }
        _pendingCommittedProperties.remove(property);
        _pendingCommittedValues.remove(property);
    }

    _applyVariantChange(property, value, committed || !requiresCommit);
    qInfo() << "[PropertyEditor] valueChanged exit";
}

void ObjectPropertyBrowser::enumValueChanged(QtProperty *property, int value) {
    qInfo() << "[PropertyEditor] enumValueChanged enter";
    // Drop enum edits while a guarded rebuild is in progress to avoid reentrant mutation.
    if (_isRebuildingProperties) {
        qInfo() << "[PropertyEditor] enumValueChanged ignored because rebuild is active";
        return;
    }

    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        qInfo() << "[PropertyEditor] enumValueChanged ignored due to missing binding";
        return;
    }

    const Binding binding = it.value();
    if (binding.control == nullptr) {
        qWarning() << "[PropertyEditor] enumValueChanged ignored due to null binding control";
        return;
    }
    if (!_hasValidActiveBindingContext(property)) {
        qWarning() << "Ignoring enumValueChanged because active binding context became invalid";
        return;
    }

    if (value < 0 || value >= static_cast<int>(binding.descriptor.choices.size())) {
        return;
    }

    std::string newValue;
    if (binding.isObjectSelector || binding.descriptor.isModelDataDefinitionReference) {
        newValue = binding.descriptor.choices[static_cast<std::size_t>(value)];
    } else if (binding.descriptor.kind == GenesysPropertyKind::Enum
               || binding.descriptor.kind == GenesysPropertyKind::TimeUnit) {
        newValue = std::to_string(value);
    } else {
        return;
    }

    std::string errorMessage;
    const bool ok = GenesysPropertyIntrospection::setValue(
        binding.control,
        newValue,
        false,
        &errorMessage
        );

    if (!ok) {
        // Rebuild safely after failed enum update to restore editor consistency.
        _scheduleDeferredRebuild();
        qWarning() << "[PropertyEditor] enumValueChanged failed to apply value. Scheduled deferred rebuild";
        return;
    }

    _notifyModelChangeApplied();
    qInfo() << "[PropertyEditor] enumValueChanged exit";
}

void ObjectPropertyBrowser::_notifyModelChangeApplied() {
    qInfo() << "[PropertyEditor] _notifyModelChangeApplied enter";
    // Suppress nested notification loops when model callbacks trigger additional edits.
    if (_isNotifyingModelChange) {
        _pendingRebuild = true;
        qInfo() << "[PropertyEditor] _notifyModelChangeApplied detected nested notification";
        return;
    }

    _isNotifyingModelChange = true;
    _scheduleDeferredRebuild();
    _scheduleDeferredModelChangedCallback();
    _isNotifyingModelChange = false;

    qInfo() << "[PropertyEditor] _notifyModelChangeApplied exit";
}

void ObjectPropertyBrowser::objectUpdated() {
    // Route external updates through the guarded rebuild path to avoid recursive crashes.
    _scheduleDeferredRebuild();
}

void ObjectPropertyBrowser::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (_openSpecializedEditorForCurrentItem()) {
            event->accept();
            return;
        }
    }

    QtTreePropertyBrowser::keyPressEvent(event);
}

void ObjectPropertyBrowser::contextMenuEvent(QContextMenuEvent* event) {
    QtBrowserItem* item = currentItem();
    if (item == nullptr || item->property() == nullptr) {
        QtTreePropertyBrowser::contextMenuEvent(event);
        return;
    }

    auto it = _bindings.find(item->property());
    if (it == _bindings.end()) {
        QtTreePropertyBrowser::contextMenuEvent(event);
        return;
    }

    const Binding binding = it.value();
    if (binding.control == nullptr) {
        QtTreePropertyBrowser::contextMenuEvent(event);
        return;
    }
    QMenu menu(this);
    QAction* editAction = nullptr;
    QAction* createAction = nullptr;
    if (binding.descriptor.supportsListEditor) {
        editAction = menu.addAction("Edit list...");
    }
    if (binding.descriptor.supportsObjectCreation && !binding.control->hasObjectInstance()) {
        createAction = menu.addAction("Create object");
    }
    if (editAction == nullptr && createAction == nullptr) {
        QtTreePropertyBrowser::contextMenuEvent(event);
        return;
    }
    QAction* chosen = menu.exec(event->globalPos());

    if (chosen == editAction && editAction != nullptr) {
        _openSpecializedEditor(item->property());
    } else if (chosen == createAction && createAction != nullptr) {
        _createObjectForProperty(item->property());
    }
}

void ObjectPropertyBrowser::mouseDoubleClickEvent(QMouseEvent* event) {
    QtTreePropertyBrowser::mouseDoubleClickEvent(event);
    if (!_isRebuildingProperties) {
        _openSpecializedEditorForCurrentItem();
    }
}
