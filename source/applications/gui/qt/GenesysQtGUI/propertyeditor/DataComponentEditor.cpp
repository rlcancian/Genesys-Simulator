#include "DataComponentEditor.h"

DataComponentEditor::DataComponentEditor(PropertyEditorGenesys* editor, SimulationControl* property) {
    _window = new QWidget;
    _view = new QTreeWidget(_window);
    _edit = new QPushButton("Edit", _window);
    _newValue = new QInputDialog(_window);

    _view->setColumnCount(2);
    _view->setHeaderLabels({"Property","Value"});
    _edit->move(270,15);

    _window->setFixedSize(360,200);

    QObject::connect(_edit, &QPushButton::clicked, this, [this, editor, property]{editProperty(editor, property);});
};

DataComponentEditor::DataComponentEditor(PropertyEditorGenesys* editor, List<SimulationControl*>* properties) {
    _window = new QWidget;
    _view = new QTreeWidget(_window);
    _edit = new QPushButton("Edit", _window);
    _newValue = new QInputDialog(_window);

    _view->setColumnCount(2);
    _edit->move(270,15);

    _window->setFixedSize(400,200);

    QObject::connect(_edit, &QPushButton::clicked, this, [this, editor, properties]{editProperty(editor, properties);});
};

DataComponentEditor::~DataComponentEditor() {
    delete _window;
    delete _view;
    delete _newValue;
};

void DataComponentEditor::open_window(SimulationControl* property) {
    _view->clear();
    configure_properties(property);

    _window->show();
};

void DataComponentEditor::open_window(List<SimulationControl*>* properties) {
    _view->clear();
    configure_properties(properties);

    _window->show();
};

void DataComponentEditor::configure_properties(SimulationControl* property) {
    if (property->getProperties()) {
        for (auto prop : *property->getProperties()->list()) {
            QTreeWidgetItem* newItem = new QTreeWidgetItem(_view);
            _view->addTopLevelItem(newItem);

            QString newValue = QString::fromStdString(prop->getName());
            QString itemValue = QString::fromStdString(prop->getValue());
            newItem->setText(0,newValue);
            newItem->setText(1,itemValue);
        }
    }
}

void DataComponentEditor::configure_properties(List<SimulationControl*>* properties) {
    for (auto prop : *properties->list()) {
        QTreeWidgetItem* newItem = new QTreeWidgetItem(_view);
        _view->addTopLevelItem(newItem);

        QString newValue = QString::fromStdString(prop->getName());
        QString itemValue = QString::fromStdString(prop->getValue());
        newItem->setText(0,newValue);
        newItem->setText(1,itemValue);
    }
}

void DataComponentEditor::editProperty(PropertyEditorGenesys* editor, SimulationControl* property) {
    // current element
    QTreeWidgetItem* currentItem = _view->currentItem();

    // change property
    int index = 0;
    for (auto prop : *property->getProperties()->list()) {
        if (index == _view->currentIndex().row()) {
            if (prop->getIsList()) {
                DataComponentProperty* newList = new DataComponentProperty(editor, prop, true);
                newList->open_window();
            } else if (prop->getIsEnum()) {
                    ComboBoxEnum* box = new ComboBoxEnum(editor, prop);
                    box->open_box();
                } else if (prop->getIsClass()) {
                        DataComponentEditor* newClass = new DataComponentEditor(editor, prop);
                        newClass->open_window(prop);
                    } else {
                        QString valueToChange = _newValue->getText(_newValue, "Item", "Enter the value:");
                        currentItem->setText(1,valueToChange);

                        editor->changeProperty(prop, valueToChange.toStdString());
                        }
        }
        index++;
    }
}

void DataComponentEditor::editProperty(PropertyEditorGenesys* editor, List<SimulationControl*>* properties) {
    // current element
    QTreeWidgetItem* currentItem = _view->currentItem();

    // change property
    int index = 0;
    for (auto prop : *properties->list()) {
        if (index == _view->currentIndex().row()) {
            if (prop->getIsList()) {
                DataComponentProperty* newList = new DataComponentProperty(editor, prop, true);

                newList->open_window();
            } else if (prop->getIsEnum()) {
                    ComboBoxEnum* box = new ComboBoxEnum(editor, prop);
                    box->open_box();
            } else if (prop->getIsClass()) {
                    DataComponentEditor* newClass = new DataComponentEditor(editor, prop);
                    newClass->open_window(prop);
                } else {
                    QString valueToChange = _newValue->getText(_newValue, "Item", "Enter the value:");
                    currentItem->setText(1,valueToChange);

                    editor->changeProperty(prop, valueToChange.toStdString());
                    }
        }
        index++;
    }
}
