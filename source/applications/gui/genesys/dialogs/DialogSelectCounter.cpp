#include "DialogSelectCounter.h"
#include <QVBoxLayout>

DialogSelectCounter::DialogSelectCounter(QWidget *parent)
    : QDialog(parent), comboBox(new QComboBox(this)), okButton(new QPushButton("OK", this)),
    cancelButton(new QPushButton("Cancel", this)) {

    setWindowTitle("Select a Counter");

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(comboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    mainLayout->addLayout(buttonLayout);

    // Conecta os sinais aos slots
    connect(okButton, &QPushButton::clicked, this, &DialogSelectCounter::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &DialogSelectCounter::onCancelClicked);
}

Counter* DialogSelectCounter::selectedIndex() const {
    int currentIndex = comboBox->currentIndex();
    if (indexToCounter.contains(currentIndex)) {
        return indexToCounter[currentIndex];
    } else {
        return nullptr;
    }
}

void DialogSelectCounter::onOkClicked() {
    accept();
}

void DialogSelectCounter::onCancelClicked() {
    reject();
}

void DialogSelectCounter::setCounterItems(QList<Counter *> *counters, Counter *counter) {
    comboBox->clear();

    QString standardInput = "None";

    comboBox->addItem(standardInput);
    indexToCounter[0] = nullptr;

    for (unsigned int i = 0; i < (unsigned int) counters->size(); i++) {
        // Adiciona entradas ao mapeamento ponteiro-nome
        QString entryName = QString::fromStdString(counters->at(i)->getName());
        indexToCounter[i+1] = counters->at(i);
        comboBox->addItem(entryName);
    }

    comboBox->setCurrentIndex(indexToCounter.key(counter, 0));
}

