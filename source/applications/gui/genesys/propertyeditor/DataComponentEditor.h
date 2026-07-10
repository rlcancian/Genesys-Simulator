#ifndef DATACOMPONENTEDITOR_H
#define DATACOMPONENTEDITOR_H

#include <functional>
#include <iostream>

#include <QObject>
#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QInputDialog>

#include "kernel/simulator/PropertyGenesys.h"

class DataComponentProperty;
class ComboBoxEnum;

class DataComponentEditor : public QObject {
public:
    using AfterChange = std::function<void()>;

    DataComponentEditor(
        PropertyEditorGenesys* editor,
        SimulationControl* property,
        AfterChange afterChange = {}
        );

    DataComponentEditor(
        PropertyEditorGenesys* editor,
        List<SimulationControl*>* properties,
        AfterChange afterChange = {}
        );

    ~DataComponentEditor() override = default;

public:
    void open_window(SimulationControl* property);
    void open_window(List<SimulationControl*>* properties);

    void configure_properties(SimulationControl* property);
    void configure_properties(List<SimulationControl*>* properties);

private:
    void _notifyChanged();

    void editProperty(SimulationControl* property);
    void editProperty(List<SimulationControl*>* properties);

private:
    QWidget* _window = nullptr;
    QTreeWidget* _view = nullptr;
    QPushButton* _edit = nullptr;
    QInputDialog* _newValue = nullptr;

    PropertyEditorGenesys* _editor = nullptr;
    AfterChange _afterChange;
};

#endif // DATACOMPONENTEDITOR_H
