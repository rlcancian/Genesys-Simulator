#include "DataComponentEditor.h"

#include <utility>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>

#include "DataComponentProperty.h"
#include "ComboBoxEnum.h"

DataComponentEditor::DataComponentEditor(
    PropertyEditorGenesys* editor,
    SimulationControl* property,
    AfterChange afterChange
    ) : _editor(editor), _afterChange(std::move(afterChange)) {

    _window = new QWidget;
    _window->setWindowTitle("List Item Editor");
    auto* rootLayout = new QVBoxLayout(_window);

    _view = new QTreeWidget(_window);
    _edit = new QPushButton("Edit", _window);
    _newValue = new QInputDialog(_window);

    _view->setColumnCount(2);
    _view->setHeaderLabels({"Property", "Value"});
    _view->header()->setStretchLastSection(true);

    auto* lineLayout = new QHBoxLayout();
    lineLayout->addWidget(_view, 1);

    auto* buttons = new QVBoxLayout();
    buttons->addWidget(_edit);
    buttons->addStretch(1);
    lineLayout->addLayout(buttons);

    rootLayout->addLayout(lineLayout);
    _window->setMinimumSize(520, 300);

    QObject::connect(_edit, &QPushButton::clicked, this, [this, property]() {
        editProperty(property);
    });
}

DataComponentEditor::DataComponentEditor(
    PropertyEditorGenesys* editor,
    List<SimulationControl*>* properties,
    AfterChange afterChange
    ) : _editor(editor), _afterChange(std::move(afterChange)) {

    _window = new QWidget;
    _window->setWindowTitle("List Item Editor");
    auto* rootLayout = new QVBoxLayout(_window);

    _view = new QTreeWidget(_window);
    _edit = new QPushButton("Edit", _window);
    _newValue = new QInputDialog(_window);

    _view->setColumnCount(2);
    _view->setHeaderLabels({"Property", "Value"});
    _view->header()->setStretchLastSection(true);

    auto* lineLayout = new QHBoxLayout();
    lineLayout->addWidget(_view, 1);

    auto* buttons = new QVBoxLayout();
    buttons->addWidget(_edit);
    buttons->addStretch(1);
    lineLayout->addLayout(buttons);

    rootLayout->addLayout(lineLayout);
    _window->setMinimumSize(520, 300);

    QObject::connect(_edit, &QPushButton::clicked, this, [this, properties]() {
        editProperty(properties);
    });
}

void DataComponentEditor::_notifyChanged() {
    if (_afterChange) {
        _afterChange();
    }
}

void DataComponentEditor::open_window(SimulationControl* property) {
    _view->clear();
    configure_properties(property);
    _window->show();
    _window->raise();
    _window->activateWindow();
}

void DataComponentEditor::open_window(List<SimulationControl*>* properties) {
    _view->clear();
    configure_properties(properties);
    _window->show();
    _window->raise();
    _window->activateWindow();
}

void DataComponentEditor::configure_properties(SimulationControl* property) {
    if (property == nullptr) {
        return;
    }

    List<SimulationControl*>* nestedProperties = property->getEditableProperties();
    if (nestedProperties == nullptr) {
        return;
    }

    for (auto prop : *nestedProperties->list()) {
        auto* item = new QTreeWidgetItem(_view);
        item->setText(0, QString::fromStdString(prop->getName()));
        item->setText(1, QString::fromStdString(prop->getValue()));
        _view->addTopLevelItem(item);
    }
}

void DataComponentEditor::configure_properties(List<SimulationControl*>* properties) {
    if (properties == nullptr) {
        return;
    }

    for (auto prop : *properties->list()) {
        auto* item = new QTreeWidgetItem(_view);
        item->setText(0, QString::fromStdString(prop->getName()));
        item->setText(1, QString::fromStdString(prop->getValue()));
        _view->addTopLevelItem(item);
    }
}

void DataComponentEditor::editProperty(SimulationControl* property) {
    if (property == nullptr) {
        return;
    }

    List<SimulationControl*>* nestedProperties = property->getEditableProperties();
    if (nestedProperties == nullptr) {
        return;
    }

    const int selectedRow = _view->currentIndex().row();
    if (selectedRow < 0) {
        return;
    }

    int index = 0;
    for (auto prop : *nestedProperties->list()) {
        if (index == selectedRow) {
            if (prop->getIsList()) {
                auto* newList = new DataComponentProperty(_editor, prop, true, _afterChange);
                newList->open_window();
            } else if (prop->getIsEnum()) {
                auto* box = new ComboBoxEnum(_editor, prop, _afterChange);
                box->open_box();
            } else if (prop->getIsClass()) {
                auto* newClass = new DataComponentEditor(_editor, prop, _afterChange);
                newClass->open_window(prop);
            } else {
                const QString valueToChange = _newValue->getText(_newValue, "Item", "Enter the value:");
                if (!valueToChange.isNull()) {
                    const std::string oldValue = prop->getValue();
                    _editor->changeProperty(prop, valueToChange.toStdString());
                    _view->clear();
                    configure_properties(property);
                    if (prop->getValue() != oldValue) {
                        _notifyChanged();
                    }
                }
            }
            return;
        }
        ++index;
    }
}

void DataComponentEditor::editProperty(List<SimulationControl*>* properties) {
    if (properties == nullptr) {
        return;
    }

    const int selectedRow = _view->currentIndex().row();
    if (selectedRow < 0) {
        return;
    }

    int index = 0;
    for (auto prop : *properties->list()) {
        if (index == selectedRow) {
            if (prop->getIsList()) {
                auto* newList = new DataComponentProperty(_editor, prop, true, _afterChange);
                newList->open_window();
            } else if (prop->getIsEnum()) {
                auto* box = new ComboBoxEnum(_editor, prop, _afterChange);
                box->open_box();
            } else if (prop->getIsClass()) {
                auto* newClass = new DataComponentEditor(_editor, prop, _afterChange);
                newClass->open_window(prop);
            } else {
                const QString valueToChange = _newValue->getText(_newValue, "Item", "Enter the value:");
                if (!valueToChange.isNull()) {
                    const std::string oldValue = prop->getValue();
                    _editor->changeProperty(prop, valueToChange.toStdString());
                    _view->clear();
                    configure_properties(properties);
                    if (prop->getValue() != oldValue) {
                        _notifyChanged();
                    }
                }
            }
            return;
        }
        ++index;
    }
}
