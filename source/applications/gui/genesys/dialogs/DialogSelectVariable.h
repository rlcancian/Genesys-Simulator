#ifndef DIALOGSELECTVARIABLE_H
#define DIALOGSELECTVARIABLE_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QList>
#include <QMap>

class ModelDataDefinition;

class DialogSelectVariable : public QDialog {
    Q_OBJECT

public:
    explicit DialogSelectVariable(QWidget *parent = nullptr);

    ModelDataDefinition* selectedIndex() const;
    void setVariableItems(QList<ModelDataDefinition *> *definitions, ModelDataDefinition *definition);

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    QComboBox *comboBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QMap<int, ModelDataDefinition*> indexToVariable;
};

#endif // DIALOGSELECTVARIABLE_H
