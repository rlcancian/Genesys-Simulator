#ifndef COMBOBOXENUM_H
#define COMBOBOXENUM_H

#include <iostream>

#include <QWidget>
#include <QComboBox>

#include "../../../../kernel/simulator/PropertyGenesys.h"


class ComboBoxEnum : public QObject {
public:
    ComboBoxEnum(PropertyEditorGenesys* editor, SimulationControl* property);
    ~ComboBoxEnum();

public:
    void open_box();
    void configure_options(SimulationControl* property);

private Q_SLOTS:
    void changeValue(PropertyEditorGenesys* editor, SimulationControl* property);

private:
    QWidget* _window;
    QComboBox* _comboBox;
};

#endif // COMBOBOXENUM_H
