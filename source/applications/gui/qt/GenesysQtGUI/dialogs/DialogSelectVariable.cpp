#include "DialogSelectVariable.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../../../../../kernel/simulator/model/ModelDataDefinition.h"

DialogSelectVariable::DialogSelectVariable(QWidget *parent)
    : QDialog(parent), comboBox(new QComboBox(this)), okButton(new QPushButton("OK", this)),
    cancelButton(new QPushButton("Cancel", this)) {

    setWindowTitle("Select a Variable or Attribute");

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(comboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    mainLayout->addLayout(buttonLayout);

    // Conecta os sinais aos slots
    connect(okButton, &QPushButton::clicked, this, &DialogSelectVariable::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &DialogSelectVariable::onCancelClicked);
}

ModelDataDefinition* DialogSelectVariable::selectedIndex() const {
    int currentIndex = comboBox->currentIndex();
    return indexToVariable.value(currentIndex);
}

void DialogSelectVariable::onOkClicked() {
    accept();
}

void DialogSelectVariable::onCancelClicked() {
    reject();
}

void DialogSelectVariable::setVariableItems(QList<ModelDataDefinition *> *definitions, ModelDataDefinition *definition) {
    comboBox->clear();
    indexToVariable.clear();

    QString standardInput = "None";

    comboBox->addItem(standardInput);
    indexToVariable[0] = nullptr;

    if (definitions != nullptr) {
        for (unsigned int i = 0; i < (unsigned int) definitions->size(); i++) {
            ModelDataDefinition* dataDefinition = definitions->at(static_cast<int>(i));
            if (dataDefinition == nullptr) {
                continue;
            }
            QString entryName = QString::fromStdString(dataDefinition->getName());
            entryName += QString(" (%1)").arg(QString::fromStdString(dataDefinition->getClassname()));
            indexToVariable[i + 1] = dataDefinition;
            comboBox->addItem(entryName);
        }
    }

    comboBox->setCurrentIndex(indexToVariable.key(definition, 0));
}
