#ifndef COMBOBOXENUM_H
#define COMBOBOXENUM_H

#include <functional>

#include <QObject>
#include <QWidget>
#include <QComboBox>

#include "../../../../kernel/simulator/PropertyGenesys.h"

class ComboBoxEnum : public QObject {
public:
    using AfterChange = std::function<void()>;

    ComboBoxEnum(
        PropertyEditorGenesys* editor,
        SimulationControl* property,
        AfterChange afterChange = {}
        );

    ~ComboBoxEnum() override = default;

public:
    void open_box();
    void configure_options(SimulationControl* property);

private Q_SLOTS:
    void changeValue();

private:
    QWidget* _window = nullptr;
    QComboBox* _comboBox = nullptr;

    PropertyEditorGenesys* _editor = nullptr;
    SimulationControl* _property = nullptr;
    AfterChange _afterChange;
};

#endif // COMBOBOXENUM_H
