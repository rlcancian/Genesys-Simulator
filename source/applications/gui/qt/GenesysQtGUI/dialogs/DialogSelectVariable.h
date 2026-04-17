#ifndef DIALOGSELECTVARIABLE_H
#define DIALOGSELECTVARIABLE_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>

#include "plugins/data/DiscreteProcessing/Variable.h"

class DialogSelectVariable : public QDialog {
    Q_OBJECT

public:
    explicit DialogSelectVariable(QWidget *parent = nullptr);

    Variable* selectedIndex() const;
    void setVariableItems(QList<Variable *> *variables, Variable *variable);

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    QComboBox *comboBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QMap<int, Variable*> indexToVariable;
};

#endif // DIALOGSELECTVARIABLE_H
