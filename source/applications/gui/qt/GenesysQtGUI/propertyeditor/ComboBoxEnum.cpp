#include "ComboBoxEnum.h"

ComboBoxEnum::ComboBoxEnum(PropertyEditorGenesys* editor, SimulationControl* property) {
    _window = new QWidget;
    _comboBox = new QComboBox(_window);

    configure_options(property);
    _comboBox->setCurrentText(QString::fromStdString(property->getValue()));

    _window->setFixedSize(100,25);

    QObject::connect(_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this, editor, property]{changeValue(editor, property);});
};

ComboBoxEnum::~ComboBoxEnum() {
    delete _window;
    delete _comboBox;
};

void ComboBoxEnum::open_box() {
    _window->show();
    _comboBox->showPopup();
};

void ComboBoxEnum::configure_options(SimulationControl* property) {
    for (auto item : *property->getStrValues()->list()) {
        _comboBox->addItem(QString::fromStdString(item));
    }
}

void ComboBoxEnum::changeValue(PropertyEditorGenesys* editor, SimulationControl* property) {
    editor->changeProperty(property, std::to_string(_comboBox->currentIndex()));
}
