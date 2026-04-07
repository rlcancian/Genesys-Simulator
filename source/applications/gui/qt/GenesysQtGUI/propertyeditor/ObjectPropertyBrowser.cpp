#include "ObjectPropertyBrowser.h"

#include <map>
#include <sstream>
#include <exception>

#include <QSignalBlocker>
#include <QString>
#include <QVariant>
#include <QMenu>

namespace {

static std::vector<std::string> _copyStringList(List<std::string>* list) {
    std::vector<std::string> result;
    if (list == nullptr) {
        return result;
    }

    for (const std::string& value : *list->list()) {
        result.push_back(value);
    }

    delete list;
    return result;
}

static GenesysPropertyKind _deduceKind(const SimulationControl* control) {
    if (control == nullptr) {
        return GenesysPropertyKind::Unknown;
    }

    if (control->getIsList()) {
        return GenesysPropertyKind::List;
    }
    if (control->getIsClass()) {
        return GenesysPropertyKind::Object;
    }

    const std::string typeName = control->propertyType();

    if (typeName == Util::TypeOf<Util::TimeUnit>()) {
        return GenesysPropertyKind::TimeUnit;
    }
    if (control->getIsEnum()) {
        return GenesysPropertyKind::Enum;
    }
    if (typeName == Util::TypeOf<bool>()) {
        return GenesysPropertyKind::Boolean;
    }
    if (typeName == Util::TypeOf<int>()) {
        return GenesysPropertyKind::Integer;
    }
    if (typeName == Util::TypeOf<unsigned int>()) {
        return GenesysPropertyKind::UnsignedInteger;
    }
    if (typeName == Util::TypeOf<unsigned short>()) {
        return GenesysPropertyKind::UnsignedShort;
    }
    if (typeName == Util::TypeOf<double>()) {
        return GenesysPropertyKind::Double;
    }
    if (typeName == Util::TypeOf<std::string>()) {
        return GenesysPropertyKind::String;
    }

    return GenesysPropertyKind::Unknown;
}

static GenesysPropertyDescriptor _describeControl(SimulationControl* control) {
    GenesysPropertyDescriptor desc;
    if (control == nullptr) {
        return desc;
    }

    desc.control = control;
    desc.ownerClassName = control->getClassname();
    desc.ownerElementName = control->getElementName();
    desc.displayName = control->getName();
    desc.technicalTypeName = control->propertyType();
    desc.kind = _deduceKind(control);
    desc.readOnly = control->isReadOnly();
    desc.isList = control->getIsList();
    desc.isClass = control->getIsClass();
    desc.isEnum = control->getIsEnum();
    desc.currentValue = control->getValue();

    desc.choices = _copyStringList(control->getStrValues());

    if (desc.kind == GenesysPropertyKind::TimeUnit && desc.choices.empty()) {
        for (int i = 0; i < static_cast<int>(Util::TimeUnit::num_elements); ++i) {
            desc.choices.push_back(Util::convertEnumToStr(static_cast<Util::TimeUnit>(i)));
        }
    }

    return desc;
}

static std::vector<GenesysPropertyDescriptor> _describeControls(List<SimulationControl*>* controls) {
    std::vector<GenesysPropertyDescriptor> result;
    if (controls == nullptr) {
        return result;
    }

    for (SimulationControl* control : *controls->list()) {
        result.push_back(_describeControl(control));
    }

    return result;
}

static bool _setControlValue(
    SimulationControl* control,
    const std::string& value,
    bool remove,
    std::string* errorMessage
    ) {
    if (control == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = "SimulationControl nulo";
        }
        return false;
    }

    try {
        control->setValue(value, remove);
        return true;
    } catch (const std::exception& e) {
        if (errorMessage != nullptr) {
            *errorMessage = e.what();
        }
        return false;
    } catch (...) {
        if (errorMessage != nullptr) {
            *errorMessage = "Erro desconhecido ao alterar propriedade";
        }
        return false;
    }
}

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

    _clearAll();

    if (_modelObject == nullptr) {
        return;
    }

    _populateKernelProperties(_modelObject);
    objectUpdated();
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

QtProperty* ObjectPropertyBrowser::_createProperty(const GenesysPropertyDescriptor& desc) {
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
    case GenesysPropertyKind::String:
    case GenesysPropertyKind::Object:
    case GenesysPropertyKind::List:
    case GenesysPropertyKind::Unknown:
    case GenesysPropertyKind::Enum:
    case GenesysPropertyKind::TimeUnit:
    default:
        variantType = QVariant::String;
        break;
    }

    QtVariantProperty* property = _variantManager->addProperty(variantType, name);
    property->setValue(_toVariant(desc));

    const bool complexProperty = desc.isList || desc.isClass;

    const bool editableInline =
        !desc.readOnly &&
        !complexProperty &&
        !(desc.kind == GenesysPropertyKind::Enum || desc.kind == GenesysPropertyKind::TimeUnit);

    property->setEnabled(true);

    if (editableInline) {
        property->setValue(_toVariant(desc));
    }

    if (complexProperty) {
        property->setToolTip("Use duplo clique, Enter ou menu de contexto para editar.");
        property->setStatusTip("Complex property editor");
    }

    Binding binding;
    binding.owner = _modelObject;
    binding.control = desc.control;
    binding.descriptor = desc;
    _bindings[property] = binding;

    return property;
}

