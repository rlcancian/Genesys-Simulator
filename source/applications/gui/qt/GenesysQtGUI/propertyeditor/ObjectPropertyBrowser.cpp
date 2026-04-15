#include "ObjectPropertyBrowser.h"
#include "../GuiScopeTrace.h"
#include "../animations/AnimationCounter.h"
#include "../animations/AnimationTimer.h"
#include "../animations/AnimationVariable.h"

#include <map>
#include <set>
#include <sstream>
#include <exception>
#include <utility>

#include <QSignalBlocker>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QString>
#include <QStringList>
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

QString graphicsItemTypeName(QGraphicsItem* item) {
    if (dynamic_cast<QGraphicsLineItem*>(item) != nullptr) {
        return "Line";
    }
    if (dynamic_cast<QGraphicsRectItem*>(item) != nullptr) {
        return "Rectangle";
    }
    if (dynamic_cast<QGraphicsEllipseItem*>(item) != nullptr) {
        return "Ellipse";
    }
    if (dynamic_cast<QGraphicsPolygonItem*>(item) != nullptr) {
        return "Polygon";
    }
    if (dynamic_cast<QGraphicsTextItem*>(item) != nullptr) {
        return "Text";
    }
    if (dynamic_cast<QGraphicsItemGroup*>(item) != nullptr) {
        return "Group";
    }
    return "Graphics item";
}

QStringList penStyleNames() {
    return {"No Pen", "Solid", "Dash", "Dot", "Dash Dot", "Dash Dot Dot"};
}

Qt::PenStyle penStyleFromIndex(int index) {
    switch (index) {
    case 0:
        return Qt::NoPen;
    case 2:
        return Qt::DashLine;
    case 3:
        return Qt::DotLine;
    case 4:
        return Qt::DashDotLine;
    case 5:
        return Qt::DashDotDotLine;
    case 1:
    default:
        return Qt::SolidLine;
    }
}

int indexFromPenStyle(Qt::PenStyle style) {
    switch (style) {
    case Qt::NoPen:
        return 0;
    case Qt::DashLine:
        return 2;
    case Qt::DotLine:
        return 3;
    case Qt::DashDotLine:
        return 4;
    case Qt::DashDotDotLine:
        return 5;
    case Qt::SolidLine:
    default:
        return 1;
    }
}

QString polygonToString(const QPolygonF& polygon) {
    QStringList points;
    for (const QPointF& point : polygon) {
        points << QString("%1,%2").arg(point.x(), 0, 'f', 4).arg(point.y(), 0, 'f', 4);
    }
    return points.join("; ");
}

QPolygonF polygonFromString(const QString& text, bool* ok) {
    QPolygonF polygon;
    bool parsed = true;
    const QStringList pointTokens = text.split(";", Qt::SkipEmptyParts);
    for (const QString& pointToken : pointTokens) {
        const QStringList coordinates = pointToken.trimmed().split(",");
        if (coordinates.size() != 2) {
            parsed = false;
            continue;
        }
        bool xOk = false;
        bool yOk = false;
        const qreal x = coordinates.at(0).trimmed().toDouble(&xOk);
        const qreal y = coordinates.at(1).trimmed().toDouble(&yOk);
        if (!xOk || !yOk) {
            parsed = false;
            continue;
        }
        polygon << QPointF(x, y);
    }

    if (ok != nullptr) {
        *ok = parsed && !polygon.isEmpty();
    }
    return polygon;
}
} // namespace

ObjectPropertyBrowser::ObjectPropertyBrowser(QWidget* parent)
    : QtTreePropertyBrowser(parent) {
    // Initialize browser managers/factories once before the first clear/rebuild.
    _ensureBrowserInfrastructure();

    _clearAll();

    setResizeMode(QtTreePropertyBrowser::Interactive);
    setAccessibleDescription("Editor for model elements");
    setHeaderVisible(true);
    setIndentation(10);
    setRootIsDecorated(true);
    setAlternatingRowColors(true);
}

