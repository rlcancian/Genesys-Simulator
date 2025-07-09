#include "DataComponentProperty.h"

DataComponentProperty::DataComponentProperty(PropertyEditorGenesys* editor, SimulationControl* property, bool necessaryConfig) {
    _window = new QWidget;
    _view = new QTreeWidget(_window);
    _add = new QPushButton("Add", _window);
    _remove = new QPushButton("Remove", _window);
    _edit = new QPushButton("Edit", _window);
    _confirmation = new QInputDialog(_window);

    _add->move(270,15);
    _remove->move(270,50);
    _edit->move(270,85);

    _view->setHeaderLabels({"Element"});
    _window->setFixedSize(360,200);

    if (necessaryConfig) {
        config_values(property);
    }

    QObject::connect(_add, &QPushButton::clicked, this, [this, editor, property]{addElement(editor, property);});
    QObject::connect(_remove, &QPushButton::clicked, this, [this, editor, property]{removeElement(editor, property);});
    QObject::connect(_edit, &QPushButton::clicked, this, [this, editor, property]{editProperty(editor, property);});
};

DataComponentProperty::~DataComponentProperty() {
    delete _window;
    delete _view;
    delete _add;
    delete _remove;
    delete _confirmation;
};

void DataComponentProperty::open_window() {
    _window->show();
};

void DataComponentProperty::config_values(SimulationControl* property) {
    List<std::string>* values = property->getStrValues();

    for (auto value : *values->list()) {
        // add element
        QTreeWidgetItem* newItem = new QTreeWidgetItem(_view);
        _view->addTopLevelItem(newItem);

        newItem->setText(0,QString::fromStdString(value));
    }
}

bool DataComponentProperty::isInList(SimulationControl* property, std::string value) {
    for (auto element : *property->getStrValues()->list()) {
        if (value == element) {
            return true;
        }
    }
    return false;
}

void DataComponentProperty::addElement(PropertyEditorGenesys* editor, SimulationControl* property) {
    QString newValue = _confirmation->getText(_confirmation, "Item", "Enter the value:");
    if (!isInList(property, newValue.toStdString())) {
        // add element
        QTreeWidgetItem* newItem = new QTreeWidgetItem(_view);
        _view->addTopLevelItem(newItem);
        newItem->setText(0,newValue);

        // change property
        editor->changeProperty(property, newValue.toStdString());
    }
};

void DataComponentProperty::removeElement(PropertyEditorGenesys* editor, SimulationControl* property) {
    // select element to remove
    QTreeWidgetItem* selectedItem = _view->currentItem();
    int column = _view->currentColumn();
    QString itemValue = selectedItem->text(column);

    // remove the selected element
    delete selectedItem;

    // change property
    editor->changeProperty(property, itemValue.toStdString());
};

void DataComponentProperty::editProperty(PropertyEditorGenesys* editor, SimulationControl* property) {
    if (property->getIsClass()) {
        int index = _view->currentIndex().row();
        List<SimulationControl*>* propertiesElement = property->getProperties(index);
        DataComponentEditor* propertyEditor = new DataComponentEditor(editor, propertiesElement);

        propertyEditor->open_window(propertiesElement);

        // delete propertyEditor;
    }
}
