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

    _variantFactory = new QtVariantEditorFactory(this);
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
    _graphicalObject = nullptr;
    _modelObject = nullptr;
    _propertyEditor = nullptr;
    _clearAll();
}

void ObjectPropertyBrowser::setModelChangedCallback(ModelChangedCallback callback) {
    _modelChangedCallback = std::move(callback);
}

void ObjectPropertyBrowser::setActiveObject(
    QObject *obj,
    ModelDataDefinition* mdd,
    PropertyEditorGenesys* peg,
    std::map<SimulationControl*, DataComponentProperty*>* pl,
    std::map<SimulationControl*, DataComponentEditor*>* peUI,
    std::map<SimulationControl*, ComboBoxEnum*>* pb
    ) {
    _graphicalObject = obj;
    _modelObject = mdd;
    _propertyEditor = peg;
    _propertyList = pl;
    _propertyEditorUI = peUI;
    _propertyBox = pb;

    _rebuildProperties();
}

void ObjectPropertyBrowser::_rebuildProperties() {
    _clearAll();

    if (_modelObject == nullptr) {
        return;
    }

    _populateKernelProperties(_modelObject);
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
        emptyNode->setValue(desc.readOnly ? "Read-only object" : "Object not available");
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
    return _openSpecializedEditor(item->property());
}

bool ObjectPropertyBrowser::_createObjectForProperty(QtProperty* property) {
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

void ObjectPropertyBrowser::valueChanged(QtProperty *property, const QVariant &value) {
    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        return;
    }

    const Binding binding = it.value();
    if (binding.control == nullptr) {
        return;
    }

    // This block filters out properties whose edits are handled by specialized editors or enum handlers.
    if (binding.descriptor.supportsInlineExpansion || binding.descriptor.supportsListEditor
        || binding.descriptor.kind == GenesysPropertyKind::Enum
        || binding.descriptor.kind == GenesysPropertyKind::TimeUnit) {
        return;
    }

    const std::string newValue = _fromVariant(binding.descriptor, value);
    if (newValue == binding.descriptor.currentValue) {
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
        _rebuildProperties();
        return;
    }

    _notifyModelChangeApplied();
}

void ObjectPropertyBrowser::enumValueChanged(QtProperty *property, int value) {
    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        return;
    }

    const Binding binding = it.value();
    if (binding.control == nullptr) {
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
        _rebuildProperties();
        return;
    }

    _notifyModelChangeApplied();
}

void ObjectPropertyBrowser::_notifyModelChangeApplied() {
    _rebuildProperties();
    if (_modelChangedCallback) {
        _modelChangedCallback();
    }
}

void ObjectPropertyBrowser::objectUpdated() {
    _rebuildProperties();
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
    _openSpecializedEditorForCurrentItem();
}
