#include "DataComponentProperty.h"

#include <utility>

#include "DataComponentEditor.h"

DataComponentProperty::DataComponentProperty(
    PropertyEditorGenesys* editor,
    SimulationControl* property,
    bool necessaryConfig,
    AfterChange afterChange
    ) : _editor(editor), _property(property), _afterChange(std::move(afterChange)) {

    _window = new QWidget;
    _view = new QTreeWidget(_window);
    _add = new QPushButton("Add", _window);
    _remove = new QPushButton("Remove", _window);
    _edit = new QPushButton("Edit", _window);
    _confirmation = new QInputDialog(_window);

    _add->move(270, 15);
    _remove->move(270, 50);
    _edit->move(270, 85);

    _view->setHeaderLabels({"Element"});
    _window->setFixedSize(360, 200);

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
        QTreeWidgetItem* newItem = new QTreeWidgetItem(_view);
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

    QString newValue = _confirmation->getText(_confirmation, "Item", "Enter the value:");
    if (newValue.isEmpty()) {
        return;
    }

    if (!isInList(newValue.toStdString())) {
        List<std::string>* oldValues = _property->getStrValues();
        const unsigned int oldSize = oldValues != nullptr ? oldValues->size() : 0;
        delete oldValues;

        _editor->changeProperty(_property, newValue.toStdString(), false);
        config_values();

        List<std::string>* newValues = _property->getStrValues();
        const unsigned int newSize = newValues != nullptr ? newValues->size() : 0;
        bool changed = false;
        if (newValues != nullptr) {
            for (const std::string& value : *newValues->list()) {
                if (value == newValue.toStdString()) {
                    changed = true;
                    break;
                }
            }
        }
        delete newValues;

        if (changed || newSize != oldSize) {
            _notifyChanged();
        }
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
    List<std::string>* oldValues = _property->getStrValues();
    const unsigned int oldSize = oldValues != nullptr ? oldValues->size() : 0;
    delete oldValues;

    _editor->changeProperty(_property, itemValue.toStdString(), true);
    config_values();

    List<std::string>* newValues = _property->getStrValues();
    const unsigned int newSize = newValues != nullptr ? newValues->size() : 0;
    bool removed = true;
    if (newValues != nullptr) {
        for (const std::string& value : *newValues->list()) {
            if (value == itemValue.toStdString()) {
                removed = false;
                break;
            }
        }
    }
    delete newValues;

    if (removed || newSize != oldSize) {
        _notifyChanged();
    }
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
