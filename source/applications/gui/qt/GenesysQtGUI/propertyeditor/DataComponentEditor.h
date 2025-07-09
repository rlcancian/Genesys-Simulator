#ifndef DATACOMPONENTEDITOR_H
#define DATACOMPONENTEDITOR_H

#include <iostream>

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QInputDialog>

#include "DataComponentProperty.h"

#include "../../../../kernel/simulator/PropertyGenesys.h"


class DataComponentEditor : public QObject {
public:
    DataComponentEditor(PropertyEditorGenesys* editor, SimulationControl* property);
    DataComponentEditor(PropertyEditorGenesys* editor, List<SimulationControl*>* properties);
    ~DataComponentEditor();

public:
    void open_window(SimulationControl* property);
    void open_window(List<SimulationControl*>* properties);

    void configure_properties(SimulationControl* property);
    void configure_properties(List<SimulationControl*>* properties);

    void editProperty(PropertyEditorGenesys* editor, SimulationControl* property);
    void editProperty(PropertyEditorGenesys* editor, List<SimulationControl*>* properties);

private:
    QWidget* _window;
    QTreeWidget* _view;
    QPushButton* _edit;
    QInputDialog* _newValue;
};

#endif // DATACOMPONENTEDITOR_H