void ObjectPropertyBrowser::_populateKernelProperties(ModelDataDefinition* mdd) {
    if (mdd == nullptr) {
        return;
    }

    const std::vector<GenesysPropertyDescriptor> properties =
        _describeControls(mdd->getProperties());

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

        QtProperty* leaf = _createProperty(desc);
        if (group != nullptr && leaf != nullptr) {
            group->addSubProperty(leaf);
        }
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

    if (control->getIsList()) {
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

    if (control->getIsClass()) {
        if (_propertyEditorUI != nullptr) {
            auto found = _propertyEditorUI->find(control);
            if (found == _propertyEditorUI->end() || found->second == nullptr) {
                (*_propertyEditorUI)[control] =
                    new DataComponentEditor(_propertyEditor, control, refresh);
            }
            (*_propertyEditorUI)[control]->open_window(control);
        } else {
            auto* editor = new DataComponentEditor(_propertyEditor, control, refresh);
            editor->open_window(control);
        }
        return true;
    }

    if (binding.descriptor.kind == GenesysPropertyKind::Enum
        || binding.descriptor.kind == GenesysPropertyKind::TimeUnit
        || control->getIsEnum()) {
        if (_propertyBox != nullptr) {
            auto found = _propertyBox->find(control);
            if (found == _propertyBox->end() || found->second == nullptr) {
                (*_propertyBox)[control] = new ComboBoxEnum(_propertyEditor, control, refresh);
            }
            (*_propertyBox)[control]->open_box();
        } else {
            auto* box = new ComboBoxEnum(_propertyEditor, control, refresh);
            box->open_box();
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

void ObjectPropertyBrowser::valueChanged(QtProperty *property, const QVariant &value) {
    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        return;
    }

    const Binding binding = it.value();
    if (binding.control == nullptr) {
        return;
    }

    if (binding.descriptor.kind == GenesysPropertyKind::Enum
        || binding.descriptor.kind == GenesysPropertyKind::TimeUnit
        || binding.descriptor.kind == GenesysPropertyKind::Object
        || binding.descriptor.kind == GenesysPropertyKind::List) {
        return;
    }

    const std::string newValue = _fromVariant(binding.descriptor, value);
    if (newValue == binding.descriptor.currentValue) {
        return;
    }

    std::string errorMessage;
    const bool ok = _setControlValue(
        binding.control,
        newValue,
        false,
        &errorMessage
        );

    if (!ok) {
        objectUpdated();
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

    if (value == _enumIndexFor(binding.descriptor)) {
        return;
    }

    std::string errorMessage;
    const bool ok = _setControlValue(
        binding.control,
        std::to_string(value),
        false,
        &errorMessage
        );

    if (!ok) {
        objectUpdated();
        return;
    }

    _notifyModelChangeApplied();
}

void ObjectPropertyBrowser::_notifyModelChangeApplied() {
    objectUpdated();
    emit modelPropertiesChanged();
}

void ObjectPropertyBrowser::objectUpdated() {
    if (_modelObject == nullptr) {
        return;
    }

    QSignalBlocker blockerVariant(_variantManager);
    QSignalBlocker blockerEnum(_enumManager);

    const QList<QtProperty*> keys = _bindings.keys();
    for (QtProperty* key : keys) {
        Binding binding = _bindings.value(key);
        if (binding.control == nullptr) {
            continue;
        }

        GenesysPropertyDescriptor fresh = _describeControl(binding.control);

        binding.descriptor = fresh;
        _bindings[key] = binding;

        if ((fresh.kind == GenesysPropertyKind::Enum || fresh.kind == GenesysPropertyKind::TimeUnit)
            && !fresh.choices.empty()) {

            _enumNames[key] = _toQStringList(fresh.choices);
            _enumManager->setEnumNames(key, _enumNames[key]);
            _enumManager->setValue(key, _enumIndexFor(fresh));

        } else {
            QtVariantProperty* variantProperty =
                dynamic_cast<QtVariantProperty*>(key);
            if (variantProperty != nullptr) {
                variantProperty->setValue(_toVariant(fresh));
            }
        }
    }
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
    const bool specializedProperty =
        binding.descriptor.isList
        || binding.descriptor.isClass
        || binding.descriptor.kind == GenesysPropertyKind::Enum
        || binding.descriptor.kind == GenesysPropertyKind::TimeUnit;

    if (!specializedProperty) {
        QtTreePropertyBrowser::contextMenuEvent(event);
        return;
    }

    QMenu menu(this);
    QAction* editAction = menu.addAction("Edit...");
    QAction* chosen = menu.exec(event->globalPos());

    if (chosen == editAction) {
        _openSpecializedEditor(item->property());
    }
}

void ObjectPropertyBrowser::mouseDoubleClickEvent(QMouseEvent* event) {
    QtTreePropertyBrowser::mouseDoubleClickEvent(event);
    _openSpecializedEditorForCurrentItem();
}