void ObjectPropertyBrowser::_ensureBrowserInfrastructure() {
    // Keep a trace for one-time browser infrastructure setup diagnostics.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::_ensureBrowserInfrastructure", this);

    if (_variantManager == nullptr) {
        // Create the variant manager only once and keep QObject parent ownership.
        _variantManager = new QtVariantPropertyManager(this);
        connect(
            _variantManager,
            SIGNAL(valueChanged(QtProperty*,QVariant)),
            this,
            SLOT(valueChanged(QtProperty*,QVariant))
            );
    }

    if (_groupManager == nullptr) {
        // Create the group manager only once for grouped properties.
        _groupManager = new QtGroupPropertyManager(this);
    }

    if (_enumManager == nullptr) {
        // Create the enum manager only once and connect value notifications once.
        _enumManager = new QtEnumPropertyManager(this);
        connect(
            _enumManager,
            SIGNAL(valueChanged(QtProperty*,int)),
            this,
            SLOT(enumValueChanged(QtProperty*,int))
            );
    }

    if (_variantFactory == nullptr) {
        // Preserve commit-aware variant editor factory behavior with one-time creation.
        auto* commitFactory = new CommitAwareVariantEditorFactory(this);
        commitFactory->setCommitCallback([this](QtProperty* property) {
            onVariantEditorCommitted(property);
        });
        _variantFactory = commitFactory;
    }

    if (_enumFactory == nullptr) {
        // Create the enum factory once for enum editor widgets.
        _enumFactory = new QtEnumEditorFactory(this);
    }

    // Bind manager/factory pairs only once to keep browser infrastructure idempotent.
    if (!_browserInfrastructureBound
        && _variantManager != nullptr
        && _variantFactory != nullptr
        && _enumManager != nullptr
        && _enumFactory != nullptr) {
        setFactoryForManager(_variantManager, _variantFactory);
        setFactoryForManager(_enumManager, _enumFactory);
        _browserInfrastructureBound = true;
    }
}

void ObjectPropertyBrowser::_clearAll() {
    // Adds scoped tracing for critical Property Editor crash-diagnosis paths.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::_clearAll", this);
    // Ensure infrastructure is available while clearing only transient browser content.
    _ensureBrowserInfrastructure();
    clear();
    _bindings.clear();
    _enumNames.clear();
    _pendingCommittedProperties.clear();
    _pendingCommittedValues.clear();
}

void ObjectPropertyBrowser::clearCurrentlyConnectedObject() {
    // Adds scoped tracing for critical Property Editor crash-diagnosis paths.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::clearCurrentlyConnectedObject", this);
    // Fully detach object/editor pointers before clearing UI bindings.
    _graphicalObject = nullptr;
    _graphicalItem = nullptr;
    _modelObject = nullptr;
    _activeMode = ActiveMode::None;
    _graphicallyRepresentedModelObjects.clear();
    _propertyEditor = nullptr;
    _isRebuildingProperties = false;
    _isNotifyingModelChange = false;
    _pendingRebuild = false;
    _isDeferredRebuildScheduled = false;
    _isDeferredModelChangedScheduled = false;
    // Logs binding state reset to correlate object detachment with rebuild activity.
    qInfo() << "[PropertyEditor] clearCurrentlyConnectedObject state graphical=" << static_cast<void*>(_graphicalObject.data())
            << " model=" << static_cast<void*>(_modelObject)
            << " rebuilding=" << _isRebuildingProperties
            << " notifying=" << _isNotifyingModelChange
            << " pendingRebuild=" << _pendingRebuild;
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
    const QSet<QString>& graphicallyRepresentedModelObjects,
    PropertyEditorGenesys* peg,
    std::map<SimulationControl*, DataComponentProperty*>* pl,
    std::map<SimulationControl*, DataComponentEditor*>* peUI,
    std::map<SimulationControl*, ComboBoxEnum*>* pb
    ) {
    // Adds scoped tracing for critical Property Editor crash-diagnosis paths.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::setActiveObject", this);
    // Always detach stale bindings first to avoid stale-pointer use during rebinding.
    clearCurrentlyConnectedObject();

    // Logs new active binding pointers and state for selection-to-editor diagnostics.
    qInfo() << "[PropertyEditor] setActiveObject bind graphical=" << static_cast<void*>(obj)
            << " model=" << static_cast<void*>(mdd)
            << " rebuilding=" << _isRebuildingProperties
            << " notifying=" << _isNotifyingModelChange
            << " pendingRebuild=" << _pendingRebuild;
    // Bind the new active object and editor dependencies for the next safe rebuild.
    _graphicalObject = obj;
    _graphicalItem = dynamic_cast<QGraphicsItem*>(obj);
    _modelObject = mdd;
    _activeMode = (mdd != nullptr) ? ActiveMode::KernelObject : ActiveMode::None;
    _graphicallyRepresentedModelObjects = graphicallyRepresentedModelObjects;
    _propertyEditor = peg;
    _propertyList = pl;
    _propertyEditorUI = peUI;
    _propertyBox = pb;

    // Rebuild once with guard logic so nested signals cannot recurse unsafely.
    _scheduleDeferredRebuild();
}

