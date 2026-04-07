#include "DataComponentProperty.h"

#include <utility>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>

#include "DataComponentEditor.h"
#include "../../../../kernel/simulator/GenesysPropertyIntrospection.h"

DataComponentProperty::DataComponentProperty(
    PropertyEditorGenesys* editor,
    SimulationControl* property,
    bool necessaryConfig,
    AfterChange afterChange
    ) : _editor(editor), _property(property), _afterChange(std::move(afterChange)) {

    _window = new QWidget;
    _window->setWindowTitle("List Editor");

    auto* rootLayout = new QVBoxLayout(_window);
    auto* title = new QLabel("Manage list items (Arena style)", _window);
    rootLayout->addWidget(title);

    auto* contentLayout = new QHBoxLayout();
    _view = new QTreeWidget(_window);
    _add = new QPushButton("Add", _window);
    _remove = new QPushButton("Remove", _window);
    _edit = new QPushButton("Edit", _window);
    _confirmation = new QInputDialog(_window);

    _view->setHeaderLabels({"Element"});
    _view->header()->setStretchLastSection(true);

    auto* buttonsLayout = new QVBoxLayout();
    buttonsLayout->addWidget(_add);
    buttonsLayout->addWidget(_remove);
    buttonsLayout->addWidget(_edit);
    buttonsLayout->addStretch(1);

    contentLayout->addWidget(_view, 1);
    contentLayout->addLayout(buttonsLayout);
    rootLayout->addLayout(contentLayout);

    _window->setLayout(rootLayout);
    _window->setMinimumSize(460, 280);

    if (necessaryConfig) {
        config_values();
    }

    QObject::connect(_add, &QPushButton::clicked, this, &DataComponentProperty::addElement);
    QObject::connect(_remove, &QPushButton::clicked, this, &DataComponentProperty::removeElement);
    QObject::connect(_edit, &QPushButton::clicked, this, &DataComponentProperty::editProperty);
}

void DataComponentProperty::_notifyChanged() {
    if (_afterChange) {
        _afterChange();
    }
}

void DataComponentProperty::open_window() {
    config_values();
    _window->show();
    _window->raise();
    _window->activateWindow();
}

void DataComponentProperty::config_values() {
    _view->clear();

    if (_property == nullptr) {
        return;
    }

    List<std::string>* values = _property->getStrValues();
    if (values == nullptr) {
        return;
    }

    for (const std::string& value : *values->list()) {
        auto* newItem = new QTreeWidgetItem(_view);
        newItem->setText(0, QString::fromStdString(value));
        _view->addTopLevelItem(newItem);
    }

    delete values;
}

bool DataComponentProperty::isInList(const std::string& value) const {
    if (_property == nullptr) {
        return false;
    }

    List<std::string>* values = _property->getStrValues();
    if (values == nullptr) {
        return false;
    }

    bool found = false;
    for (const std::string& element : *values->list()) {
        if (value == element) {
            found = true;
            break;
        }
    }

    delete values;
    return found;
}

void DataComponentProperty::addElement() {
    if (_editor == nullptr || _property == nullptr) {
        return;
    }

    // This block routes Add to explicit typed-creation support when the list control provides it.
    GenesysPropertyDescriptor descriptor = GenesysPropertyIntrospection::describe(_property);
    if (descriptor.supportsNewListElementCreation) {
        std::string errorMessage;
        const bool ok = GenesysPropertyIntrospection::setValue(
            _property,
            "",
            false,
            &errorMessage
            );
        if (ok) {
            config_values();
            _notifyChanged();
        }
        return;
    }

    // This block preserves the legacy textual fallback path for lists without typed creation support.
    const QString prompt = _property->getIsClass() ? "Enter the new item name:" : "Enter the value:";
    QString newValue = _confirmation->getText(_confirmation, "Add Item", prompt);
    if (newValue.isEmpty()) {
        return;
    }

    if (!isInList(newValue.toStdString())) {
        _editor->changeProperty(_property, newValue.toStdString(), false);
        config_values();
        _notifyChanged();
    }
}

void DataComponentProperty::removeElement() {
    if (_editor == nullptr || _property == nullptr) {
        return;
    }

    QTreeWidgetItem* selectedItem = _view->currentItem();
    if (selectedItem == nullptr) {
        return;
    }

    const QString itemValue = selectedItem->text(0);
    _editor->changeProperty(_property, itemValue.toStdString(), true);
    config_values();
    _notifyChanged();
}

void DataComponentProperty::editProperty() {
    if (_property == nullptr || !_property->getIsClass()) {
        return;
    }

    const int index = _view->currentIndex().row();
    if (index < 0) {
        return;
    }

    List<SimulationControl*>* propertiesElement = _property->getProperties(index);
    if (propertiesElement == nullptr) {
        return;
    }

    auto* propertyEditor = new DataComponentEditor(_editor, propertiesElement, _afterChange);
    propertyEditor->open_window(propertiesElement);
}
