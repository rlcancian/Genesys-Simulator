#ifndef DIALOGSELECTCOUNTER_H
#define DIALOGSELECTCOUNTER_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>

#include "kernel/simulator/Counter.h"

class DialogSelectCounter : public QDialog {
    Q_OBJECT

public:
    explicit DialogSelectCounter(QWidget *parent = nullptr);

    Counter* selectedIndex() const;
    void setCounterItems(QList<Counter *> *counters, Counter *counter);

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    QComboBox *comboBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QMap<int, Counter*> indexToCounter;
};

#endif // DIALOGSELECTCOUNTER_H
