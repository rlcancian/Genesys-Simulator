#ifndef DATACOMPONENTPROPERTY_H
#define DATACOMPONENTPROPERTY_H

#include <functional>
#include <iostream>

#include <QObject>
#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QInputDialog>

#include "../../../../kernel/simulator/PropertyGenesys.h"

class DataComponentEditor;
class ComboBoxEnum;

class DataComponentProperty : public QObject {
public:
    using AfterChange = std::function<void()>;

    DataComponentProperty(
        PropertyEditorGenesys* editor,
        SimulationControl* property,
        bool necessaryConfig,
        AfterChange afterChange = {}
        );

    ~DataComponentProperty() override = default;

public:
    void open_window();
    void config_values();
    bool isInList(const std::string& value) const;

private:
    void _notifyChanged();

private Q_SLOTS:
    void addElement();
    void removeElement();
    void editProperty();

private:
    QWidget* _window = nullptr;
    QTreeWidget* _view = nullptr;
    QPushButton* _add = nullptr;
    QPushButton* _remove = nullptr;
    QPushButton* _edit = nullptr;
    QInputDialog* _confirmation = nullptr;

    PropertyEditorGenesys* _editor = nullptr;
    SimulationControl* _property = nullptr;
    AfterChange _afterChange;
};

#endif // DATACOMPONENTPROPERTY_H