void ObjectPropertyBrowser::setActiveGraphicsItem(QGraphicsItem* item) {
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::setActiveGraphicsItem", this);
    clearCurrentlyConnectedObject();

    if (item == nullptr) {
        return;
    }

    _graphicalItem = item;
    _graphicalObject = item->toGraphicsObject();
    _activeMode = ActiveMode::GraphicsItem;

    qInfo() << "[PropertyEditor] setActiveGraphicsItem bind item=" << static_cast<void*>(item)
            << " type=" << graphicsItemTypeName(item);

    _scheduleDeferredRebuild();
}

// Rebuild properties with explicit suppression of nested recursive rebuild execution.
void ObjectPropertyBrowser::_rebuildPropertiesGuarded() {
    // Adds scoped tracing for critical Property Editor crash-diagnosis paths.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::_rebuildPropertiesGuarded", this);
    // Logs guarded rebuild state transitions for crash-path observability.
    qInfo() << "[PropertyEditor] _rebuildPropertiesGuarded enter. graphical=" << static_cast<void*>(_graphicalObject.data())
            << " model=" << static_cast<void*>(_modelObject)
            << " rebuilding=" << _isRebuildingProperties
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
    if (_activeMode == ActiveMode::GraphicsItem) {
        if (_graphicalItem == nullptr) {
            return false;
        }
        if (property != nullptr) {
            auto it = _bindings.constFind(property);
            if (it == _bindings.constEnd()) {
                return false;
            }
            const Binding& binding = it.value();
            if (binding.kind != BindingKind::GraphicsVariant && binding.kind != BindingKind::GraphicsEnum) {
                return false;
            }
            if (binding.graphicsItem != _graphicalItem) {
                return false;
            }
        }
        return true;
    }

    if (_activeMode != ActiveMode::KernelObject || _modelObject == nullptr || _graphicalObject.isNull()) {
        return false;
    }

    if (property != nullptr) {
        auto it = _bindings.constFind(property);
        if (it == _bindings.constEnd()) {
            return false;
        }
        const Binding& binding = it.value();
        if (binding.kind != BindingKind::Kernel) {
            return false;
        }
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
    // Adds scoped tracing for critical Property Editor crash-diagnosis paths.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::_rebuildProperties", this);
    // Logs rebuild context pointers and flags before mutating property browser structures.
    qInfo() << "[PropertyEditor] _rebuildProperties enter graphical=" << static_cast<void*>(_graphicalObject.data())
            << " model=" << static_cast<void*>(_modelObject)
            << " rebuilding=" << _isRebuildingProperties
            << " notifying=" << _isNotifyingModelChange
            << " pendingRebuild=" << _pendingRebuild;
    // Clear existing browser state first so stale bindings cannot survive across rebuilds.
    _clearAll();

    if (_activeMode == ActiveMode::GraphicsItem) {
        if (_graphicalItem == nullptr) {
            qInfo() << "Skipping property rebuild because graphics item is null";
            return;
        }
        _populateGraphicsProperties(_graphicalItem);
        qInfo() << "[PropertyEditor] _rebuildProperties exit";
        return;
    }

    if (_activeMode != ActiveMode::KernelObject || _modelObject == nullptr) {
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

QtProperty* ObjectPropertyBrowser::_createGraphicalReferenceProperty(const GenesysPropertyDescriptor& desc) {
    QtVariantProperty* property = _variantManager->addProperty(
        QVariant::String,
        QString::fromStdString(desc.displayName)
        );
    QString value = QString::fromStdString(desc.currentValue);
    if (value.trimmed().isEmpty()) {
        value = "Edit this property by selecting the corresponding object in the model";
    } else {
        value = QString("%1 - Edit this object by selecting it in the model").arg(value);
    }
    property->setValue(value);
    property->setEnabled(false);

    Binding binding;
    binding.owner = _modelObject;
    binding.control = desc.control;
    binding.descriptor = desc;
    _bindings[property] = binding;

    return property;
}

bool ObjectPropertyBrowser::_hasGraphicalRepresentation(const GenesysPropertyDescriptor& desc) const {
    if (!desc.isModelDataDefinitionReference || desc.currentValue.empty()) {
        return false;
    }

    return _graphicallyRepresentedModelObjects.contains(QString::fromStdString(desc.currentValue));
}

QtVariantProperty* ObjectPropertyBrowser::_createGraphicsVariantProperty(
    const QString& name,
    int variantType,
    const QVariant& value,
    std::function<void(const QVariant&)> setter,
    bool requiresCommit,
    bool enabled
    ) {
    QtVariantProperty* property = _variantManager->addProperty(variantType, name);
    property->setValue(value);
    property->setEnabled(enabled && setter != nullptr);

    Binding binding;
    binding.kind = BindingKind::GraphicsVariant;
    binding.graphicsItem = _graphicalItem;
    binding.graphicsRequiresCommit = requiresCommit;
    binding.graphicsVariantSetter = std::move(setter);
    _bindings[property] = binding;

    return property;
}

QtProperty* ObjectPropertyBrowser::_createGraphicsEnumProperty(
    const QString& name,
    const QStringList& choices,
    int value,
    std::function<void(int)> setter,
    bool enabled
    ) {
    QtProperty* property = _enumManager->addProperty(name);
    _enumNames[property] = choices;
    _enumManager->setEnumNames(property, choices);
    _enumManager->setValue(property, value);
    property->setEnabled(enabled && setter != nullptr);

    Binding binding;
    binding.kind = BindingKind::GraphicsEnum;
    binding.graphicsItem = _graphicalItem;
    binding.graphicsEnumSetter = std::move(setter);
    _bindings[property] = binding;

    return property;
}

void ObjectPropertyBrowser::_notifyGraphicsChangeApplied(QGraphicsItem* item) {
    if (item == nullptr) {
        return;
    }

    item->update();
    if (QGraphicsScene* itemScene = item->scene()) {
        itemScene->update();
    }
}

void ObjectPropertyBrowser::_appendCommonGraphicsProperties(QtProperty* group, QGraphicsItem* item) {
    if (group == nullptr || item == nullptr) {
        return;
    }

    group->addSubProperty(_createGraphicsVariantProperty(
        "Type",
        QVariant::String,
        graphicsItemTypeName(item),
        nullptr,
        false,
        false
        ));

    group->addSubProperty(_createGraphicsVariantProperty(
        "X",
        QVariant::Double,
        item->pos().x(),
        [item, this](const QVariant& value) {
            item->setX(value.toDouble());
            _notifyGraphicsChangeApplied(item);
        },
        true
        ));
    group->addSubProperty(_createGraphicsVariantProperty(
        "Y",
        QVariant::Double,
        item->pos().y(),
        [item, this](const QVariant& value) {
            item->setY(value.toDouble());
            _notifyGraphicsChangeApplied(item);
        },
        true
        ));
    group->addSubProperty(_createGraphicsVariantProperty(
        "Rotation",
        QVariant::Double,
        item->rotation(),
        [item, this](const QVariant& value) {
            item->setRotation(value.toDouble());
            _notifyGraphicsChangeApplied(item);
        },
        true
        ));
    group->addSubProperty(_createGraphicsVariantProperty(
        "Scale",
        QVariant::Double,
        item->scale(),
        [item, this](const QVariant& value) {
            item->setScale(value.toDouble());
            _notifyGraphicsChangeApplied(item);
        },
        true
        ));
    group->addSubProperty(_createGraphicsVariantProperty(
        "Z value",
        QVariant::Double,
        item->zValue(),
        [item, this](const QVariant& value) {
            item->setZValue(value.toDouble());
            _notifyGraphicsChangeApplied(item);
        },
        true
        ));
    group->addSubProperty(_createGraphicsVariantProperty(
        "Visible",
        QVariant::Bool,
        item->isVisible(),
        [item, this](const QVariant& value) {
            item->setVisible(value.toBool());
            _notifyGraphicsChangeApplied(item);
        }
        ));
    group->addSubProperty(_createGraphicsVariantProperty(
        "Opacity",
        QVariant::Double,
        item->opacity(),
        [item, this](const QVariant& value) {
            item->setOpacity(value.toDouble());
            _notifyGraphicsChangeApplied(item);
        },
        true
        ));
}

void ObjectPropertyBrowser::_appendPenProperties(QtProperty* group, const QPen& pen, std::function<void(const QPen&)> setter) {
    if (group == nullptr || setter == nullptr) {
        return;
    }

    group->addSubProperty(_createGraphicsVariantProperty(
        "Line color",
        QMetaType::QColor,
        pen.color(),
        [pen, setter, this](const QVariant& value) {
            QPen updatedPen = pen;
            updatedPen.setColor(value.value<QColor>());
            setter(updatedPen);
            _notifyGraphicsChangeApplied(_graphicalItem);
        }
        ));
    group->addSubProperty(_createGraphicsVariantProperty(
        "Line width",
        QVariant::Double,
        pen.widthF(),
        [pen, setter, this](const QVariant& value) {
            QPen updatedPen = pen;
            updatedPen.setWidthF(value.toDouble());
            setter(updatedPen);
            _notifyGraphicsChangeApplied(_graphicalItem);
        },
        true
        ));
    group->addSubProperty(_createGraphicsEnumProperty(
        "Line style",
        penStyleNames(),
        indexFromPenStyle(pen.style()),
        [pen, setter, this](int value) {
            QPen updatedPen = pen;
            updatedPen.setStyle(penStyleFromIndex(value));
            setter(updatedPen);
            _notifyGraphicsChangeApplied(_graphicalItem);
        }
        ));
}

void ObjectPropertyBrowser::_appendBrushProperties(QtProperty* group, const QBrush& brush, std::function<void(const QBrush&)> setter) {
    if (group == nullptr || setter == nullptr) {
        return;
    }

    group->addSubProperty(_createGraphicsVariantProperty(
        "Fill color",
        QMetaType::QColor,
        brush.color(),
        [brush, setter, this](const QVariant& value) {
            QBrush updatedBrush = brush;
            updatedBrush.setColor(value.value<QColor>());
            if (updatedBrush.style() == Qt::NoBrush) {
                updatedBrush.setStyle(Qt::SolidPattern);
            }
            setter(updatedBrush);
            _notifyGraphicsChangeApplied(_graphicalItem);
        }
        ));
    group->addSubProperty(_createGraphicsVariantProperty(
        "Filled",
        QVariant::Bool,
        brush.style() != Qt::NoBrush,
        [brush, setter, this](const QVariant& value) {
            QBrush updatedBrush = brush;
            updatedBrush.setStyle(value.toBool() ? Qt::SolidPattern : Qt::NoBrush);
            setter(updatedBrush);
            _notifyGraphicsChangeApplied(_graphicalItem);
        }
        ));
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

    if (_hasGraphicalRepresentation(desc)) {
        QtProperty* reference = _createGraphicalReferenceProperty(desc);
        if (reference != nullptr) {
            parent->addSubProperty(reference);
        }
        return;
    }

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

void ObjectPropertyBrowser::_populateGraphicsProperties(QGraphicsItem* item) {
    if (item == nullptr) {
        return;
    }

    QtProperty* graphicsGroup = _groupManager->addProperty("Graphics");
    addProperty(graphicsGroup);
    _appendCommonGraphicsProperties(graphicsGroup, item);

    if (QGraphicsLineItem* lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
        QtProperty* lineGroup = _groupManager->addProperty("Line");
        addProperty(lineGroup);
        const QLineF line = lineItem->line();
        lineGroup->addSubProperty(_createGraphicsVariantProperty(
            "X1", QVariant::Double, line.x1(),
            [lineItem, this](const QVariant& value) {
                QLineF line = lineItem->line();
                line.setP1(QPointF(value.toDouble(), line.y1()));
                lineItem->setLine(line);
                _notifyGraphicsChangeApplied(lineItem);
            }, true));
        lineGroup->addSubProperty(_createGraphicsVariantProperty(
            "Y1", QVariant::Double, line.y1(),
            [lineItem, this](const QVariant& value) {
                QLineF line = lineItem->line();
                line.setP1(QPointF(line.x1(), value.toDouble()));
                lineItem->setLine(line);
                _notifyGraphicsChangeApplied(lineItem);
            }, true));
        lineGroup->addSubProperty(_createGraphicsVariantProperty(
            "X2", QVariant::Double, line.x2(),
            [lineItem, this](const QVariant& value) {
                QLineF line = lineItem->line();
                line.setP2(QPointF(value.toDouble(), line.y2()));
                lineItem->setLine(line);
                _notifyGraphicsChangeApplied(lineItem);
            }, true));
        lineGroup->addSubProperty(_createGraphicsVariantProperty(
            "Y2", QVariant::Double, line.y2(),
            [lineItem, this](const QVariant& value) {
                QLineF line = lineItem->line();
                line.setP2(QPointF(line.x2(), value.toDouble()));
                lineItem->setLine(line);
                _notifyGraphicsChangeApplied(lineItem);
            }, true));
        _appendPenProperties(lineGroup, lineItem->pen(), [lineItem](const QPen& pen) {
            lineItem->setPen(pen);
        });
        return;
    }

    if (QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
        QtProperty* rectGroup = _groupManager->addProperty("Rectangle");
        addProperty(rectGroup);
        const QRectF rect = rectItem->rect();
        rectGroup->addSubProperty(_createGraphicsVariantProperty(
            "Local X", QVariant::Double, rect.x(),
            [rectItem, this](const QVariant& value) {
                QRectF rect = rectItem->rect();
                rect.moveLeft(value.toDouble());
                rectItem->setRect(rect);
                _notifyGraphicsChangeApplied(rectItem);
            }, true));
        rectGroup->addSubProperty(_createGraphicsVariantProperty(
            "Local Y", QVariant::Double, rect.y(),
            [rectItem, this](const QVariant& value) {
                QRectF rect = rectItem->rect();
                rect.moveTop(value.toDouble());
                rectItem->setRect(rect);
                _notifyGraphicsChangeApplied(rectItem);
            }, true));
        rectGroup->addSubProperty(_createGraphicsVariantProperty(
            "Width", QVariant::Double, rect.width(),
            [rectItem, this](const QVariant& value) {
                QRectF rect = rectItem->rect();
                rect.setWidth(value.toDouble());
                rectItem->setRect(rect);
                _notifyGraphicsChangeApplied(rectItem);
            }, true));
        rectGroup->addSubProperty(_createGraphicsVariantProperty(
            "Height", QVariant::Double, rect.height(),
            [rectItem, this](const QVariant& value) {
                QRectF rect = rectItem->rect();
                rect.setHeight(value.toDouble());
                rectItem->setRect(rect);
                _notifyGraphicsChangeApplied(rectItem);
            }, true));
        _appendPenProperties(rectGroup, rectItem->pen(), [rectItem](const QPen& pen) {
            rectItem->setPen(pen);
        });
        _appendBrushProperties(rectGroup, rectItem->brush(), [rectItem](const QBrush& brush) {
            rectItem->setBrush(brush);
        });

        if (AnimationCounter* counter = dynamic_cast<AnimationCounter*>(rectItem)) {
            QtProperty* animationGroup = _groupManager->addProperty("Animation counter");
            addProperty(animationGroup);
            animationGroup->addSubProperty(_createGraphicsVariantProperty(
                "Value",
                QVariant::Double,
                counter->getValue(),
                [counter, this](const QVariant& value) {
                    counter->setValue(value.toDouble());
                    _notifyGraphicsChangeApplied(counter);
                },
                true
                ));
        } else if (AnimationVariable* variable = dynamic_cast<AnimationVariable*>(rectItem)) {
            QtProperty* animationGroup = _groupManager->addProperty("Animation variable");
            addProperty(animationGroup);
            animationGroup->addSubProperty(_createGraphicsVariantProperty(
                "Value",
                QVariant::Double,
                variable->getValue(),
                [variable, this](const QVariant& value) {
                    variable->setValue(value.toDouble());
                    _notifyGraphicsChangeApplied(variable);
                },
                true
                ));
        } else if (AnimationTimer* timer = dynamic_cast<AnimationTimer*>(rectItem)) {
            QtProperty* animationGroup = _groupManager->addProperty("Animation timer");
            addProperty(animationGroup);
            animationGroup->addSubProperty(_createGraphicsVariantProperty(
                "Time",
                QVariant::Double,
                timer->getTime(),
                [timer, this](const QVariant& value) {
                    timer->setTime(value.toDouble());
                    _notifyGraphicsChangeApplied(timer);
                },
                true
                ));
            animationGroup->addSubProperty(_createGraphicsVariantProperty(
                "Initial hours",
                QVariant::Int,
                static_cast<int>(timer->getInitialHours()),
                [timer, this](const QVariant& value) {
                    timer->setInitialHours(static_cast<unsigned int>(value.toInt()));
                    _notifyGraphicsChangeApplied(timer);
                },
                true
                ));
            animationGroup->addSubProperty(_createGraphicsVariantProperty(
                "Initial minutes",
                QVariant::Int,
                static_cast<int>(timer->getInitialMinutes()),
                [timer, this](const QVariant& value) {
                    timer->setInitialMinutes(static_cast<unsigned int>(value.toInt()));
                    _notifyGraphicsChangeApplied(timer);
                },
                true
                ));
            animationGroup->addSubProperty(_createGraphicsVariantProperty(
                "Initial seconds",
                QVariant::Int,
                static_cast<int>(timer->getInitialSeconds()),
                [timer, this](const QVariant& value) {
                    timer->setInitialSeconds(static_cast<unsigned int>(value.toInt()));
                    _notifyGraphicsChangeApplied(timer);
                },
                true
                ));
        }
        return;
    }

    if (QGraphicsEllipseItem* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
        QtProperty* ellipseGroup = _groupManager->addProperty("Ellipse");
        addProperty(ellipseGroup);
        const QRectF rect = ellipseItem->rect();
        ellipseGroup->addSubProperty(_createGraphicsVariantProperty(
            "Local X", QVariant::Double, rect.x(),
            [ellipseItem, this](const QVariant& value) {
                QRectF rect = ellipseItem->rect();
                rect.moveLeft(value.toDouble());
                ellipseItem->setRect(rect);
                _notifyGraphicsChangeApplied(ellipseItem);
            }, true));
        ellipseGroup->addSubProperty(_createGraphicsVariantProperty(
            "Local Y", QVariant::Double, rect.y(),
            [ellipseItem, this](const QVariant& value) {
                QRectF rect = ellipseItem->rect();
                rect.moveTop(value.toDouble());
                ellipseItem->setRect(rect);
                _notifyGraphicsChangeApplied(ellipseItem);
            }, true));
        ellipseGroup->addSubProperty(_createGraphicsVariantProperty(
            "Width", QVariant::Double, rect.width(),
            [ellipseItem, this](const QVariant& value) {
                QRectF rect = ellipseItem->rect();
                rect.setWidth(value.toDouble());
                ellipseItem->setRect(rect);
                _notifyGraphicsChangeApplied(ellipseItem);
            }, true));
        ellipseGroup->addSubProperty(_createGraphicsVariantProperty(
            "Height", QVariant::Double, rect.height(),
            [ellipseItem, this](const QVariant& value) {
                QRectF rect = ellipseItem->rect();
                rect.setHeight(value.toDouble());
                ellipseItem->setRect(rect);
                _notifyGraphicsChangeApplied(ellipseItem);
            }, true));
        _appendPenProperties(ellipseGroup, ellipseItem->pen(), [ellipseItem](const QPen& pen) {
            ellipseItem->setPen(pen);
        });
        _appendBrushProperties(ellipseGroup, ellipseItem->brush(), [ellipseItem](const QBrush& brush) {
            ellipseItem->setBrush(brush);
        });
        return;
    }

    if (QGraphicsPolygonItem* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item)) {
        QtProperty* polygonGroup = _groupManager->addProperty("Polygon");
        addProperty(polygonGroup);
        polygonGroup->addSubProperty(_createGraphicsVariantProperty(
            "Points",
            QVariant::String,
            polygonToString(polygonItem->polygon()),
            [polygonItem, this](const QVariant& value) {
                bool ok = false;
                const QPolygonF polygon = polygonFromString(value.toString(), &ok);
                if (!ok) {
                    return;
                }
                polygonItem->setPolygon(polygon);
                _notifyGraphicsChangeApplied(polygonItem);
            },
            true
            ));
        _appendPenProperties(polygonGroup, polygonItem->pen(), [polygonItem](const QPen& pen) {
            polygonItem->setPen(pen);
        });
        _appendBrushProperties(polygonGroup, polygonItem->brush(), [polygonItem](const QBrush& brush) {
            polygonItem->setBrush(brush);
        });
        return;
    }

    if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
        QtProperty* textGroup = _groupManager->addProperty("Text");
        addProperty(textGroup);
        textGroup->addSubProperty(_createGraphicsVariantProperty(
            "Content",
            QVariant::String,
            textItem->toPlainText(),
            [textItem, this](const QVariant& value) {
                textItem->setPlainText(value.toString());
                _notifyGraphicsChangeApplied(textItem);
            },
            true
            ));
        textGroup->addSubProperty(_createGraphicsVariantProperty(
            "Text color",
            QMetaType::QColor,
            textItem->defaultTextColor(),
            [textItem, this](const QVariant& value) {
                textItem->setDefaultTextColor(value.value<QColor>());
                _notifyGraphicsChangeApplied(textItem);
            }
            ));
        textGroup->addSubProperty(_createGraphicsVariantProperty(
            "Font",
            QMetaType::QFont,
            textItem->font(),
            [textItem, this](const QVariant& value) {
                textItem->setFont(value.value<QFont>());
                _notifyGraphicsChangeApplied(textItem);
            }
            ));
        textGroup->addSubProperty(_createGraphicsVariantProperty(
            "Text width",
            QVariant::Double,
            textItem->textWidth(),
            [textItem, this](const QVariant& value) {
                textItem->setTextWidth(value.toDouble());
                _notifyGraphicsChangeApplied(textItem);
            },
            true
            ));
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
    if (binding.kind == BindingKind::GraphicsVariant) {
        return binding.graphicsRequiresCommit;
    }
    if (binding.kind == BindingKind::GraphicsEnum) {
        return false;
    }

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

bool ObjectPropertyBrowser::_applyGraphicsVariantChange(QtProperty* property, const QVariant& value, bool committed) {
    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        return false;
    }

    Binding& binding = it.value();
    if (binding.kind != BindingKind::GraphicsVariant || binding.graphicsVariantSetter == nullptr) {
        return false;
    }

    if (!_hasValidActiveBindingContext(property)) {
        return false;
    }

    if (binding.graphicsRequiresCommit && !committed) {
        return false;
    }

    binding.graphicsVariantSetter(value);
    return true;
}

bool ObjectPropertyBrowser::_applyGraphicsEnumChange(QtProperty* property, int value) {
    auto it = _bindings.find(property);
    if (it == _bindings.end()) {
        return false;
    }

    Binding& binding = it.value();
    if (binding.kind != BindingKind::GraphicsEnum || binding.graphicsEnumSetter == nullptr) {
        return false;
    }

    if (!_hasValidActiveBindingContext(property)) {
        return false;
    }

    binding.graphicsEnumSetter(value);
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
        auto bindingIt = _bindings.find(property);
        if (bindingIt != _bindings.end() && bindingIt.value().kind == BindingKind::GraphicsVariant) {
            _applyGraphicsVariantChange(property, committedValue, true);
            return;
        }
        _applyVariantChange(property, committedValue, true);
    }, Qt::QueuedConnection);
}

void ObjectPropertyBrowser::valueChanged(QtProperty *property, const QVariant &value) {
    // Adds scoped tracing for critical Property Editor crash-diagnosis paths.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::valueChanged", this);
    // Logs value-change state for commit pipeline and active-object diagnostics.
    qInfo() << "[PropertyEditor] valueChanged enter graphical=" << static_cast<void*>(_graphicalObject.data())
            << " model=" << static_cast<void*>(_modelObject)
            << " rebuilding=" << _isRebuildingProperties
            << " notifying=" << _isNotifyingModelChange
            << " pendingRebuild=" << _pendingRebuild;
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

    if (binding.kind == BindingKind::GraphicsVariant) {
        if (requiresCommit && !committed) {
            qInfo() << "[PropertyEditor] graphics valueChanged treated as transient for" << property->propertyName();
            return;
        }
        if (committed) {
            const QVariant pendingValue = _pendingCommittedValues.value(property);
            if (pendingValue.isValid() && pendingValue != value) {
                return;
            }
            _pendingCommittedProperties.remove(property);
            _pendingCommittedValues.remove(property);
        }
        _applyGraphicsVariantChange(property, value, committed || !requiresCommit);
        qInfo() << "[PropertyEditor] valueChanged exit";
        return;
    }

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
    // Adds scoped tracing for critical Property Editor crash-diagnosis paths.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::enumValueChanged", this);
    // Logs enum-change state for commit pipeline and active-object diagnostics.
    qInfo() << "[PropertyEditor] enumValueChanged enter graphical=" << static_cast<void*>(_graphicalObject.data())
            << " model=" << static_cast<void*>(_modelObject)
            << " rebuilding=" << _isRebuildingProperties
            << " notifying=" << _isNotifyingModelChange
            << " pendingRebuild=" << _pendingRebuild;
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
    if (binding.kind == BindingKind::GraphicsEnum) {
        _applyGraphicsEnumChange(property, value);
        qInfo() << "[PropertyEditor] enumValueChanged exit";
        return;
    }

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
    // Adds scoped tracing for critical Property Editor crash-diagnosis paths.
    const GuiScopeTrace scopeTrace("ObjectPropertyBrowser::_notifyModelChangeApplied", this);
    // Logs model-change notification state to diagnose nested refresh cycles.
    qInfo() << "[PropertyEditor] _notifyModelChangeApplied enter graphical=" << static_cast<void*>(_graphicalObject.data())
            << " model=" << static_cast<void*>(_modelObject)
            << " rebuilding=" << _isRebuildingProperties
            << " notifying=" << _isNotifyingModelChange
            << " pendingRebuild=" << _pendingRebuild;
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
