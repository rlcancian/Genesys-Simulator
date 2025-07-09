#ifndef DATACOMPONENTPROPERTY_H
#define DATACOMPONENTPROPERTY_H

#include <iostream>

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QInputDialog>

#include "DataComponentEditor.h"
#include "ComboBoxEnum.h"

#include "../../../../kernel/simulator/PropertyGenesys.h"


class DataComponentProperty : public QObject {
public:
    DataComponentProperty(PropertyEditorGenesys* editor, SimulationControl* property, bool necessaryConfig);
    ~DataComponentProperty();

public:
    void open_window();
    void config_values(SimulationControl* property);
    bool isInList(SimulationControl* property, std::string value);

private Q_SLOTS:
    void addElement(PropertyEditorGenesys* editor, SimulationControl* property);
    void removeElement(PropertyEditorGenesys* editor, SimulationControl* property);
    void editProperty(PropertyEditorGenesys* editor, SimulationControl* property);

private:
    QWidget* _window;
    QTreeWidget* _view;
    QPushButton* _add;
    QPushButton* _remove;
    QPushButton* _edit;
    QInputDialog* _confirmation;
};

#endif // DATACOMPONENTPROPERTY_H
