#include "ComboBoxEnum.h"

#include <utility>

ComboBoxEnum::ComboBoxEnum(
    PropertyEditorGenesys* editor,
    SimulationControl* property,
    AfterChange afterChange
    ) : _editor(editor), _property(property), _afterChange(std::move(afterChange)) {

    _window = new QWidget;
    _comboBox = new QComboBox(_window);

    configure_options(property);
    if (property != nullptr) {
        _comboBox->setCurrentText(QString::fromStdString(property->getValue()));
    }

    _window->setFixedSize(160, 28);

    QObject::connect(
        _comboBox,
        static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this,
        &ComboBoxEnum::changeValue
        );
}

void ComboBoxEnum::open_box() {
    _window->show();
    _window->raise();
    _window->activateWindow();
    _comboBox->showPopup();
}

void ComboBoxEnum::configure_options(SimulationControl* property) {
    _comboBox->clear();
    if (property == nullptr || property->getStrValues() == nullptr) {
        return;
    }

    List<std::string>* values = property->getStrValues();
    for (const std::string& item : *values->list()) {
        _comboBox->addItem(QString::fromStdString(item));
    }
    delete values;
}

void ComboBoxEnum::changeValue() {
    if (_editor == nullptr || _property == nullptr) {
        return;
    }

    _editor->changeProperty(_property, std::to_string(_comboBox->currentIndex()));
    if (_afterChange) {
        _afterChange();
    }
}
